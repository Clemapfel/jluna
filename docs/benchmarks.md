## Performance Optimization

The `unsafe` libraries' biggest draw is that of increased performance. So far, readers just had to trust the author that `unsafe` was actually much faster. In this section, this will be verified empirically using **benchmarks**.

> **Hint**: Benchmarking is the process of running a piece of codes many times, recording the time it takes to finish during each cycle. If benchmarks are well-designed, we can calculate how much slower or faster a specific function is by comparing their runtimes to each other.

In general, when optimizing a program using jluna, the following statement is appropriate:

+ speed, safety, convenience - you can only pick two

The first part of this manual dealt with functions that offer **safety and convenience**. Proxies and the more abstracted parts of jluna are easy to use. They forward any exception to the user, preventing most of the C-like errors such as segfaults or silent data corruption.

The `unsafe` library swaps out safety, leaving use with **speed and convenience**. We will see that, by using `unsafe` functions, we can achieve top-notch performance without having to handle raw memory in C. In return, we have no safety-net whatsoever.

Lastly, if we want **speed and safety**, we need to use the `unsafe` library or C-API and do all the exception catching and GC-safety ourselves. This is obviously very annoying to do, dropping convenience from the equation.

The obvious question is: how much slower is the "safe way"? This section will answer this question, giving an exact percentage.

### Methodology

> **Hint**: This section explains the performance evaluations methodology, for the sake of transparency and reproducibility. User who are only interested in the results can safely skip this section.

All benchmarks were done on a machine with the following CPU (output of unix' `lshw`)

```
*-memory
    description: System memory
    physical id: 0
    size: 8064MiB

*-cpu
    product: Intel(R) Core(TM) i5-8250U CPU @ 1.60GHz
    vendor: Intel Corp.
    physical id: 1
    bus info: cpu@0
    size: 3040MHz
    capacity: 3400MHz
    width: 64 bits
    capabilities: fpu fpu_exception wp vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp x86-64 constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf pni pclmulqdq dtes64 monitor ds_cpl est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb invpcid_single pti ssbd ibrs ibpb stibp fsgsbase tsc_adjust sgx bmi1 avx2 smep bmi2 erms invpcid mpx rdseed adx smap clflushopt intel_pt xsaveopt xsavec xgetbv1 xsaves dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp md_clear flush_l1d cpufreq
```

Benchmarks were limited to run only in a single thread. The machine was kept clean of other processes during benchmarking. The benchmarks were run long enough (>30s per benchmark) for noise to not significantly affect the results. Care was taken to never fill up the RAM, as to not introduce potential swap-space related artifacts. Cache-related effects were not accounted for.

A [custom benchmark library](../.benchmark/benchmark.hpp) was used, which timed each benchmark using a `std::chrono::steady_clock`. The full sourcecode of all benchmarks in this section is available  [here](../.benchmark/main.cpp).

Relative overhead was measured using the following function:

```cpp
using Duration_t = std::chrono::duration<double>;
double overhead(Duration_t a, Duration_t b) 
{
    return (a < b ? 1 : -1) * (abs(a - b) / a); 
};
```

This function was deemed to represent the intuitive notion of speedup and overhead:

+ A is 10% faster than B, if Bs runtime is 110% that of A
    - -> A  exhibits a -10% overhead compared to B
+ C is 25% slower than D, if Cs runtime is 125% that of D.
    - -> C  exhibits a +25% overhead compared to D

The **median cycle duration** for any particular benchmark run was used to measure overhead. This mitigates potential spikes due to noise or one-time-only allocations, which a simple mean would have exhibited.

### Results: Introduction

When measuring performance, absolute number are rarely very informative. We either need to normalize the duration relative to the machine it was run on, or, **compare two results**, both run during the same benchmarking session, on the same machine.

The latter approach was used for this chapter. Each benchmark starts out with a "baseline" that all further results for that section will be compared to. This is usually the fastest possible way to do a certain task using **only the Julia C-API**. Afterwards, the exact same task is accomplished using jluna functions instead. We then calculate their overhead compared to baseline, as detailed above.

Each benchmark has the following form:

```cpp
Benchmark::run_as_base("name of base", n_reps, [](){
    // code of baseline
});

Benchmark::run("name of comparison", n_reps, [](){
    // code of comparison
});
```

Where

+ `Benchmark::run_as_base` executes the function and sets its duration as the baseline
+ `Benchmark::run` executes the function and compares it to the baseline
+ `n_reps` is the number of benchmark cycles
+ each benchmark has a name, identifying it

Note that one of jlunas design goals was for any function in the `unsafe` library to exhibit an overhead of no more than 5%. We will see if this goal was achieved.

---

### Benchmark: Accessing Julia-side Values

One of the most basic tasks in jluna is getting the value of a Julia-side object. This can be accomplished in many ways, some of which are listed here:

```cpp
// number of cycles
size_t n_reps = 1000000;

// allocate function object Julia-side
// this is the value we will access
Main.safe_eval(R"(
function f()
    sum = 0;
    for i in 1:100
        sum += rand();
    end
end
)");

// C-API
Benchmark::run_as_base("C-API: get", n_reps, [](){
    volatile auto* f = jl_get_function(jl_main_module, "f");
    // exactly identical to jl_get_global
});

// unsafe::get_function
Benchmark::run("unsafe: get_function", n_reps, [](){
    volatile auto* f = unsafe::get_function(jl_main_module, "f"_sym);
});

// unsafe::get_value
Benchmark::run("unsafe: get_value", n_reps, [](){
    volatile auto* f = unsafe::get_value(jl_main_module, "f"_sym);
});

// named Proxy from unsafe
Benchmark::run("named proxy: from unsafe", n_reps, [](){
    volatile auto* f = (unsafe::Function*) Proxy(unsafe::get_value(jl_main_module, "f"_sym), "f"_sym);
});

// Proxy.operator[](std::string)
Benchmark::run("named proxy: operator[]", n_reps, [](){
    volatile auto* f = (unsafe::Function*) Main["f"];
});

// Main.get
Benchmark::run("module: get", n_reps, [](){
    volatile auto* f = Main.get<unsafe::Function*>("f");
});

// C-API: eval
Benchmark::run("eval: get", n_reps / 4, [](){
    volatile auto* f = (unsafe::Function*) jl_eval_string("return f");
});
```

> **C++ Hint**: `volatile` is a specifier keyword that prevents whatever was declared as volatile to be optimized-away by the compiler. This is highly useful for benchmarks, as it allows us to make sure that compiler optimization does not prevent a valid comparison.

Here, we created a Julia-side function `f`. We used various ways to access the value of `f`, which is a Julia function object pointer.

### Accessing Julia-side Values: Results

| function used                    | median duration (ms) | overhead (%)  |
|----------------------------------|----------------------|---------------|
| `C-API`                          | `8.1e-05ms`          | `0%`          |
| `unsafe::get_function`           | `8.4e-05ms`          | `4%`          |
| `unsafe::get_value`              | `8.4e-05ms`          | `4%`          |
| `Module::get<T>`                 | `0.000282ms`         | `248%`        |
| `Proxy::Proxy(unsafe::Value*)`   | `0.005378ms`         | `6190%`       |
| `Proxy::operator[](std::string)` | `0.005378ms`         | `6540%`       |
| `jl_eval_string`                 | `0.085889ms`         | `105936%`     |

We see that there are vast runtime difference between individual benchmarks. Firstly, `unsafe` falls into the `< 5%` overhead range, which is good to see.

Returning values by evaluating a string is, as stated many times in this manual, prohibitively slow and should never be used unless unavoidable, even if we use `jl_eval_string`, which should theoretically be the fastest possible way to evaluate a string.

`Module::get<T>` may not look like it would be much faster than just using `Module::operator[]`, however, this is not the case. `Module::get<T>` is much faster because it **never has to construct a proxy**. The function knows the final result type, `T` (`unsafe::Function*` in this example), because of this `Module::get<T>` can directly cast the raw C-pointer to our desired type, circumventing the internal proxy interface completely.

Being able to circumvent proxies completely matters a lot. Both `Module::get`, and casting a proxy to `unsafe::Function*`, perform the same task, yet the latter introduces a ~6000% overhead, even though we immediately discard the proxy to `f` after its creation and `static_cast` to our desired type anyway.

In summary, `unsafe` and the C-API are unmatched in performance. To safely access a value, `Module::get` should be preferred. Proxy construction should be avoided in performance-critical code. Returning a value by evaluating a string is always a bad idea.

---

### Benchmark: Mutating Julia-side Variables

We benchmarked **accessing** a variables value, now, we'll turn our attention to **mutating** them. Recall that, in the manual section on proxies, **only named proxies mutate values**.  This is not the only way to mutate a variable by name, however:

```cpp
// number of cycles
size_t n_reps = 10000000;

// initialize variable to be modified
Main.safe_eval("x = 1234");

// C-API: jl_set_global
Benchmark::run_as_base("C-API: set", n_reps, [](){

    Int64 to_box = generate_number<Int64>();
    jl_set_global(jl_main_module, "x"_sym, jl_box_int64(to_box));
});

// unsafe: set_value
Benchmark::run("unsafe: set", n_reps, [](){

    Int64 to_box = generate_number<Int64>();
    unsafe::set_value(jl_main_module, "x"_sym, unsafe::unsafe_box<Int64>(to_box));
});

// Module::assign
Benchmark::run("Module::assign", n_reps / 10, [](){

    Int64 to_box = generate_number<Int64>();
    Main.assign("x", to_box);
});

// Proxy::operator=
auto x_proxy = Main["x"];
Benchmark::run("Proxy::operator=", n_reps / 10, [&](){

    Int64 to_box = generate_number<Int64>();
    x_proxy = to_box;
});
```

Here, we have 4 different ways to mutate our variable, `x`. The C-APIs `jl_set_global` and the `unsafe` libraries `set_value` will most likely be the most performant options. Much more safe and convenient, however, are the familiar `Proxy::operator=` and `Module::assign`.

During each cycle of each benchmark, we generate a random `Int64` using `generate_number`. This function introduces the same amount of overhead anytime it is called. This means, it will slow down all benchmarks by the exact same amount, thereby not invalidating comparison.

### Mutating Julia-side Variables: Results

| name                | median duration (ms) | overhead   |
|---------------------|----------------------|------------|
| `C-API`             | `0.000137ms`         | `0%`       |
| `unsafe::set_value` | `0.000141ms`         | `3%`       |
| `Module::assign`    | `0.000379ms`         | `176%`     |
| `named proxy`       | `0.062295ms`         | `45370.8%` | 


`unsafe` once again makes it below its target 5% goal, much more comfortably this time.

The next-most performant option is `Module::assign`. Like `Module::get`, it introduces an overhead of ~1.5x, which is the cost of safety and exception forwarding. The large gap between it, and `Proxy::operator=` is evident.

Why are named proxies so expensive? During a call to `Proxy::operator=`, the proxy has to:

+ update its pointer
+ add the new value to the GC safeguard
+ evaluate its name
+ update the variable of that name
+ create a new proxy pointing to the new value, then return it

Only the second-to-last of which has to be performed by any of the other functions. This explains the large amount of overhead. Using named proxy to change a variable is a bad idea runtime-wise (even if it is great both syntactically and convenience-wise).

---

### Benchmark: Calling Julia-side Functions

Executing Julia-side code through functions needs to have as little overhead as possible. This section will investigate if / where this was achieved.

```cpp
// number of benchmark cycles
size_t n_reps = 10000000;

// function to test
Main.safe_eval(R"(
    function f()
        sum = 0;
        for i in 1:100
            sum += rand();
        end
    end
)");

// pointer to f
auto* f_ptr = unsafe::get_function(jl_main_module, "f"_sym);

// C-API
Benchmark::run_as_base("C-API: jl_call", n_reps, [&](){
    jl_call0(f_ptr);
});

// unsafe
Benchmark::run("unsafe: call", n_reps, [&](){
    unsafe::call(f_ptr);
});

// jluna::safe_call
Benchmark::run("jluna::safe_call", n_reps, [&](){
    jluna::safe_call(f_ptr);
});

// proxy::safe_call<T>
auto f_proxy = Proxy(f_ptr);
Benchmark::run("Proxy::safe_call<T>", n_reps, [&](){
    f_proxy.safe_call<void>();
});

// proxy::operator()
Benchmark::run("Proxy::operator()", n_reps, [&](){
    f_proxy();
});
``` 

Here, we first access a pointer to our simple Julia-side function, `f`.This function simply adds 100 randomly generated numbers together, introducing the same amount of overhead each time it is run. Because `f` has the signature `() -> void`, no overhead is incurred from boxing / unboxing any arguments or potential results.

### Calling Julia-side Functions: Results

| name                  | median duration (ms) | overhead    |
|-----------------------|----------------------|-------------|
| `C-API`               | `0.000243ms`         | `0%`        |
| `unsafe::call`        | `0.000256ms`         | `5.34%`     |
| `jluna::safe_call`    | `0.000513ms`         | `111%`      | 
| `Proxy::safe_call<T>` | `0.000689ms`         | `184%`      |
| `Proxy::operator()`   | `0.006068ms`         | `2397.12%`  |

`unsafe` barely misses its target 5%, though `0.34%` could very well be due to noise.

Next, in terms of performance, we have `jluna::safe_call`. Unlike the C-API and `unsafe::call`, this function **does perform full exception forwarding**. This process incurs a `111%` overhead, which matches "the cost of safety" in previous sections.

Similar to `Module::get<T>`, `Proxy::safe_call<T>` does not have to construct a proxy of its result. It knows the result should be unboxed to `T`, skipping the proxy construction. This explains the large gap between it and `Proxy::operator()`. The latter of which incurs a large amount of overhead, almost purely due to having to construct a new proxy for its result.

### Benchmark: Calling C++ Functions from Julia

We've seen how fast we can call a Julia-side function from C++, this section will measure how performant the other way around is.

```cpp
// number of cycles
size_t n_reps = 1000000;

// actual C++ function
static auto task_f = [](){
    for (volatile size_t i = 0; i < 10000; i = i+1);
};

// Benchmark: call in C++
Benchmark::run_as_base("Call C++-Function in C++", n_reps, [](){
    task_f();
});

// move to Julia
Main.create_or_assign("task_f", as_julia_function<void()>(task_f));
auto* jl_task_f = unsafe::get_function(jl_main_module, "task_f"_sym);

// Benchmark: call in Julia
Benchmark::run_as_base("Call C++-Function in Julia", n_reps, [&](){
    jl_call0(jl_task_f);
});
```

Here, we are measuring the time it takes for `task_f` to finish when called from either language. `task_f` itself just counts to 10000, where `volatile` assures this happens during each benchmark cycle.

This time, our baseline comparison is not the C-API, as it offers no way of calling C++-functions. Instead, we are comparing executing `task_f` in Julia to executing it natively in C++. The latter of which, represents a 0% overhead scenario. We are using `jl_call0` to call `jl_task_f`, because it most closely matches, how Julia would call any of its functions internally.

### Calling C++ Functions from Julia: Results

| name                | median duration (ms) | overhead    |
|---------------------|----------------------|-------------|
| `called C++-side`   | `0.015963ms`         | `0%`        |
| `called Julia-side` | `0.016145ms`         | `1.14%`     |

Results suggest that there is very little overhead at all, about 1%. Users can be assured, that calling any C++ function moved to Julia via `as_julia_function` is basically just as fast af it was called from pure C++. `as_julia_function` can therefore be used freely in performance-critical code.

---

### Benchmark: Using jluna::Array

In the section on `jluna::Array`, it was asserted as being much faster than `jluna::Proxy`. While this is true, for large arrays, we often need "optimal" performance, not just "better than `jluna::Proxy`" performance. This is potentially achieved by the `unsafe` array interface, which is compared against the C-API and `jluna::Array` here:

```cpp
// number of benchmark cycles
size_t n_reps = 1000000;

// construct with C-API
Benchmark::run_as_base("Allocate Array: C-API", n_reps, [&](){

    // initialize as empty Array{Int64, 1}
    auto* arr = (jl_array_t*) jl_alloc_array_1d(jl_apply_array_type((jl_value_t*) jl_int64_type, 1), 0);
    
    // sizehint! to 1000
    jl_array_sizehint(arr, 1000);
    
    // fill by appending
    for (size_t i = 1; i <= 1000; ++i)
    {
        jl_array_grow_end(arr, 1);
        jl_arrayset(arr, jl_box_int64(generate_number<Int64>()), i);
    }

    // collect garbage to save on RAM
    jl_gc_collect(JL_GC_AUTO);
});

// construct with unsafe library
Benchmark::run("Allocate Array: unsafe", n_reps, [](){

    auto* arr = unsafe::new_array(Int64_t, 0);
    unsafe::sizehint(arr, 1000);

    for (size_t i = 1; i <= 1000; ++i)
        unsafe::push_back(arr, unsafe::unsafe_box<Int64>(generate_number<Int64>()));

    jl_gc_collect(JL_GC_AUTO);
});

// construct with jluna::Array
Benchmark::run("Allocate Array: jluna::Array", n_reps, [&](){

    auto arr = jluna::Vector<Int64>();
    arr.reserve(1000);

    for (size_t i = 1; i <= 1000; ++i)
        arr.push_back(generate_number<Int64>());

    jl_gc_collect(JL_GC_AUTO);
});
```

Here, in each benchmark, we are creating an empty vector, `sizehint!`ing it to 1000, then filling it with 1000 random `Int64`s by appending them one-by-one to the end. This is a common use-case, as it tests both allocation and element-access, giving a good overview of `jluna::Array`s most important features.

For the C-API benchmark only, it was necessary to call `jl_gc_collect`, as otherwise, the GC would have had no opportunity to free arrays before the end of *all* benchmark cycles. This would slowly fill up the ram, ruining the results.

To make for a fair comparison, the overhead of `jl_gc_collect` was introduced to the non C-API benchmark runs as well, even if not technically necessary.

### Using jluna::Array: Results

| name           | median duration (ms) | overhead    |
|----------------|----------------------|-------------|
| `C-API`        | `0.049239ms`         | `0%`        |
| `unsafe`       | `0.049703ms`         | `0.94%`     |
| `jluna::Array` | `0.074531ms`         | `51.3658%`  |

Once again, `unsafe` is very close to the C-API, 1% being far below 5%.

`jluna::Array` is obviously slower. However, unlike what we've seen so far, it only incurs an overhead of 0.5x, compared to the 1x - 2x overhead of previous sections. This is, despite `jluna::Array` also being a proxy. This shows how much more optimized `jluna::Array` is, when compared to a regular proxy.

Whether 0.5x overhead is acceptable depends on the user and application. What is out of the question, though, is that `jluna::Array` offers a very safe and convenient way of interacting with data of any size, type or dimensionality. When we resort to the `unsafe` library, we incur the huge risk inherent to handling raw memory. Sometimes, this risk is a necessary one, however.

---

### Benchmark: Constructing `jluna::Task`

With all the wrapping that needs to take place for any C++-side function to be executable from within a Julia-side task, user may worry about the incurred overhead.

In this benchmark, we are measuring the time it takes to create a `jluna::Task` and execute it until conclusion. Note that each benchmark cycle runs **in sequence**: there is only a single thread available for the entire benchmark. We are not measuring parallel performance, only the overhead of construction.

To make for a fair comparison, a 1-thread, minimal thread pool was implemented using only the standard library. This allows for the overhead of constructing `std::thread` to not affect benchmarks results, as we want to compare jluna task-creation to C++ task-creation, not jluna task-creation to C++ **thread**-creation.

```cpp
// setup 1-thread thread pool
static std::mutex queue_mutex;
static auto queue_lock = std::unique_lock(queue_mutex, std::defer_lock);
static std::condition_variable queue_cv;

static auto queue = std::queue<std::packaged_task<void()>>();
static auto shutdown = false;

static std::mutex master_mutex;
static auto master_lock = std::unique_lock(queue_mutex, std::defer_lock);
static std::condition_variable master_cv;

// worker thread
static auto thread = std::thread([](){

    while (true)
    {
        queue_cv.wait(queue_lock, []() -> bool {
            return not queue.empty() or shutdown;
        });

        if (shutdown)
            return;

        auto task = std::move(queue.front());
        queue.pop();
        task();

        master_cv.notify_all();
    }
});

// task
std::function<void()> task = []() {
    size_t sum = 0;
    for (volatile auto i = 0; i < 100; ++i)
        sum += generate_number<Int64>();

    return sum;
};

// ### BENCHMARK ###

// n cycles
size_t n_reps = 10000000;

// execute task using jluna::Task
Benchmark::run_as_base("threading: jluna::Task", n_reps, [&]()
{
    auto t = ThreadPool::create<void()>(task);
    t.schedule();
    t.join();
});

// execute task using std::thread
Benchmark::run("threading: std::thread", n_reps, [&]()
{
    // queue task
    queue.emplace(std::packaged_task<void()>(task));
    
    // start worker
    queue_cv.notify_all();
    
    // wait for worker to finish before
    // going to the next benchmark cycle
    master_cv.wait(master_lock, [](){
        return queue.empty();
    });
});

// safely shutdown worker
shutdown = true;
queue_cv.notify_all();
```

This benchmark does not measure overhead compared to the C-API, it measures how much faster / slower the `std::` thread pool is at completing a C++-only task, compared to `jluna::ThreadPool`. Considering the C-API more or less forces us to use jlunas thread pool, this is an important statistics.

### Constructing `jluna::Task`: Results

| name          | median duration (ms) | overhead    |
|---------------|----------------------|-------------|
| `jluna::Task` | `0.011592ms`         | `0%`        |
| `std::thread` | `0.012151ms`         | `4.82229%`  |

Results suggest that the `std::` thread pool is actually slightly slower, about 5%. While this is not significant enough to make one thread pool better than the other, it assures users that migrating a parallel architecture from C++-only to jluna will not incur any overhead, making `jluna::ThreadPool` a fully valid replacement in both functionality and performance.

---

### Performance Evaluation: Summary

We have seen that, for any specific task, jluna offers a way to accomplish that task with < 5% overhead, compared to the C-API. Often, this is achieved by the `unsafe` library specifically, which is unattractive to many users due to its missing safety features.

This section will take the benchmark results into account, giving tips on how to achieve the best performance in a way that is **still safe and convenient**. These tips are not meant for users trying to achieve mathematically optimal performance, as it was made clear that in these scenarios, `unsafe` is the go-to choice.

#### Accessing Julia Values

To access the value of something Julia-side, be it a function or the value of a variable, it is best to use `Module::get<T>` wherever possible. Because the function knows what type the return value will be at compile time, it can directly (but safely) access the Julia-state through C, unboxing the value for us without ever going through the `jluna::Proxy` infrastructure.


#### Changing a Julia Value

To mutate a variable, `Module::assign` is the preferred choice. It is much faster than a named proxy, yet still forwards exceptions and automatically boxes C++-side values for us.

#### Creating a Julia Variable

To create a new variable, `Module::create_or_assign` is the most performant option, that still offers safety and automatic boxing. If we specifically want to create a new variable **and** get a named proxy pointing to it, `Module::new_*` is best for this. It simply calls `create_or_assign`, then accesses the variable in the fastest way possible, returning a proxy managing that variable to us.


#### Calling a Julia Function

Unlike many values, Julia Functions usually do not have to be protected from the GC.
This is because most functions are named bindings inside a module (such as `Base.println`). Calling these functions should be done using `jluna::safe_call`. For this function, the user will have to manually box its argument values. In return, they get decent speed and full exception forwarding.

While the function itself may be protected from the GC, the return value of `jluna::safe_call` is not. Because of this, users are encouraged to access the value of a function like so:

```cpp
Int64 return_value = unbox<Int64>(jluna::safe_call(my_function_ptr, arg1, arg2));
```

Here, the GC has no chance to collect the return value of `jluna::safe_call` between its conclusion and the call to `unbox`. This line of code is always memory-safe.

Some functions are not named bindings, however. Temporary functions are best managed by unnamed proxies. While this incurs a significant overhead, in return, we not only don't have to manually box / unbox any arguments, but the proxy makes sure the temporary function is not collected prematurely. We need to use `Proxy::safe_call<T>` (where `T` is the type of the C++-side return value) though, as `Proxy::operator()` incurs a significant performance overhead.

#### Executing Strings as Julia Code

If we really have no choice but to execute a string as Julia code, `jluna::safe_eval` is the function of choice. It has all the bells and whistles `Module::safe_eval` has, except that it returns a raw C-pointer to the result of the code, rather than a proxy. This makes it ideal for this purpose.

Just like `jluna::safe_call`, the following is GC-safe:

```cpp
Int64 variable_value = unbox<Int64>(jluna::safe_eval("length([i for i in 1:43 if i % 2 != 0]"))
```

Here we have a somewhat complex line of code, what it does exactly isn't important. What is important, how is that it returns a value of type `Int64`. We can capture this value safely by making sure that there is no line of code executed between `safe_eval` returning, and the `unbox` call starting. This allows us to safely unbox the result of the `safe_eval` call.

Regardless, running Julia code as strings instead of functions should always be a last resort.

#### Handling Big Arrays

Many common tasks require handling of large arrays. Julia is very adept at this, making it a natural choice for this purpose. When doing the same with jluna, rather than pure Julia, the following should be kept in mind:

+ **Always choose a C-Type as value type**
    - The only way to share arrays losslessly, and without performance overhead, is if it has a value-type that [is interpretable for both C++ and Julia](unsafe.md#shared-memory)
+ **Don't be scared of jluna::Array**
    - `jluna::Array` is decently fast. Most importantly, we can always just access the raw C-pointer it is managing using `data()`, which then lets us operate on it using the `unsafe` library. This is the best compromise between convenience C-like performance.


#### Multi-Threading

Admittedly, the fact C-side threads are inherently incompatible with Julia is a big problem. This is unrelated to jluna, but was attempted to be addressed using `jluna::ThreadPool`.

While C-side threads don't play nice with Julia, Julia-side threads *do* work well in C++. As a `jluna::Task` is also a Julia-side `Base.Task`, we can handle them like any other Julia-side variable, giving us the exact same flexibility offered by pure Julia. Furthermore, any code executed inside a `jluna::Task` can make use of both Julia-side and C++-side synchronization primitives, making it possible to be fit into an already existing parallel architecture.

For any project using jluna, it may be necessary to redesign any particular multi-threaded environment, such that, any concurrent tasks is done Julia-side only. Luckily, this option may actually be faster, and it can make full use of Julia's multi-threading support. This makes the transition as smooth as one could hope for.
