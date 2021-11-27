#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  int Pid();
  std::string User();
  std::string Command();
  float CpuUtilization();
  std::string Ram();
  long int UpTime();
  bool operator<(Process const& a) const;
  float JiffiesLast();

  //Public Mutators
  void Pid(int pid);
  void User(std::string user);
  void Command(std::string command);
  void CpuUtilization(float cpuUtilization);
  void Ram(std::string ram);
  void UpTime(long int upTime);
  void SortIdx(int sortidx);
  void JiffiesLast(float jiffieslast);

 private:
    int pid_{0};
    std::string user_{"u"};
    std::string command_{"c"};
    float cpuUtilization_{0.0};
    std::string ram_{0};
    long int upTime_{0};
    int sortidx_{0};
    float jiffieslast_{0};
};

#endif