//
// Created by Lukas Varhol on 21/7/2025.
//

#ifndef COINMANAGER_H
#define COINMANAGER_H
#include <memory>
#include <unordered_map>
#include <vector>
#include "Coin.h"

class BinanceClient;

class CoinManager {
private:
  std::unordered_map<std::string, std::unique_ptr<Coin>> coins_;
  BinanceClient* binance_client_;

public:
  CoinManager();
  void set_binance_client(BinanceClient* client);
  void add_coins(const std::vector<std::string>& symbols);
  void remove_coins(const std::vector<std::string>& symbols);
  void update_coin_data(CoinData &data);
  std::vector<std::string> all_coins() const;
  bool has_coin(const std::string& symbol) const;
};

#endif //COINMANAGER_H
