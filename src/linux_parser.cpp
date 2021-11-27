#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

vector<string> vsj; //vector string of system jiffies from selected cpu
vector<float> vfj; //vector float of system jiffies
int cpuidx;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization()
{
  //https://stackoverflow.com/questions/41224738/how-to-calculate-system-memory-usage-from-proc-meminfo-like-htop/41251290#41251290
  //  post by Hisham H M - used to calculate total used memory
  float rv = -1.0f;
  float vMemTotal;
  float vMemFree;
  //These two are not required
  //float vMemAvailable;
  //float vBuffers;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) 
  {
    vMemTotal = StrToFloat(TokenByNum(stream, 0, 1));
    vMemFree = StrToFloat(TokenByNum(stream, 1, 1));
    //vMemAvailable = StrToFloat(TokenByNum(stream, 2, 1));
    //vBuffers = StrToFloat(TokenByNum(stream, 3, 1));
    rv = (1.0f - vMemFree / vMemTotal);
    return rv;
  }
  return rv;
}

long LinuxParser::UpTime()
{
  long rv = -1; 
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) 
  {
    rv = StrToLong(TokenByNum(stream, 0, 0));
    return rv;
  }
  return rv;
}

long LinuxParser::Jiffies()
{
  float totaljiffies = ActiveJiffies() + IdleJiffies();
  return totaljiffies;
}

long LinuxParser::ActiveJiffies(int pid)
{
  long rv = 0;
  string pdir = kProcDirectory + std::to_string(pid) + "/";
  std::ifstream stream(pdir + kStatFilename);
  if (stream.is_open()) 
  {
    long utime = StrToLong(TokenByNum(stream, 0, 13));
    long stime = StrToLong(TokenByNum(stream, 0, 14));
    long cutime = StrToLong(TokenByNum(stream, 0, 15));
    long cstime = StrToLong(TokenByNum(stream, 0, 16));
    rv = utime + stime + cutime + cstime;
  }
  return rv;
}

long LinuxParser::ActiveJiffies()
{
  //https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
  //  answer by Vangelis Tasoulas - used to calculate non-idle CPU usage
  //float jactive = vfj[0] + vfj[1] + vfj[2] + vfj[5] + vfj[6] + vfj[7];
  float jactive = vfj[kUser_] + vfj[kNice_] + vfj[kSystem_] + vfj[kIRQ_] + vfj[kSoftIRQ_] + vfj[kSteal_];
  return jactive;
}

long LinuxParser::IdleJiffies()
{
  //https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
  //  answer by Vangelis Tasoulas - used to calculate idle CPU usage
  //float jidle =  vfj[3] + vfj[4];
  float jidle =  vfj[kIdle_] + vfj[kIOwait_];
  return jidle;
}

vector<string> LinuxParser::CpuUtilization()
{
  vector<string> rv;
  vector<long> vtemp;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) 
  {
    for (int i = 1; i <= 10; i++)
    {
      rv.emplace_back(TokenByNum(stream, cpuidx, i));
    }
  }
  return rv;
}

int LinuxParser::TotalProcesses()
{
  int rv = -1;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) 
  {
    rv = StrToInt(TokenAfterStr(stream, "processes"));
    return rv;
  }
  return rv;
}

int LinuxParser::RunningProcesses()
{
  int rv = -1;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) 
  {
    rv = StrToInt(TokenAfterStr(stream, "procs_running"));
    return rv;
  }
  return rv;
  }

string LinuxParser::Command(int pid)
{
  string line = "";
  string pdir = kProcDirectory + std::to_string(pid) + "/";
  std::ifstream stream(pdir + kCmdlineFilename);
  if (stream.is_open()) 
  {
    std::getline(stream, line);
  }
  return line;
}

string LinuxParser::Ram(int pid)
{
  string rv = "";
  string pdir = kProcDirectory + std::to_string(pid) + "/";
  std::ifstream stream(pdir + kStatusFilename);
  if (stream.is_open()) 
  {
    rv = TokenAfterStr(stream, "VmSize:");
    return rv;
  }
  return rv;
}

string LinuxParser::Uid(int pid)
{
  string uid = "";
  string pdir = kProcDirectory + std::to_string(pid) + "/";
  std::ifstream stream(pdir + kStatusFilename);
  if (stream.is_open()) 
  {
    uid = TokenAfterStr(stream, "Uid:");
  }
  return uid;
}

string LinuxParser::User(int pid)
{
  string user = "";
  string test;
  string uid = LinuxParser::Uid(pid);

  std::ifstream stream2(kPasswordPath);
  if (stream2.is_open())
  {
    test = ":x:" + uid + ":";
    user = TokenBeforeStr(stream2, test);
  }
  return user;
}

long LinuxParser::UpTime(int pid)
{
  long elapsed_sec = -1;
  string pdir = kProcDirectory + std::to_string(pid) + "/";
  std::ifstream stream(pdir + kStatFilename);
  if (stream.is_open()) 
  {
    long hz = sysconf(_SC_CLK_TCK);
    long starttime = StrToLong(TokenByNum(stream, 0, 21));
    elapsed_sec = LinuxParser::UpTime() - (starttime / hz);
  }
  return elapsed_sec;
}

//Helper functions
string LinuxParser::TokenByNum(std::ifstream& strm, long lineNum, long tokenNum)
{
  //Return the tokenNum'th token in the lineNum'th line of the stream
  long adjtokenNum;
  string line;
  string rv = "nf1";
  //Make sure to start at the beginning of the stream each time
  strm.seekg(0, strm.beg);
  //Initial line in stream is lineNum = 0
  for (int i = 0; i <= lineNum; i++)
  {
    std::getline(strm, line);
  }
  std::istringstream linestream(line);
  //Initial token in line is tokenNum = 1
  //  So use adjtokenNum to target counter j
  //  So that both function arguments are zero based to user 
  adjtokenNum = tokenNum + 1;
  for (int j = 0; j < adjtokenNum; j++)
  {
    linestream >> rv;
  }
  return rv;
}

string LinuxParser::TokenAfterStr(std::ifstream& strm, std::string lineStartString)
{
  bool foundStartString = false;
  string line;
  string testString;
  string nextString = "nf2";
  while (foundStartString == false && strm.eof() == false)
  {
    std::getline(strm, line);
    std::istringstream linestream(line);
    linestream >> testString >> nextString;
    if (testString == lineStartString) foundStartString = true;
  }
  return nextString;
}

string LinuxParser::TokenBeforeStr(std::ifstream& strm, std::string specifiedString)
{
  string rv = "nf3";
  bool foundSpecifiedString = false;
  string line;
  size_t pos;
  while (foundSpecifiedString == false && strm.eof() == false)
  {
    std::getline(strm, line);
    pos = line.find(specifiedString);
    if (pos > 0)
    {
      rv = line.substr(0, pos);
      foundSpecifiedString = true;
    }
  }
  return rv;
}

int LinuxParser::GetCpuQty()
{
  int rv = -1;
  string strStart;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) 
  {
    int i = 0;
    strStart = TokenByNum(stream, i, 0);
    while (strStart.substr(0, 3) == "cpu")
    {
      i++;
      strStart = TokenByNum(stream, i, 0);
    }
    rv = i;
  }
  return rv;
}

void LinuxParser::SetCpuIndex(int idx)
{
  cpuidx = idx;
  vsj = CpuUtilization();
  vfj.clear();
  for (int i = 0; i < 10; i++) vfj.emplace_back(LinuxParser::StrToFloat(vsj[i]));
}

int LinuxParser::StrToInt(std::string strNum)
{
  return std::stoi(strNum);
}

long LinuxParser::StrToLong(std::string strNum)
{
  const char *c = strNum.c_str();
  char *stopstring;
  return strtol(c, &stopstring, 10);
}

float LinuxParser::StrToFloat(std::string strNum)
{
  const char *c = strNum.c_str();
  char *stopstring;
  return strtof(c, &stopstring);
}
