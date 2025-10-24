#pragma once
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <filesystem>

#include "ebpf_kprobe_input_uspace.h"


class EventLogger
{
public:
    explicit EventLogger();
    ~EventLogger();

    void logEvent(const event* ev);

private:
    std::ofstream file;
};
