//
// Created by Lukas Varhol on 20/7/2025.
//

#include "../../include/client/BinanceClient.h"
#include <iostream>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class BinanceClient::Impl {
public:
  std::unique_ptr<ix::WebSocket> web_socket;
  long long message_id = 0;
  bool is_connected = false;

  Impl() : web_socket(std::make_unique<ix::WebSocket>()) {}
  ~Impl() {
    if (web_socket) {
      web_socket->stop();
    }
    ix::uninitNetSystem();
  }

  void send_message(const std::string &message);
  void parse_raw_message(const std::string &raw_message);
  void handle_message(const ix::WebSocketMessagePtr &msg);
  void handle_ticker(const json &msg);
  void handle_trade(const json &msg);

  void setup_websocket(const std::string& url) {
    ix::initNetSystem();
    std::cout << "Setting up WebSocket connection to: " << url << std::endl;
    web_socket->setUrl(url);
    web_socket->setHandshakeTimeout(10);
    web_socket->setPingInterval(20);
    web_socket->setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) { handle_message(msg); });
  }
};

BinanceClient::BinanceClient() : pImpl(std::make_unique<Impl>()) {}

BinanceClient::~BinanceClient() = default;

void BinanceClient::setup_websocket(const std::string& url) { pImpl->setup_websocket(url); }

void BinanceClient::connect() {
  std::cout << "Calling web_socket->start()..." << std::endl;
  pImpl->web_socket->start();
  std::cout << "web_socket->start() called" << std::endl;
}

void BinanceClient::disconnect() { pImpl->web_socket->stop(); }

void BinanceClient::subscribe_to_streams(const std::vector<std::string> &subscribe_streams) {
  json subscribe_msg = {{"method", "SUBSCRIBE"}, {"params", subscribe_streams}, {"id", ++pImpl->message_id}};

  pImpl->send_message(subscribe_msg.dump());
}

void BinanceClient::unsubscribe_from_streams(const std::vector<std::string> &unsubscribe_streams) {
  json unsubscribe_msg = {{"method", "UNSUBSCRIBE"}, {"params", unsubscribe_streams}, {"id", ++pImpl->message_id}};

  pImpl->send_message(unsubscribe_msg.dump());
}

bool BinanceClient::is_connected() const {
  return pImpl->is_connected;
}


void BinanceClient::Impl::send_message(const std::string& message) {
  if (is_connected) {
    web_socket->sendText(message);
    std::cout << "Sent: " << message << std::endl;
  } else {
    std::cout << "Not connected, cannot send: " << message << std::endl;
  }
}

void BinanceClient::Impl::parse_raw_message(const std::string &raw_message) {
  if (!json::accept(raw_message)) {
    std::cerr << "Failed to parse message: " << std::endl;
    return;
  }
  json msg = json::parse(raw_message);

  if (msg.is_object() && msg.contains("result") && msg.contains("id")) {
    if (msg["result"].is_null()) {
      std::cout << "Successfully subscribed" << std::endl;
    } else {
      std::cout << "Failed to subscribe" << std::endl;
    }
    return;
  }

  if (msg.is_object() && msg.contains("e")) {
    std::string event_type = msg["e"];

    if (event_type == "trade") {
      handle_trade(msg);
    } else if (event_type == "24hrTicker") {
      handle_ticker(msg);
    }
  }
}

// TODO: once logger is setup convert prints to logs
void BinanceClient::Impl::handle_message(const ix::WebSocketMessagePtr& msg) {
  if (msg->type == ix::WebSocketMessageType::Message) {
    std::cout << "received message: " << msg->str << std::endl;
    parse_raw_message(msg->str);
  } else if (msg->type == ix::WebSocketMessageType::Open) {
    std::cout << "Connection established." << std::endl;
    is_connected = true;
  } else if (msg->type == ix::WebSocketMessageType::Close) {
    std::cout << "Connection closed." << std::endl;
    is_connected = false;
  } else if (msg->type == ix::WebSocketMessageType::Error) {
    std::cout << "WebSocket message error." << msg->errorInfo.reason << std::endl;
    is_connected = false;
  } else if (msg->type == ix::WebSocketMessageType::Ping) {
    std::cout << "Ping received." << std::endl;
  }
}

void BinanceClient::Impl::handle_ticker(const json &msg) {
  if (!msg.is_object() || !msg.contains("s") || !msg.contains("c") || !msg.contains("p") || !msg.contains("P") ||
      !msg.contains("h") || !msg.contains("l") || !msg.contains("v")) {
    std::cerr << "Missing required ticker fields" << std::endl;
    return;
  }

  std::string symbol = msg["s"];
  std::string price = msg["c"];
  std::string change = msg["p"];
  std::string change_percent = msg["P"];
  std::string high = msg["h"];
  std::string low = msg["l"];
  std::string volume = msg["v"];

  std::cout << "\n" << symbol << " TICKER" << std::endl;
  std::cout << "Price: $" << price << std::endl;
  std::cout << "24h Change: " << change << " (" << change_percent << "%)" << std::endl;
  std::cout << "24h High: $" << high << std::endl;
  std::cout << "4h Low: $" << low << std::endl;
  std::cout << "Volume: " << volume << std::endl;
  std::cout << "----------------------------------------" << std::endl;
}

void BinanceClient::Impl::handle_trade(const json &msg) {
  if (!msg.is_object() || !msg.contains("s") || !msg.contains("t") || !msg.contains("p") ||
    !msg.contains("q") || !msg.contains("T") || !msg.contains("m")) {
    std::cerr << "Missing required ticker fields" << std::endl;
    return;
    }

  std::string symbol = msg["s"];
  long trade_id = msg["t"];
  std::string price = msg["p"];
  std::string quantity = msg["q"];
  long trade_time = msg["T"];
  bool is_market_maker = msg["m"];

  std::cout << "\n" << symbol << " TRADE" << std::endl;
  std::cout << "Symbol: " << symbol << std::endl;
  std::cout << "Price: $" << price << std::endl;
  std::cout << "Trade ID: " << trade_id << std::endl;
  std::cout << "Quantity: " << quantity << std::endl;
  std::cout << "Trade Time: " << trade_time << std::endl;
  std::cout << "Buyer is Market Maker: " << is_market_maker << std::endl;
}

