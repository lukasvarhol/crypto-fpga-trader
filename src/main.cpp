#include "../include/logging/Logger.h"


int main() {
  Logger& logger = Logger::getInstance("../logs/");
  logger.log(info, "data2", "message2");
}
