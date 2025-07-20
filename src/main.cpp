
#include <iostream>
#include "../include/client/BinanceClient.h"
#include <thread>
#include <chrono>

int main() {

  BinanceClient client;

  client.setup_websocket();

  client.connect();

  bool connected = false;
  int max_wait = 15;
  for (int i = 1; i <= max_wait; ++i) {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (client.is_connected()) {
      std::cout << "Connected after " << i << " seconds" << std::endl;
      connected = true;
      break;
    }
    std::cout << "Waiting... " << i << "/15s" << std::endl;
  }

  if (connected) {
    std::cout << "Subscribing to streams..." << std::endl;
    std::vector<std::string> streams = {
      "btcusdt@ticker",
      "btcusdt@trade"
  };
    client.subscribe_to_streams(streams);

    std::cout << "Listening for 30 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));
  } else {
    std::cout << "Failed to connect within "<< max_wait << " seconds." << std::endl;
  }

  client.disconnect();
  return 0;
}
