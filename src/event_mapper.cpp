#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <libevdev/libevdev.h>

#include "event_mapper.h"

std::string EventMapper::mapEvent(const event* ev)
{
    std::string line = eventToLine(ev->type, ev->code, ev->value);
    return line;
}

std::string EventMapper::valueToString(unsigned int type, int value) const
{
    // EV_KEY semantics
    if (type == EV_KEY) {
        if (value == 0) return "RELEASE";
        if (value == 1) return "PRESS";
        if (value == 2) return "REPEAT";
        return std::to_string(value);
    }
    // EV_REL/EV_ABS return numeric value
    if (type == EV_REL || type == EV_ABS) {
        return std::to_string(value);
    }
    // default fallback
    return std::to_string(value);
}

std::string EventMapper::eventToLine(unsigned int type, unsigned int code, int value) const
{
    const char *type_name = libevdev_event_type_get_name(type);
    const char *code_name = libevdev_event_code_get_name(type, code);
    std::string val = valueToString(type, value);

    std::ostringstream oss;
    std::string name = code_name ? code_name : ("CODE_" + std::to_string(code));
    oss << std::left << std::setw(32) << name << val;
    return oss.str();
}    
