// 
// This file contains the code of the section on manual implementation of usertypes in docs/manual.md
//

#include <jluna.hpp>

/// cpp-side implementation of frogs and tadpoles
class Frog
{
    public:
        struct Tadpole
        {
            std::string _name;

            Tadpole()
                : _name("")
            {}

            Frog evolve() const
            {
                if (_name == "")
                    throw std::invalid_argument("tadpole needs to be named before evolving");

                return Frog(_name);
            }
        };

    public:
        std::vector<Frog::Tadpole> spawn(size_t n) const
        {
            std::vector<Frog::Tadpole> out;
            for (size_t i = 0; i < n; ++i)
                out.push_back(Frog::Tadpole());

            return out;
        }

        std::string get_name()
        {
            return _name;
        }

    private:
        Frog(std::string name)
            : _name(name)
        {}

        std::string _name;
};

/// julia-side implementation of frogs and tadpoles
const char* frogs_dot_jl = R"(
    mutable struct Tadpole

        _name::String
        evolve::Function

        Tadpole() = new(
            "",
            (this::Tadpole) -> Frog(this)
        )
    end

    struct Frog

        _name::String
        spawn::Function

        Frog(as_tadpole::Tadpole) = new(
            as_tadpole._name,
            (n::Integer) -> [Tadpole() for _ in 1:n]
        )
    end

    function generate_frog(name::String) ::Frog
        tadpole = Tadpole()
        tadpole._name = name
        return Frog(tadpole)
    end
)";

// box cpp Frog::Tadpole to julia Tadpole
template<Is<Frog::Tadpole> T>
unsafe::Value* box(T in)
{
    auto sentinel = GCSentinel();
    static auto* tadpole_ctor = jl_find_function("Main", "Tadpole");

    auto* out = jluna::safe_call(tadpole_ctor);

    static auto* setfield = jl_find_function("Base", "setfield!");
    static auto field_symbol = Symbol("_name");
    jluna::safe_call(setfield, out, (unsafe::Value*) field_symbol, box<std::string>(in._name));
    return out;
}

// box cpp Frog to julia Frog
template<Is<Frog> T>
unsafe::Value* box(T in)
{
    auto sentinel = GCSentinel();
    static auto* frog_ctor = jl_find_function("Main", "generate_frog");

    auto* out = jluna::safe_call(frog_ctor, box<std::string>(in._name));
    return out;
}

// unbox julia Tadpole to cpp Frog::Tadpole
template<Is<Frog::Tadpole> T>
T unbox(unsafe::Value* in)
{
    auto sentinel = GCSentinel();
    static auto* getfield = jl_find_function("Base", "getfield");
    static auto field_symbol = Symbol("_name");

    unsafe::Value* julia_side_name = jluna::safe_call(getfield, in, (unsafe::Value*) field_symbol);

    auto out = Frog::Tadpole();
    out._name = unbox<std::string>(julia_side_name);
    return out;
}

// unbox julia Frog to cpp Frog
template<Is<Frog> T>
T unbox(unsafe::Value* in)
{
    auto sentinel = GCSentinel();
    static auto* getfield = jl_find_function("Base", "getfield");
    static auto field_symbol = Symbol("_name");

    unsafe::Value* julia_side_name = jluna::safe_call(getfield, in, (unsafe::Value*) field_symbol);

    auto tadpole = Frog::Tadpole();
    tadpole._name = unbox<std::string>(julia_side_name);

    return tadpole.evolve();
}

int main()
{
    State::initialize();

    State::safe_eval(frogs_dot_jl);

    auto cpp_tadpole = Frog::Tadpole();
    cpp_tadpole._name = "Ted";

    State::new_named_undef("jl_tadpole") = box<Frog::Tadpole>(cpp_tadpole);
    State::safe_eval(R"(
        println(jl_tadpole)
        jl_frog = jl_tadpole.evolve(jl_tadpole);
        println(jl_frog)
    )");

    Frog cpp_frog = Main["jl_frog"];
    std::cout << cpp_frog.get_name();

    return 0;
}
