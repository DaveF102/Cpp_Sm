#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

Processor& System::Cpu()
{
    LinuxParser::SetCpuIndex(cpuidx_);
    float jactive = LinuxParser::ActiveJiffies();
    float jactivetemp = jactive;    //unchanged copy of active jiffies to save as jactivelast_
    float jidle =  LinuxParser::IdleJiffies();
    float jidletemp = jidle;    //unchanged copy of idle jiffies to save as jidlelast_
    //calculate utilization over refresh time instead of system UpTime
    if (utilizationtime_ == 1)
    {
        jactive = jactive - jactivelast_;
        jidle = jidle - jidlelast_;
    }
    float jtotal = jactive + jidle;
    float cpuUtil = 1.0f - jidle / jtotal;
    cpu_.Utilization(cpuUtil);
    jactivelast_ = jactivetemp;
    jidlelast_ = jidletemp;
    return cpu_; 
}

vector<Process>& System::Processes()
{
    Process tempP;
    //These need to be floats so that small values don't end up as zeroes
    float jiffies;
    float jiffiestemp;
    float jiffieslast;
    float elapsed_sec;
    float hz;
    float cpu_usage;
    vector<float> ti = {0.2f, 0.5f, 1.0f, 2.0f};

    processes_.clear();
    vector<int> vpid = LinuxParser::Pids();
    for (auto pid : vpid)
    {
        tempP.Pid(pid);
        tempP.User(LinuxParser::User(pid));
        tempP.Command(LinuxParser::Command(pid));
        tempP.Ram(LinuxParser::Ram(pid));
        tempP.UpTime(LinuxParser::UpTime(pid));
        tempP.SortIdx(processsortkey_);

        //Calculate CPU Utilization
        jiffies = LinuxParser::ActiveJiffies(pid);
        jiffiestemp = jiffies;
        jiffieslast = 0;
        elapsed_sec = LinuxParser::UpTime(pid);
        hz = sysconf(_SC_CLK_TCK);
        if (utilizationtime_ == 1)
        {
            elapsed_sec = ti[timeinterval_]; 
            for (auto pl : processeslast_)
            {
                if (tempP.Pid() == pl.Pid())
                {
                    jiffieslast = pl.JiffiesLast();
                    break;
                }
            }
        }
        if (elapsed_sec <= 0.0f)
        {
            //prevent division by zero
            cpu_usage = 0.0f; 
        }
        else
        {
            cpu_usage = (((jiffies - jiffieslast) / hz) / elapsed_sec);
        }
        tempP.JiffiesLast(jiffiestemp);
        tempP.CpuUtilization(cpu_usage);

        processes_.emplace_back(tempP);
    }
    //Save copy of unsorted array for next iteration
    processeslast_ = processes_;
    std::sort(processes_.begin(), processes_.end(), Compare);
    return processes_;
}

std::string System::Kernel()
{
    return LinuxParser::Kernel();
}

float System::MemoryUtilization()
{ 
    return LinuxParser::MemoryUtilization(); 
}

std::string System::OperatingSystem()
{
    return LinuxParser::OperatingSystem();
}

int System::RunningProcesses()
{
    return LinuxParser::RunningProcesses(); 
}

int System::TotalProcesses()
{
    return LinuxParser::TotalProcesses(); 
}

long int System::UpTime()
{ 
    return LinuxParser::UpTime();
}


//Helper Functions
int System::CpuQty()
{
    cpuqty_ = LinuxParser::GetCpuQty();
    return cpuqty_;
}

void System::SetCpuIdx(int idx)
{
    cpuidx_ = idx;
}

int System::GetCpuIdx()
{
    return cpuidx_;
}

void System::SetProcessesToShow(int idx)
{
    processestoshow_ = idx;
}

int System::GetProcessesToShow()
{
    return processestoshow_;
}

void System::SetProcessSortKey(int idx)
{
    processsortkey_ = idx;
}

int System::GetProcessSortKey()
{
    return processsortkey_;
}

void System::SetUtilizationTime(int idx)
{
    utilizationtime_ = idx;
}

int System::GetUtilizationTime()
{
    return utilizationtime_;
}

bool System::Compare(const Process a, const Process b)
{
    return b < a;
}

void System::SetTimeInterval(int idx)
{
    timeinterval_ = idx;
}

int System::GetTimeInterval()
{
    return timeinterval_;
}
