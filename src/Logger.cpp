#include "../include/logging/Logger.h"
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>

Logger::Logger(const std::string& dir) {
  directory = dir;
}

void Logger::log(TYPE type, std::string data, std::string message) {
  time_t timestamp; // seconds since 1970
  time(&timestamp); //set current time
  struct tm* timeinfo = localtime(&timestamp);
  char filename[100];
  strftime(filename, sizeof(filename), "%Y-%m-%d.log", timeinfo);
  std::string enum_string = enum_to_string(type);
  std::string file_path = directory+enum_string+"-"+filename;
  std::ofstream myfile (file_path, std::ios::app);
  if (myfile.is_open()) {
    myfile << enum_string << "," << data << ","<< message <<"\n";
    myfile.close();
  }
  else {
    std::cout << "Unable to open file ";
  }
}

Logger& Logger::getInstance(const std::string& dir) {
  static Logger instance(dir);
  return instance;
}

std::string Logger::enum_to_string(TYPE type) {
  switch (type) {
    case info:
      return "info";
    case warning:
      return "warning";
    case error:
      return "error";
    default:
      return "unknown";
  }
}


