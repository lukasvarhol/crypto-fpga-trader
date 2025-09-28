#include "../include/logging/Logger.h"

int main() {
  Logger& logger = Logger::getInstance();
  logger.log(warning, "data3", "message3");
  return 0;
}
