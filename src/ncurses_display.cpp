#include <curses.h>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "format.h"
#include "ncurses_display.h"
#include "system.h"

using std::string;
using std::to_string;
using std::vector;
/*  Add user interaction
      - Show current settings of 5 categories in yellow text
      - Press 's' to pause refresh and open "settings_window"
      - Change category using left/right arrow keys
      - Change value of selected category using up/down arrow keys
      - Press 'Enter' to:
        - accept changes
        - close settings window
        - restart periodic refresh
*/

//'settings' - 2d vector of strings containing text descriping each setting
vector<vector<string>> settings;
//'values' - 2d vector of ints containing values some settings need
//  - Tenths of seconds for "Refresh Interval"
//  - Number of processes for "Processes to Show"
vector<vector<int>> values;
//'idx' - initial index of selection for each category
vector<int> idx = {2, 0, 2, 1, 1};
//'cats' - category names
vector<string> cats = {"Refresh Interval", "CPU's", "Processes to Show", "Process Sort Key", "Utilization Time"};
//'maxidx' - number of choices for each category
//  InitSettings() populates extra vector elements with "empty"
vector<int> maxidx = {0, 0, 0, 0, 0};

// 50 bars uniformly displayed from 0 - 100 %
// 2% is one bar(|)
std::string NCursesDisplay::ProgressBar(float percent) {
  std::string result{"0%"};
  int size{50};
  float bars{percent * size};

  for (int i{0}; i < size; ++i) {
    result += i <= bars ? '|' : ' ';
  }

  string display{to_string(percent * 100).substr(0, 4)};
  if (percent < 0.1 || percent == 1.0)
    display = " " + to_string(percent * 100).substr(0, 3);
  return result + " " + display + "/100%";
}

void NCursesDisplay::DisplaySystem(System& system, WINDOW* window) {
  int row{0};
  int pluscol = 35;
  string plus = "";
  mvwprintw(window, ++row, 2, ("OS: " + system.OperatingSystem()).c_str());
  plus = "press 's' to change settings";
  wattron(window, COLOR_PAIR(3));
  mvwprintw(window, row, pluscol, plus.c_str());
  wattroff(window, COLOR_PAIR(3));
  mvwprintw(window, ++row, 2, ("Kernel: " + system.Kernel()).c_str());
  wattron(window, COLOR_PAIR(3));
  mvwprintw(window, ++row, 2, (settings[1][idx[1]] + ":").c_str());
  wattroff(window, COLOR_PAIR(3));
  wattron(window, COLOR_PAIR(1));
  mvwprintw(window, row, 10, "");
  wprintw(window, ProgressBar(system.Cpu().Utilization()).c_str());
  wattroff(window, COLOR_PAIR(1));
  mvwprintw(window, ++row, 2, "Memory: ");
  wattron(window, COLOR_PAIR(1));
  mvwprintw(window, row, 10, "");
  wprintw(window, ProgressBar(system.MemoryUtilization()).c_str());
  wattroff(window, COLOR_PAIR(1));
  mvwprintw(window, ++row, 2,
            ("Total Processes: " + to_string(system.TotalProcesses())).c_str());
  plus = cats[0] + ": " + settings[0][idx[0]] + "   |   Show: " + settings[2][idx[2]];
  wattron(window, COLOR_PAIR(3));
  mvwprintw(window, row, pluscol, plus.c_str());
  wattroff(window, COLOR_PAIR(3));
  mvwprintw(
      window, ++row, 2,
      ("Running Processes: " + to_string(system.RunningProcesses())).c_str());
  plus = cats[3] + ": " + settings[3][idx[3]];
  wattron(window, COLOR_PAIR(3));
  mvwprintw(window, row, pluscol, plus.c_str());
  wattroff(window, COLOR_PAIR(3));
  mvwprintw(window, ++row, 2,
            ("Up Time: " + Format::ElapsedTime(system.UpTime())).c_str());
  plus = cats[4] + ": " + settings[4][idx[4]];
  wattron(window, COLOR_PAIR(3));
  mvwprintw(window, row, pluscol, plus.c_str());
  wattroff(window, COLOR_PAIR(3));
  wrefresh(window);
}

void NCursesDisplay::DisplayProcesses(std::vector<Process>& processes,
                                      WINDOW* window, int n) {
  int row{0};
  int const pid_column{2};
  int const user_column{9};
  int const cpu_column{16};
  int const ram_column{26};
  int const time_column{35};
  int const command_column{46};
  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, pid_column, "PID");
  mvwprintw(window, row, user_column, "USER");
  mvwprintw(window, row, cpu_column, "CPU[%%]");
  mvwprintw(window, row, ram_column, "RAM[MB]");
  mvwprintw(window, row, time_column, "TIME+");
  mvwprintw(window, row, command_column, "COMMAND");
  wattroff(window, COLOR_PAIR(2));
  for (int i = 0; i < n; ++i) {
    //You need to take care of the fact that the cpu utilization has already been multiplied by 100.
    // Clear the line
    mvwprintw(window, ++row, pid_column, (string(window->_maxx-2, ' ').c_str()));
    
    mvwprintw(window, row, pid_column, to_string(processes[i].Pid()).c_str());
    mvwprintw(window, row, user_column, processes[i].User().c_str());
    float cpu = processes[i].CpuUtilization() * 100;
    mvwprintw(window, row, cpu_column, to_string(cpu).substr(0, 4).c_str());
    mvwprintw(window, row, ram_column, processes[i].Ram().c_str());
    mvwprintw(window, row, time_column,
              Format::ElapsedTime(processes[i].UpTime()).c_str());
    mvwprintw(window, row, command_column,
              processes[i].Command().substr(0, window->_maxx - 46).c_str());
  }
}

void NCursesDisplay::Display(System& system, int n) {
  initscr();      // start ncurses
  noecho();       // do not print input values
  cbreak();       // terminate ncurses on ctrl + c
  start_color();  // enable color

  halfdelay(values[0][idx[0]]);

  int x_max{getmaxx(stdscr)};
  int ch;
  WINDOW* system_window = newwin(9, x_max - 1, 0, 0);
  WINDOW* process_window =
      newwin(3 + n, x_max - 1, system_window->_maxy + 1, 0);

  while (1) {
    ch = wgetch(system_window);
    //Look for 's' character to open settings window
    if (ch == 115)
    {
      SetingsMenu(system);
      //clear;
      halfdelay(values[0][idx[0]]);
      int n_old = n;
      n = system.GetProcessesToShow();
      if (n != n_old)
      {
        werase(process_window);
        wrefresh(process_window);
        wresize(process_window, 3 + n, x_max - 1);
      }
    }
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    box(system_window, 0, 0);
    box(process_window, 0, 0);
    DisplaySystem(system, system_window);
    DisplayProcesses(system.Processes(), process_window, n);
    wrefresh(system_window);
    wrefresh(process_window);
    refresh();
    //Replaced this with wgetch
    //std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  endwin();
}

void NCursesDisplay::InitSettings(System& system)
{
  //This is called by main.cpp
  uint maxsize = system.CpuQty();  //Number of CPU data lines, including all cpus
  uint indcpus = maxsize - 1;      //Individual CPUs
  uint i;
  vector<int> valtemp;
  if (maxsize < 4) maxsize = 4;
  for (i = 0; i < maxsize; i++) valtemp.emplace_back(i);
  for (i = 0; i < 5; i++) values.emplace_back(valtemp);
  //Refresh Intervals
  vector<string> vsis = {"0.2 sec", "0.5 sec", "1.0 sec", "2.0 sec"};
  for (i = 0; i < maxsize - vsis.size(); i++) vsis.emplace_back("empty");
  values[0][0] = 2;
  values[0][1] = 5;
  values[0][2] = 10;
  values[0][3] = 20;
  maxidx[0] = 3;
  //CPUs
  vector<string> vcpus = {"CpuAll"};
  for (i = 0; i < indcpus; i++) vcpus.emplace_back("Cpu" + to_string(i) + "  ");
  for (i = 0; i < maxsize - vcpus.size(); i++) vcpus.emplace_back("empty");
  maxidx[1] = indcpus;
  //Number of Processes to view
  vector<string> cprocs = {"3 processes ", "5 processes ", "10 processes"};
  for (i = 0; i < maxsize - cprocs.size(); i++) cprocs.emplace_back("empty");
  values[2][0] = 3;
  values[2][1] = 5;
  values[2][2] = 10;
  maxidx[2] = 2;
  //Process sort key
  vector<string> vsorts = {"Memory Utilization", "CPU Utilization   "};
  for (i = 0; i < maxsize - vsorts.size(); i++) vsorts.emplace_back("empty");
  maxidx[3] = 1;
  //Utilization Time
  vector<string> vuts = {"System UpTime   ", "Refresh Interval"};
  for (i = 0; i < maxsize - vuts.size(); i++) vuts.emplace_back("empty");
  maxidx[4] = 1;

  settings.emplace_back(vsis);
  settings.emplace_back(vcpus);
  settings.emplace_back(cprocs);
  settings.emplace_back(vsorts);
  settings.emplace_back(vuts);

  system.SetTimeInterval(idx[0]);
  system.SetCpuIdx(values[1][idx[1]]);
  system.SetProcessesToShow(values[2][idx[2]]);
  system.SetProcessSortKey(values[3][idx[3]]);
  system.SetUtilizationTime(values[4][idx[4]]);
}

void NCursesDisplay::SetingsMenu(System& system)
{
  nocbreak();
  initscr();      // start ncurses
  noecho();       // do not print input values
  cbreak();       // terminate ncurses on ctrl + c
  start_color();  // enable color

  int ch;
  int cat = 0;

  WINDOW *settings_window = newwin(7, 40, 5, 10); //newwin(int nlines, int ncols, int begin_y, int begin_x);
  keypad(settings_window, TRUE);
  box(settings_window, 0, 0);
  wattron(settings_window, COLOR_PAIR(3));
  mvwprintw(settings_window, 1, 2, "L-R arrows change category");
  mvwprintw(settings_window, 2, 2, "U-D arrows change setting");
  mvwprintw(settings_window, 3, 2, "Enter to go back to Process Monitor");
  mvwprintw(settings_window, 4, 4, ("Category: " + cats[cat]).c_str());
  mvwprintw(settings_window, 5, 4, ("Value   : " + settings[cat][idx[cat]]).c_str());
  wattroff(settings_window, COLOR_PAIR(3));
  wrefresh(settings_window);
  while (1)
  {
    ch = wgetch(settings_window);
    //Exit loop
    if (ch == 10) break;
    //Adjust Cat up
    if (ch == 260)
    {
      cat--;
      if (cat < 0) cat = 4;
    }
    //Adjust Cat down
    if (ch == 261)
    {
      cat++;
      if (cat > 4) cat = 0;
    }
    //Adjust Value up
    if (ch == 258)
    {
      idx[cat]--;
      if (idx[cat] < 0) idx[cat] = maxidx[cat];
    }
    //Adjust Value down
    if (ch == 259)
    {
      idx[cat]++;
      if (idx[cat] > maxidx[cat]) idx[cat] = 0;
    }
    system.SetTimeInterval(idx[0]);
    system.SetCpuIdx(values[1][idx[1]]);
    system.SetProcessesToShow(values[2][idx[2]]);
    system.SetProcessSortKey(values[3][idx[3]]);
    system.SetUtilizationTime(values[4][idx[4]]);

    mvwprintw(settings_window, 4, 4, "                                 ", ch);
    mvwprintw(settings_window, 4, 4, ("Category: " + cats[cat]).c_str());
    mvwprintw(settings_window, 5, 4, "                                 ", ch);
    mvwprintw(settings_window, 5, 4, ("Value   : " + settings[cat][idx[cat]]).c_str());
    wrefresh(settings_window);
  }
  endwin();
}