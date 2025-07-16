#ifndef LOGGER_H
#define LOGGER_H
#include <string>

enum TYPE{info, warning, error};

class Logger {
  public:
  static Logger& getInstance(const std::string& dir = "../logs/");
  void log(TYPE type, std::string data, std::string message);

private:
  Logger(const std::string &dir);
  std::string directory;
  std::string enum_to_string(TYPE type);

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
};

#endif //LOGGER_H
