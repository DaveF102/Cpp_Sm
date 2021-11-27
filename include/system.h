#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>

#include "process.h"
#include "processor.h"

class System {
 public:
  Processor& Cpu();
  std::vector<Process>& Processes();
  float MemoryUtilization();
  long UpTime();
  int TotalProcesses();
  int RunningProcesses();
  std::string Kernel();
  std::string OperatingSystem();
  int CpuQty();
  void SetCpuIdx(int idx);
  int GetCpuIdx();
  void SetProcessesToShow(int idx);
  int GetProcessesToShow();
  void SetProcessSortKey(int idx);
  int GetProcessSortKey();
  void SetUtilizationTime(int idx);
  int GetUtilizationTime();
  void SetTimeInterval(int idx);
  int GetTimeInterval();
  static bool Compare(const Process a, const Process b);

 private:
  Processor cpu_ = {};
  std::vector<Process> processes_ = {};
  std::vector<Process> processeslast_ = {};
  int cpuqty_;
  int cpuidx_{0};
  int processestoshow_{10};
  int processsortkey_{0};
  int utilizationtime_{0};
  float jactivelast_{0.0f};
  float jidlelast_{0.0f};
  int timeinterval_{2};
};

#endif