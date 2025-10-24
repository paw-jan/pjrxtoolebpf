#pragma once

#include <stdint.h>

struct event
{
    uint64_t ts_ns;
    uint32_t code;
    int32_t value;
    char comm[16];
};