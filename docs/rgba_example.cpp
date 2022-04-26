//
// Copyright 2022 Clemens Cords
// Created on 19.04.22 by clem (mail@clemens-cords.com)
//
// this file summarizes all code from the usertype section of the manual
// see: https://github.com/Clemapfel/jluna/blob/master/docs/manual.md#usertypes
//

#include <jluna.hpp>
using namespace jluna;

// define RGBA
struct RGBA
{
    float _red;
    float _green;
    float _blue;
    float _alpha;

    RGBA(float r, float g, float b)
        : _red(r), _green(g), _blue(b), _alpha(1)
    {}

    RGBA()
        : _red(0), _green(0), _blue(0), _alpha(1)
    {}
};

// enable as usertype
set_usertype_enabled(RGBA);

int main()
{
    jluna::initialize();

    // add fields
    Usertype<RGBA>::add_property<float>(
        "_red",
        [](RGBA& in) -> float {return in._red;},
        [](RGBA& out, float in) -> void {out._red = in;}
    );
    Usertype<RGBA>::add_property<float>(
        "_green",
        [](RGBA& in) -> float {return in._green;},
        [](RGBA& out, float in) -> void {out._green = in;}
    );
    Usertype<RGBA>::add_property<float>(
        "_blue",
        [](RGBA& in) -> float {return in._blue;},
        [](RGBA& out, float in) -> void {out._blue = in;}
    );
    Usertype<RGBA>::add_property<float>(
        "_alpha",
        [](RGBA& in) -> float {return in._alpha;},
        [](RGBA& out, float in) -> void {out._alpha = in;}
    );
    Usertype<RGBA>::add_property<float>(
        "_value",
        [](RGBA& in) -> float {
            float max = 0;
            for (auto v : {in._red, in._green, in._blue})
                max = std::max(v, max);
            return max;
        }
    );

    // implement
    Usertype<RGBA>::implement();

    // add additional ctor
    Main.safe_eval(R"(

        function RGBA(r, g, b, a) ::RGBA
            out = RGBA()
            out._red = r
            out._green = g
            out._blue = b
            out._alpha = a
            out._value = max(r, g, b, a)

            return out
        end
    )");

    // usage
    Main.create_or_assign("jl_rgba", RGBA(0.75, 0.5, 0.1));
    Main.safe_eval("println(jl_rgba)");

    RGBA cpp_rgba = Main.safe_eval("return RGBA(0.5, 0.5, 0.3, 1.0)");
    std::cout << "unboxed: "
    std::cout << cpp_rgba._red << " ";
    std::cout << cpp_rgba._green << " ";
    std::cout << cpp_rgba._blue << " ";

    // output:
    //
    // [JULIA][LOG] initialization successful.
    // RGBA(0.75f0, 0.5f0, 0.1f0, 1.0f0, 0.75f0)
    // unboxed: 0.5 0.5 0.3
    // Process finished with exit code 0
    //
    return 0;
}

