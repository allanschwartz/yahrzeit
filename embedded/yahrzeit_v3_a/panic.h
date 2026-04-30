// Panic.h
#pragma once

#include <Arduino.h>
#include <Modulino.h>

extern ModulinoPixels statusPixels;

[[noreturn]] void panic(const char *expr, const char *file, int line);

#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            panic(#expr, __FILE__, __LINE__); \
        } \
    } while (0)