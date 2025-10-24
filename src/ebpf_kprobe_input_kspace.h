#pragma once

#include "vmlinux.h"

struct event
{
    __u64 ts_ns;
    __u32 type;
    __u32 code;
    __s32 value;
    char comm[16];
};
