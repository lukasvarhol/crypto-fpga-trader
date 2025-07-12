#include "../include/logging/Logger.h"


int main() {
  Logger logger("../logs/");
  logger.log(info, "data2", "message2");

}
