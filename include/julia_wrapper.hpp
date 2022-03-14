#pragma once

#ifdef _WIN32
/* julia.h includes Windows headers that are not self contained, so this
 * wrapper header includes Windows.h before includeing julia.h, so everything
 * is setup for the other headers to actually work */
# define NOMINMAX
# define WIN32_LEAN_AND_MEAN
# include <Windows.h>
#endif

#include <julia.h>
