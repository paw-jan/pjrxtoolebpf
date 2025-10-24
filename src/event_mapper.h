#pragma once
#include <string>

#include "ebpf_kprobe_input_uspace.h"


class EventMapper
{
public:
    explicit EventMapper() {};

    std::string mapEvent(const event* ev);

private:
    std::string valueToString(unsigned int type, int value) const;
    std::string eventToLine(unsigned int type, unsigned int code, int value) const;
};
