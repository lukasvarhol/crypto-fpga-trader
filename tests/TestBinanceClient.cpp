//
// Created by Lukas Varhol on 20/7/2025.
//

#include <gtest/gtest.h>
#include "../include/client/BinanceClient.h"
#include <thread>
#include <chrono>
#include <atomic>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class BinanceClientTest : public ::testing::Test {
protected:
  void SetUp() override {
    client = std::make_unique<BinanceClient>();
  }

  void TearDown() override {
    if (client) {
      client->disconnect();

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      client.reset();
    }
  }

  std::unique_ptr<BinanceClient> client;

  bool wait_for_connection(int timeout_seconds = 10) {
    for (int i = 0; i < timeout_seconds; ++i) {
      if (client && client->is_connected()) {
        return true;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return false;
  }
};

TEST_F(BinanceClientTest, ClientCreationAndDestruction) {
  EXPECT_NE(client, nullptr);
  EXPECT_FALSE(client->is_connected());
}

TEST_F(BinanceClientTest, WebSocketSetup) {
  EXPECT_NO_THROW(client->setup_websocket("wss://echo.websocket.org"));
  EXPECT_FALSE(client->is_connected());
}

TEST_F(BinanceClientTest, EchoServerConnection) {
  client->setup_websocket("wss://echo.websocket.org");
  client->connect();

  bool connected = wait_for_connection(15);
  EXPECT_TRUE(connected) << "Failed to connect to websocket server within timeout";

  if (connected) {
    client->disconnect();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

TEST_F(BinanceClientTest, BinanceWebSocketConnection) {
  client->setup_websocket("wss://stream.binance.com:9443/ws/websocket");
  client->connect();

  bool connected = wait_for_connection(15);
  EXPECT_TRUE(connected) << "Failed to connect to websocket server within timeout";

  if (connected) {
    client->disconnect();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

TEST_F(BinanceClientTest, StreamSubscription) {
  std::string url = "wss://stream.binance.com:9443/ws/websocket";
  client->setup_websocket(url);
  client->connect();

  ASSERT_TRUE(wait_for_connection(15)) << "Failed to establish connection for subscription test";

  std::vector<std::string> streams = {"btcusdt@ticker"};
  EXPECT_NO_THROW(client->subscribe_to_streams(streams));

  std::this_thread::sleep_for(std::chrono::seconds(3));

  EXPECT_NO_THROW(client->unsubscribe_from_streams(streams));
}

TEST_F(BinanceClientTest, MultipleStreamSubscription) {
  std::string url = "wss://stream.binance.com:9443/ws/websocket";
  client->setup_websocket(url);
  client->connect();

  ASSERT_TRUE(wait_for_connection(15)) << "Failed to establish connection";

  std::vector<std::string> streams = {
    "btcusdt@ticker",
    "btcusdt@trade",
    "ethusdt@ticker"
};

  EXPECT_NO_THROW(client->subscribe_to_streams(streams));

  std::this_thread::sleep_for(std::chrono::seconds(5));

  EXPECT_NO_THROW(client->unsubscribe_from_streams(streams));
}

TEST_F(BinanceClientTest, ConnectionStateManagement) {
  std::string url = "wss://stream.binance.com:9443/ws/websocket";

  EXPECT_FALSE(client->is_connected());

  client->setup_websocket(url);
  EXPECT_FALSE(client->is_connected());

  client->connect();
  bool connected = wait_for_connection(15);
  EXPECT_TRUE(connected);
  EXPECT_TRUE(client->is_connected());

  client->disconnect();

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(BinanceClientTest, Reconnection) {
  std::string url = "wss://stream.binance.com:9443/ws/websocket";

  // First connection
  client->setup_websocket(url);
  client->connect();
  ASSERT_TRUE(wait_for_connection(15));

  // Disconnect
  client->disconnect();
  std::this_thread::sleep_for(std::chrono::seconds(1));

  client->setup_websocket(url);
  client->connect();
  bool reconnected = wait_for_connection(15);
  EXPECT_TRUE(reconnected) << "Failed to reconnect";
}

TEST_F(BinanceClientTest, InvalidStreamNames) {
  std::string url = "wss://stream.binance.com:9443/ws/websocket";
  client->setup_websocket(url);
  client->connect();
  ASSERT_TRUE(wait_for_connection(15));

  std::vector<std::string> invalid_streams = {
    "invalid@stream",
    "nonexistent@ticker",
    ""
};

  EXPECT_NO_THROW(client->subscribe_to_streams(invalid_streams));
  std::this_thread::sleep_for(std::chrono::seconds(2));
}

TEST_F(BinanceClientTest, EmptyStreamList) {
  std::string url = "wss://stream.binance.com:9443/ws/websocket";
  client->setup_websocket(url);
  client->connect();
  ASSERT_TRUE(wait_for_connection(15));

  std::vector<std::string> empty_streams;
  EXPECT_NO_THROW(client->subscribe_to_streams(empty_streams));
  EXPECT_NO_THROW(client->unsubscribe_from_streams(empty_streams));
}

TEST_F(BinanceClientTest, SubscriptionWithoutConnection) {
  std::vector<std::string> streams = {"btcusdt@ticker"};

  EXPECT_NO_THROW(client->subscribe_to_streams(streams));
  EXPECT_NO_THROW(client->unsubscribe_from_streams(streams));
}

TEST_F(BinanceClientTest, RapidSubscriptions) {
  std::string url = "wss://stream.binance.com:9443/ws/websocket";
  client->setup_websocket(url);
  client->connect();
  ASSERT_TRUE(wait_for_connection(15));

  // Rapid subscribe/unsubscribe cycles
  for (int i = 0; i < 5; ++i) {
    std::vector<std::string> streams = {"btcusdt@ticker"};
    EXPECT_NO_THROW(client->subscribe_to_streams(streams));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_NO_THROW(client->unsubscribe_from_streams(streams));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

TEST_F(BinanceClientTest, DifferentEndpoints) {
  // Test alternative Binance endpoint
  std::string url1 = "wss://stream.binance.com:443/ws";
  client->setup_websocket(url1);
  client->connect();

  bool connected1 = wait_for_connection(10);
  if (connected1) {
    client->disconnect();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Test echo server for comparison
  std::string url2 = "wss://echo.websocket.org";
  client->setup_websocket(url2);
  client->connect();

  bool connected2 = wait_for_connection(10);
  EXPECT_TRUE(connected2) << "Echo server should be reliable for testing";

  if (connected2) {
    client->disconnect();
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}