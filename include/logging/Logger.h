#ifndef LOGGER_H
#define LOGGER_H
#include <string>

#define LOG logger::getInstance() // it is cleaner to define a macro like this

enum TYPE{info, warning, error};

class Logger {
private:
  std::string directory;
  Logger(const std::string& dir = "../logs/"): directory(dir) {}

public:
  Logger(const Logger& obj) = delete;
  Logger& operator=(const Logger& obj) = delete;

  static Logger& getInstance(const std::string& dir = "../logs/") {
    static Logger instance(dir);
    return instance;
  }

  void log(TYPE type, std::string data, std::string message);

private:
  std::string enum_to_string(TYPE type);
};

#endif //LOGGER_H
