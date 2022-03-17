//
// this main.cpp was created by jluna/install.sh
//

#include <jluna.hpp>

using namespace jluna;

int main()
{
    State::initialize();
    Base["println"]("hello julia");
}
