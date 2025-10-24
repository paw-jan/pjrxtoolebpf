#include <filesystem>
#include <string>
#include <unistd.h>    // readlink, getuid
#include <limits.h>    // PATH_MAX
#include <sstream>
#include <iostream>

#include <libevdev/libevdev.h>

#include "event_logger.h"

using namespace std;

class TmpOutputPath
{
public:
    explicit TmpOutputPath()
    {
        generateFilename();
    }

    std::string getFullPath()
    {
        return fullpath_;
    }

private:
    void generateFilename()
    {
        std::filesystem::path baseDir("/tmp");
        char buf[PATH_MAX] = {0};
        ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        // cout<<"JESTEM:  len:  "<<len<<std::endl;
        if (len > 0) 
        {
            buf[len] = '\0';
            const std::filesystem::path exe = std::filesystem::path(buf);
            // cout<<"JESTEM:  exe:  "<<exe<<std::endl;
            for (auto it = exe.begin(); it != exe.end(); ++it) 
            {
                std::string comp = it->string();
                // cout<<"JESTEM:  comp:  "<<comp<<std::endl;
                if (comp.empty() || comp == "/") continue;
                if (comp == "home")
                {
                    auto next = std::next(it);
                    if (next != exe.end()) 
                    {
                        std::string user = next->string();
                        if (!user.empty()) 
                        {
                            baseDir = std::filesystem::path("/") / "home" / user / "tmp";
                        }
                    }
                }
            }
        }

        // cout<<"JESTEM:  baseDir:  "<<baseDir<<std::endl;

        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);
    
        std::ostringstream oss;
        oss << std::put_time(&tm, "%y%m%d_%H%M%S");
        std::string timestamp = oss.str();
    
        std::filesystem::path path(baseDir);
        path /= "umain_kprobe_" + timestamp + ".txt";
    
        fullpath_ =  path.string();
    }

    std::string fullpath_;
};


EventLogger::EventLogger()
{
    TmpOutputPath path;
    std::string filename = path.getFullPath();

    // cout<<"JESTEM:  "<<filename<<std::endl;

    file.open(filename, std::ios::out | std::ios::app);
}

EventLogger::~EventLogger()
{
    if (file.is_open())
        file.close();
}


void EventLogger::logEvent(const event* ev)
{
    string line = em.mapEvent(ev);
    
    if (1 == 0)
    {
        cout << "EV:  "<< line << std::endl;
    }

    if (!file.is_open())
        return;

    file << line << std::endl;
}

