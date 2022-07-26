//
// Created by clem on 7/25/22.
//

#include <jluna.hpp>
#include <julia.h>



using namespace jluna;
int main()
{
    jluna::initialize(1, 1);
    Base["println"]("hello jluna");

    auto lambda = [](){
        std::cout << "succesful." << std::endl;
    };

    Main.create_or_assign("lambda", as_julia_function<void()>(lambda));

    Main.safe_eval(R"(
        using Distributed;

        @everywhere invoke(f) = f()
        wait(remotecall(invoke, 2, lambda))
    )");
}
