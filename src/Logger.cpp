#include "../include/logging/Logger.h"
#include <fstream>
#include <iostream>
//#include <json/json.h>
#include <string>
#include <ctime>

void Logger::log_data(std::string type, std::string data, std::string message) {
  time_t timestamp; // seconds since 1970
  time(&timestamp);
  struct tm* timeinfo = localtime(&timestamp);
  char filename[100];
  strftime(filename, sizeof(filename), "%Y-%m-%d.log", timeinfo);
  std::cout << ctime(&timestamp);
  std::ofstream myfile (type+"-"+filename, std::ios::app);
  if (myfile.is_open()) {
    myfile << type << "," << data << ","<< message <<"\n";
    myfile.close();
  }
  else {
    std::cout << "Unable to open file ";
  }
}


