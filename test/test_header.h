#pragma once
#define main app_man
#include "../build_linux.cpp"
#undef main

#include <stdio.h>
#include <assert.h>

#define FAIL 0
#define PASS 1

#define TILE_W 80
#define TILE_H 36

struct data_ll {
    data_ll* next;
    uint8_t pxls[];
};