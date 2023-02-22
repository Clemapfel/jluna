#pragma once

#ifdef _WIN32
// julia.h includes Windows headers that are not self contained. This wrapper
// header includes Windows.h before including julia.h, so everything is setup
// for the other headers to actually work.

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#endif

#include <julia.h>
#include <julia_gcext.h>
#include <julia_threads.h>

