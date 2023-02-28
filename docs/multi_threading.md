## Multi Threading

Given Julia's application in high-performance computing and its native multi-threading support, it is only natural a Julia wrapper should also allow for asynchronous execution in a similarly convenient and performant manner.

In relation to specifically a C++ <-> Julia environment, we run into a problem, however:

```cpp
#include <jluna.hpp>
#include <thread>

int main()
{
    // initialize
    jl_init();
    
    // create a lambda that does nothing
    auto noop = []() -> void {
        jl_eval_string("");
        return;
    };
    
    // execute it in a std::thread
    auto thread = std::thread(noop);
    
    // wait fo thread to finish
    thread.join();
    
    // exit main
    return 0;
}
```

Here, we're doing a very simple operation. We created a C++ lambda that *does nothing*. It executes no Julia-side code by handing `jl_eval_string` (the C-APIs way to execute strings as code) an empty string, after which it simply returns. We then create a C++-side thread using `std::thread`. In C++, once a thread is created, it starts running immediately. We wait for its execution to finish using `.join()`.

Note that **not a single jluna function was called** over the runtime of this main. All functions where purely Julia C-API functions.

Running the above code, the following happens:
```text
signal (11): Segmentation fault
in expression starting at none:0
Allocations: 782030 (Pool: 781753; Big: 277); GC: 1

Process finished with exit code 139 (interrupted by signal 11: SIGSEGV)
```

It segfaults without an error.

I am unsure of the exact reason for this behavior, but [some have stated](https://github.com/Clemapfel/jluna/issues/42) that it is related to the way Julia initializes thread local storage (TLS), which is the environment of code ran inside a thread. Only Julia threads themselves allocate the TLS correctly, when running Julia from a non-Julia thread, it will look for it's allocated TLS, which isn't there, hence the segfault.

While this was apparently [fixed in Julia version 1.9](https://github.com/JuliaLang/julia/pull/46609), for those of us using an earlier Julia version, this makes things quite difficult for us. **We cannot access the Julia state from within a C++-side thread**. This has nothing to do with jluna, it is how the C-API was designed.

Given this, we can immediately throw out using any of the C++ `std::thread`-related multi-threading support, as well as libraries like [libuv](https://github.com/libuv/libuv). It is important to keep this in mind, if our application already uses these frameworks, we have to take care to **never execute code that interacts with Julia from within a thread**. This includes any of jlunas functionalities, as it is, of course, build entirely on the Julia C-API.

All is not lost, however: jluna offers its own multi-threading framework, allowing for parallel execution of truly arbitrary C++ code - even if that code interacts with the Julia state.

### Initializing the Julia Threadpool

In Julia, we have to decide the number of threads we want to use **before startup**.
In the Julia REPL, we would use the `-threads` (or `-t`) argument. In jluna, we instead specify the desired number of threads as an argument to `jluna::initialize`:

```cpp
int main()
{
    jluna::initialize(8); 
    // equivalent to `julia -t 8`
    
    // application here
    
    return 0;
}
```
```
[JULIA][LOG] initialization successful (8 threads).
```

If left unspecified, jluna will initialize Julia with exactly 1 thread. We can set the number of threads to `auto` by supplying the following constant to `jluna::initialize`:

```cpp
using namespace jluna;

initialize(JULIA_NUM_THREADS_AUTO);
// equivalent to `julia -t auto`
```

This sets the number of threads to number of local CPUs, just like [setting environment variable `JULIA_NUM_THREADS` to `auto`](https://docs.julialang.org/en/v1/manual/multi-threading/#Starting-Julia-with-multiple-threads) would do for pure Julia.

Note that any already existing `JULIA_NUM_THREAD` variable, in the environment the jluna executable is run in, is **ignored and overridden**. We can only specify the number of threads through `jluna::initialize`.

### Creating a Task

Owing to its status of being in-between two languages with differing vocabulary and design, jlunas thread pool architecture borrows from both C++ and Julia.

To execute a C++-side piece of code, we have to first wrap it into a C++ lambda, then wrap that lambda in a `jluna::Task`.

We cannot initialize a task directly, rather, we use `jluna::ThreadPool::create`:

```cpp
using namespace jluna;

// declare lambda that prints to console and returns its argument
auto forward_arg = [](size_t in) -> size_t {
    Base["println"]("lambda called with ", in);
    return in;
};

// create task
auto task = ThreadPool::create<size_t(size_t)>( // signature
    forward_arg,    // function
    size_t(1234)    // function arguments
);
```

Here, `ThreadPool::create` takes multiple arguments:

+ its **template argument** is the signature of the lambda
    - Just like with `as_julia_function`, it expects a C-style signature. `forward_arg` has the signature `(size_t) -> size_t`, making `size_t(size_t)` the appropriate template argument.

+ the **first argument** is the **function object** itself
    - this can be a lambda, like `forward_arg`, or a `std::function` object.

+ any **following arguments** will be used as the **arguments for the given function**
    - In our case, because we specified `size_t(1234)`, the thread pool will invoke our lambda `forward_arg` with that argument (and only that argument).

Note that `create` invokes the copy constructor on all its argument. If this behavior is not desired, we can wrap the argument in a `std::reference_wrapper` using `std::ref`, meaning only the reference itself will be copied, not the actual object.

Unlike with `as_julia_function`, the signature of lambdas used for `ThreadPool::create` is unrestricted - any lambda can be used.

### Running a Task

Unlike C++-threads (but much like Julia tasks), `jluna::Task` does not immediately start execution once it is constructed. We need to manually "start" is using `.schedule()`.

We can wait for its execution to finish using `.join()`. This stalls the thread `.join()` was called from (usually the master thread in which `main` is executed) until the task completes:

```cpp
// declare lambda
std::function<size_t(size_t)> forward_arg = [](size_t in) -> size_t {
    Base["println"]("lambda called with ", in);
    return in;
};

// create task
auto task = ThreadPool::create(forward_arg, size_t(1234));

// start task
task.schedule();

// wait for task to finish
task.join();
```
```text
lambda called with 1234
```

Note how, even though we called the Julia function `println`, the task **did not segfault**. Using jlunas thread pool is the only way to call C++-side functions that also access the Julia state concurrently.

### Managing a Tasks Lifetime

The result of `ThreadPool::create` is a `jluna::Task<T>`, where `T` is the return type of the C++ function used to `create` it, or `void`. **The user is responsible for keeping the task in memory**. If the variable the task is bound to, goes out of scope, the task simply ends:

```cpp
/// in main.cpp
jluna::initialize(8);

// open block scope
{
    // declare function that counts to 9999
    std::function<void()> count_to_9999 = []() -> void
    {
        for (size_t i = 0; i < 9999; ++i)
            std::cout << i << std::endl;
    };

    // create task
    auto task = ThreadPool::create(count_to_9999);
    
    // start task
    task.schedule();

    // wait for 1 millisecond
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
}
// task is destructed here

std::this_thread::sleep_for(10ms); // wait for another 10ms
return 0; // exit main
```
```text
(...)
2609
2609
2610
2611

Process finished with exit code 0
```
> **C++ Hint**: A *block* or *anonymous block scope* is created using `{` `}`. It acts similar to Julia's `begin` `end`, anything declared inside the block will go out of scope when the block ends. In C++, all variables declared inside the block are local to only that block.

> **C++ Hint**: `std::chrono` are C++s time-related functionalities. calling `std::this_thread::sleep_for(1ms)` will stall the master thread for 1 millisecond.

Here, our task is supposed to count all the way up to 9999. Instead, the task went out of scope before it could finish, only being able to count to 2611 before it was terminated.

A way to solve this is to store the task in a collection that is itself in master scope:

```cpp
// task storage
std::vector<Task<void>> tasks;

{
    // declare lambda
    std::function<void()> print_numbers = []() -> void
    {
        for (size_t i = 0; i < 10000; ++i)
            std::cout << i << std::endl;
    };

    // add task to storage
    tasks.push_back(ThreadPool::create(print_numbers));
    
    // start just pushed task
    tasks.back().schedule();
    
    // wait for 1ms
    std::this_thread::sleep_for(1ms);
}

// wait for another 10ms
std::this_thread::sleep_for(10ms);
return 0;
```
```text
(...)
9996
9997
9998
9999

Process finished with exit code 0
```

This time, because the task was not destructed prematurely, it had 10 milliseconds more time to finish. This happened to be enough for it to reach its intended count, after which it was safely destructed when `main` returned.

jluna has no equivalent to Julia's `Threads.@spawn`. This is to force users to keep track of their tasks. To `.schedule` a task, they need to first cache it in a variable. This hopefully avoids situations where tasks end unexpectedly because they go out of scope.

### Accessing a Tasks State

We can check the status of any task using the following member functions:

+ `is_done`: is the task finished or failed, c.f. `Base.istaskdone`
+ `is_failed`: has the task failed, c.f. `Base.istaskfailed`
+ `is_running`: has the task been scheduled but not yet finished, c.f. `Base.istaskstarted`

Furthermore, any Julia function that works with tasks can be called directly using the `jluna::Task` object as an argument.

### Accessing a Tasks Result

Returning to our example from before:

```cpp
// declare lambda
std::function<size_t(size_t)> forward_arg = [](size_t in) -> size_t {
    
    // print
    Base["println"]("lambda called with ", in);
    
    // return argument
    return in;
};

// create task, schedule, wait for it to finish
auto task = ThreadPool::create(forward_arg, size_t(1234));
task.schedule();
task.join();
```

Here, `forward_arg` does more than just print to the command line, it also returns a value.

To access the return value of a task, we use the member function `.result()`. This function returns an object of type `jluna::Future`.

A future is a thread-safe container that may or may not (yet) contain a value. The future itself is available immediately when its corresponding task is created.
```cpp
auto task = ThreadPool::create(forward_arg, size_t(1234));
auto future = task.result();
```

Until the task has successfully completed, however, the future will be "empty". Once the task is done, the return value will be copied into the future, after which we can access it. If we want to avoid the copying, we need to wrap the result in a `std::reference_wrapper`.

To get the potential value of a future, we use `.get()`, which returns a `std::optional<T>` where `T` is the return type of the C++ function used to `create` the task. Once completed, we can access the value of the optional using `std::optional::value()`. To check whether the value is already available, we can use `jluna::Future::is_available()`:

```cpp
// create task
auto task = ThreadPool::create(forward_arg, size_t(1234));

// get future of task (currently empty)
auto future = task.result();

std::cout << future.is_available() << std::endl; // false

// start task
task.schedule();

// wait for task to finish
task.join();

std::cout << future.is_available() << std::endl; // true

// print value of future
std::cout << future.get().value() << std::endl;
```
```
0
1
1234
```
> **C++ Hint**: Trying to access the value of a std::optional before it is available will raise an exception. See the [standard libraries documentation](https://en.cppreference.com/w/cpp/utility/optional) for more ways to interact with `std::optional<T>`.

We can wait for the value of a future to become available by calling `.wait()`. This will stall the thread `.wait()` is called from until the value becomes accessible, after which the function will return that value. This way, we don't necessarily need to keep track of the futures task, just having the future allows us to access the task's result. We do still need to make sure the corresponding task stays in scope, however.

### Data Race Freedom

The user is responsible for any potential data races a `jluna::Task` may trigger. Useful C++-side tools for this application include the following (where their Julia-side functional equivalent is listed for reference):

| C++                       | Julia                | C++ Documentation                                                     |
|---------------------------|----------------------|-----------------------------------------------------------------------|
| `std::mutex`              | `Base.ReentrantLock` | [[here]](https://en.cppreference.com/w/cpp/thread/mutex)              |
| `std::lock_guard`         | `Threads.@lock`      | [[here]](https://en.cppreference.com/w/cpp/thread/lock_guard)         |
| `std::condition_variable` | `Threads.Condition`  | [[here]](https://en.cppreference.com/w/cpp/thread/condition_variable) |
| `std::unique_lock`        | `n/a`                | [[here]](https://en.cppreference.com/w/cpp/thread/unique_lock)        |

Furthermore, jluna provides its own lock-like object `jluna::Mutex`, which is a simple wrapper around a Julia-side `Base.ReentrantLock`. It has the same usage and interface as `std::mutex`, except that it works when called both from C++ and Julia, because it is (Un)Boxable.

### Thread-Safety

As a general rule, any particular part of jluna is thread-safe, as long as two threads are **not modifying the same object at the same time**.

For example, `jluna::safe_call` can be called from multiple threads, and `safe_call` itself will work. If both `safe_call` modify the same value, however, concurrency artifacts may occur.

Similarly, we can freely create unrelated proxies and modify them individually. If we create two proxies that reference the same Julia-side variable, mutating both proxies (if they are named) at the same time will possibly trigger an error or data corruption.

The user is required to ensure thread-safety in these conditions, just like they would have to in Julia. jluna has no hidden pitfalls or behind-the-scene machinery that multi-threading may throw a wrench into, all its internals (that are inaccessible to the user) are thread-safe as of version 0.9.0.

Any user-created objects are outside of jlunas responsibility, however.

Calls to C++ lambdas forwarded to the Julia state using `as_julia_function` are thread-safe to *call*, whether their behavior is thread-safe depends on the user-defined implementation.

Notably, `Module::new_*`, `Module::create` and `Module::create_or_assign` are thread-safe. Each `jluna::Module` carries exactly one lock, which allows these calls to happen safely in a multi-threaded environment. All other functions of `jluna::Module` do not make use of this lock. Instead, users will be required to manually "lock" an object, and manage concurrent interaction with it, themselves.

### Thread-Safety in Julia

As an example for how to make modifying an object thread-safe, let's say we have the following variable in `Main`:

```julia
# in Julia
to_be_modified = []
```

We want to thread-safely add elements to this vector. One way to do this would be with a Julia-side Lock:

```julia
# in Julia
to_be_modified = []
to_be_modified_lock = Base.ReentrantLock()
```

Now, when a thread wants to modify `to_be_modified`, it first needs to acquire this lock:

```cpp
// in cpp

// thread function
auto push_to_modify = [](size_t) -> void
{
    // access Base.lock
    static auto lock = Base["lock"];
    
    // access Base.unlock
    static auto unlock = Base["unlock"];
  
    // access Julia-side lock
    auto* mutex = Main["to_be_modified_lock"];
    
    // lock 
    lock(mutex);
  
    // modify vector here
  
    // unlock
    unlock(mutex);
    return;
}
```
> **C++ Hint**: Using `static` in block-scope declares (and potentially defines) a variable *exactly once*. Anytime the code is run through, afterwards, the line with `static` is skipped. For this example, by making `lock` and `unlock` static variables, the lookup performed by `Base::operator[]` is only done a single time across the entire runtime of the application, increasing performance.
>
> See [here](https://en.cppreference.com/w/cpp/language/storage_duration) for more information on storage classifiers.

Since `push_to_modify` will be executed through `jluna::ThreadPool`, the Julia function `lock` will stall the thread, allowing for safe access.

### Thread-Safety in C++

A similar approach can be taken when trying to safely modify C++-side objects. Instead of a Julia-side lock, we use a C++-side `jluna::Mutex`:

```cpp
// C++-side variable we want to thread-safely access
jluna::Vector<size_t> to_be_modified;

// C++-side mutex
auto to_be_modified_lock = jluna::Mutex();

auto push_to_modify = [](size_t)
{
    to_be_modified_lock.lock();
  
    // modify here
  
    to_be_modified_lock.unlock();
    return;
}
```

Similarly, `std::mutex` or `std::unique_lock` can be used for the same purpose.

### Multi-Threading: Closing Notes

#### Interacting with `jluna::Task` from Julia

Internally, jluna makes accessing the Julia-state from a C++-sided, asynchronously executed function possible, by wrapping it in a Julia-side `Task`. jluna can then use Julia's native thread pool, allowing for C-API functions to be safely executed. This has some side effects, most of them useful.

For example, `yield`, called from C++ like so:

```cpp
static auto yield = Main.safe_eval("return Base.yield");
yield();
```

Will actually yield the thread this C++ code is executed in, letting another Julia thread take over. This applies to all Julia-side functions such as `fetch`, `bind`, etc. <br>Calling them from within a `jluna::Task` has exactly the same effect as calling them from within a `Base.Task`.

We can access the Julia-side object, `jluna::Task` is managing, using `operator unsafe::Value*()`, which returns a raw C-pointer to the Julia-side tasks. This allows us to create a `jluna::Proxy` of a `jluna::Task` like so:

```cpp
// declare lambda
auto lambda = [](){ //...
    
// create task
auto task = ThreadPool::create<void()>(lambda);

// create proxy to task
auto task_proxy = Proxy(static_cast<unsafe::Value*>(task));

// all Julia-only attributes can now be accessed:
std::cout << (bool) task_proxy["sticky"] << std::endl;
```
```
true
```

> **Julia Hint**: `Threads.Tasks.sticky` is a property that governs whether a task can be executed concurrently. By default, `sticky` is set to `false`, making it "stick" to the main thread, instead of "detaching" and being run on its own.

We can then use this proxy as we would use a Julia-side variable, allowing for full freedom on how to manage and schedule tasks.

#### Moving / Copying Tasks

Each task holds a pointer to an internal state, which manages the task's future among other things. To preserve this state, copying a task is made impossible (by declaring the tasks [copy constructor](https://en.cppreference.com/w/cpp/language/copy_constructor) and [assignment operator](https://en.cppreference.com/w/cpp/language/copy_assignment) as `delete`d):

```cpp
auto f = [](){};

Task<void> original = ThreadPool::create<void()>(f);
Task<void> copy = original; // compiler error
```
```text
/home/Workspace/jluna/.test/main.cpp:33:23: error: use of deleted function ‘jluna::Task<void>::Task(const jluna::Task<void>&)’
    4 |     Task<void> copy = original;
      |                       ^~~~~~~~
/home/Workspace/jluna/.src/multi_threading.inl:213:13: note: declared here
  213 |             Task(const Task&) = delete;
      |             ^~~~
```

However, task's can still be **move** [assigned](https://en.cppreference.com/w/cpp/language/move_assignment) / [constructed](https://en.cppreference.com/w/cpp/language/move_constructor). This is why we can store them in containers. 

If a move is invoked explicitly, the old task's internal state is safely transferred:

```cpp
auto f = []() -> int {
    return 1234
};

Task<int> original = ThreadPool::create<int()>(f);
Task<int> copy = std::move(original); // no error
```

Users need to be aware that, **after a move assignment / construction, the old task is no longer valid**. `Task::is_failed` will return `true`, other member functions will simply exit at the earliest possible point.
Trying to access an invalidated tasks future, however, will cause an error:

```cpp
auto f = [](){};

Task<int> original = ThreadPool::create<int()>(f);
Task<int> copy = std::move(original);

auto future = original.get().value(); // despite original being now invalid
```
```
[ERROR][C++] In Task<T>::result: trying to access the future of a task that is no longer valid. Tasks are invalidated when the move assignment operator or move constructor is called, which transfers a tasks internal state into the newly constructed one.
jluna_test: /home/clem/Workspace/jluna/.src/multi_threading.inl:197: jluna::Future<Result_t>& jluna::Task<Result_t>::result() [with Result_t = int]: Assertion `this->_value != nullptr' failed.
```

In general, a **task invalidated after a move should not be interacted with**. Users should be aware of this, though during general usage it is rare to run into this error.


#### Do **NOT** use `@threadcall`

Lastly, a warning: Julia has a macro called `@threadcall`, which purports to simply execute a `ccall` in a new thread. Internally, it actually uses the libuv thread pool for this, not the native Julia thread pool. Because the C-API is seemingly hardcoded to segfault any attempt at accessing the Julia state through any non-master C-side thread, using `@threadcall` to call arbitrary code will also trigger this segfault. Because of this, it is not recommended to use `@threadcall` in any circumstances. Instead, we can `ccall` from within a proper `Base.Task`, or use jlunas thread pool to execute C-side code in the first place.
