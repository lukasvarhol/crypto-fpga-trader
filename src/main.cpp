
#include <chrono>
#include <iostream>
#include <thread>
#include "../include/client/BinanceClient.h"

#include "../include/common/CoinManager.h"
#include "../include/logging/Logger.h"

int main() {
  Logger& logger = Logger::getInstance();
  logger.log(warning, "data666", "message3");
  CoinManager coin_manager;


  BinanceClient binance_client(coin_manager);
  coin_manager.set_binance_client(&binance_client);

  binance_client.setup_websocket("wss://stream.binance.com:9443/ws/websocket");

  binance_client.connect();

  bool connected = false;
  int max_wait = 15;
  for (int i = 1; i <= max_wait; ++i) {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (binance_client.is_connected()) {
      std::cout << "Connected after " << i << " seconds" << std::endl;
      connected = true;
      break;
    }
    std::cout << "Waiting... " << i << "/15s" << std::endl;
  }

  if (connected) {
    std::cout << "Subscribing to streams..." << std::endl;
    std::vector<std::string> symbols = {"btcusdt", "ethusdt", "solusdt" };
    coin_manager.add_coins(symbols);

    std::cout << "Listening for 30 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));
  } else {
    std::cout << "Failed to connect within " << max_wait << " seconds." << std::endl;
  }

  return 0;
}
