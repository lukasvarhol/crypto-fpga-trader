#ifndef LOGGER_H
#define LOGGER_H
#include <string>

enum TYPE{info, warning, error};

class Logger {
  public:
  Logger(const std::string &dir);
  void log(TYPE type, std::string data, std::string message);

private:
  std::string directory;
  std::string enum_to_string(TYPE type);
};



#endif //LOGGER_H
