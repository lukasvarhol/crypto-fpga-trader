#include "../include/logging/Logger.h"
#include <fstream>
#include <iostream>
//#include <json/json.h>
#include <string>
#include <ctime>

void Logger::line_getter(const std::string& FILENAME) {
  std::ifstream myfile (FILENAME); // instance of ifstream object
  if (myfile.is_open()) {
    std::string line;
    while (std::getline (myfile, line)) {
      std::cout << line << "\n";
    }
    myfile.close();
  }
  else {
    std::cout << "Unable to open file ";
  }
}

void json_parser(const std::string& DATA) { //using const std::string& so the data doesn't get copied but the incoming version gets changed into a json values

}

void Logger::log_data(std::string type, std::string data, std::string message) {
  time_t timestamp;
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


