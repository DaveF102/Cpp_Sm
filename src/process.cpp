#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

int Process::Pid() { return pid_; }

float Process::CpuUtilization() { return cpuUtilization_; }

string Process::Command() { return command_.substr(0,32); }

string Process::Ram() { return ram_; }

string Process::User() { return user_.substr(0,6); }

long int Process::UpTime() { return upTime_; }

bool Process::operator<(Process const& a) const 
{
    bool rv;
    if (sortidx_ == 0)
    {
        if (LinuxParser::StrToLong(ram_) < LinuxParser::StrToLong(a.ram_)) rv = true;
    }
    if (sortidx_ == 1)
    {
        if (cpuUtilization_ < a.cpuUtilization_) rv = true;
    }
    return rv;
}


//Helper functions
void Process::Pid(int pid) { pid_ = pid; }

void Process::User(std::string user) { user_ = user; }

void Process::Command(std::string command) { command_ = command; }

void Process::CpuUtilization(float cpuUtilization) { cpuUtilization_ = cpuUtilization; }

void Process::Ram(std::string ram) { ram_ = ram; }

void Process::UpTime(long int upTime) { upTime_ = upTime; }

void Process::SortIdx(int sortidx) { sortidx_ = sortidx; }

void Process::JiffiesLast(float jiffieslast) { jiffieslast_ = jiffieslast; }

float Process::JiffiesLast() { return jiffieslast_; }
