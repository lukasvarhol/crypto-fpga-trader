//
// Created by Lukas Varhol on 23/7/2025.
//
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "../include/common/CoinManager.h"

class CoinManagerTest : public ::testing::Test {
protected:
  void SetUp() override { manager = std::make_unique<CoinManager>(); }

  std::unique_ptr<CoinManager> manager;
};

TEST_F(CoinManagerTest, HasCoinNoCoinsAdded) { EXPECT_FALSE(manager->has_coin("test")); }

TEST_F(CoinManagerTest, AllCoinsNoCoinsAdded) { EXPECT_TRUE(manager->all_coins().empty()); }

TEST_F(CoinManagerTest, AllCoinSymbolsNoCoinsAdded) { EXPECT_TRUE(manager->all_coin_symbols().empty()); }

TEST_F(CoinManagerTest, UpdateCoinNoCoinsAdded) {
  CoinData data;
  data.symbol = "test";
  data.price = 100;
  data.trade_id = 1;
  data.trade_quantity = 100;
  data.trade_time = 100;

  EXPECT_NO_THROW(manager->update_coin_data(data));
  EXPECT_FALSE(manager->has_coin("test"));
  EXPECT_TRUE(manager->all_coins().empty());
}

TEST_F(CoinManagerTest, RemoveCoinNoCoinsAdded) {
  std::vector<std::string> coins = {"test"};
  EXPECT_NO_THROW(manager->remove_coins(coins));
  EXPECT_TRUE(manager->all_coins().empty());
}

TEST_F(CoinManagerTest, AddOneCoin) {
  std::vector<std::string> coins = {"test"};
  EXPECT_NO_THROW(manager->add_coins(coins));
  EXPECT_TRUE(manager->has_coin("test"));
}

TEST_F(CoinManagerTest, AddMultipleCoins) {
  std::vector<std::string> coins = {"test1", "test2", "test3", "test4", "test5", "test6", "test7", "test8", "test9"};
  EXPECT_NO_THROW(manager->add_coins(coins));
  EXPECT_TRUE(manager->all_coins().size() == 9);
}
