#ifndef LOGGER_H
#define LOGGER_H
#include <string>


class Logger {
  public:
  void line_getter(const std::string& filename);
  void log_data(std::string type, std::string data, std::string message);
};



#endif //LOGGER_H
