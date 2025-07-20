#ifndef BINANCECLIENT_H
#define BINANCECLIENT_H

#include <memory>
#include <string>
#include <vector>

class BinanceClient {
private:
  class Impl;  // Forward declaration of implementation
  std::unique_ptr<Impl> pImpl;  // Pointer to implementation

public:
  BinanceClient();
  ~BinanceClient();

  // Move constructor/assignment for unique_ptr
  BinanceClient(BinanceClient&&) = default;
  BinanceClient& operator=(BinanceClient&&) = default;

  // Delete copy operations (or implement if needed)
  BinanceClient(const BinanceClient&) = delete;
  BinanceClient& operator=(const BinanceClient&) = delete;

  void setup_websocket();
  void connect();
  void disconnect();
  void subscribe_to_streams(const std::vector<std::string>& streams);
  void unsubscribe_from_streams(const std::vector<std::string>& streams);
  bool is_connected() const;
};

#endif //BINANCECLIENT_H