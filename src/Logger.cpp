#include "../include/logging/Logger.h"
#include <fstream>
#include <iostream>
//#include <json/json.h>
#include <string>

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


