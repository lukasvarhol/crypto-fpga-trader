#include "../include/logging/Logger.h"
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>
#include <filesystem>


void Logger::log(TYPE type, std::string data, std::string message) {
  time_t timestamp; // seconds since 1970
  time(&timestamp); //set current time
  struct tm* timeinfo = localtime(&timestamp);

  char fileDate[20];
  strftime(fileDate, sizeof(fileDate), "%Y-%m-%d", timeinfo);
  std::string dateDir = directory + fileDate + "/";

  if (!std::filesystem::exists(dateDir)) {
    std::filesystem::create_directory(dateDir);
  }
  std::string enum_string = enum_to_string(type);
  std::string filename = enum_string + ".log";
  std::string file_path = dateDir + filename;
  std::ofstream myfile (file_path, std::ios::app);

  if (myfile.is_open()) {
    myfile << enum_string << "," << data << ","<< message <<"\n";
    myfile.close();
  }
  else {
    std::cout << "Unable to open file ";
  }
}


std::string Logger::enum_to_string(TYPE type) {
  switch (type) {
    case info:
      return "info"; //TODO: convention is to make enums all UPPERCASE (refer to best practice page on wiki)
    case warning:
      return "warning";
    case error:
      return "error";
    default:
      return "unknown";
  }
}


