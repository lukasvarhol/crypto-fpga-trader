#ifndef BINANCECLIENT_H
#define BINANCECLIENT_H

#include <memory>
#include <string>
#include <vector>
#include "../common/CoinManager.h"

class BinanceClient {
private:
  class Impl;
  std::unique_ptr<Impl> pImpl;  // Pointer to implementation
  CoinManager& coin_manager_;

public:
  explicit BinanceClient(CoinManager& manager);
  ~BinanceClient();

  BinanceClient(BinanceClient&&) = default;
  BinanceClient& operator=(BinanceClient&&) = delete;

  BinanceClient(const BinanceClient&) = delete;
  BinanceClient& operator=(const BinanceClient&) = delete;

  void setup_websocket(const std::string& url);
  void connect();
  void disconnect();
  void subscribe_to_streams(const std::vector<std::string>& streams);
  void unsubscribe_from_streams(const std::vector<std::string>& streams);
  [[nodiscard]] bool is_connected() const;
};

#endif //BINANCECLIENT_H
