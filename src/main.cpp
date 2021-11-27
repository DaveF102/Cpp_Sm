#include "ncurses_display.h"
#include "system.h"

int main() {
  System system;
  
  //Get the total number of cpu's and initialize the settings options 
  system.CpuQty();
  NCursesDisplay::InitSettings(system);

  NCursesDisplay::Display(system);
}