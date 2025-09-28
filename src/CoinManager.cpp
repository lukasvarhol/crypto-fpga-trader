//
// Created by Lukas Varhol on 21/7/2025.
//

#include "../include/common/CoinManager.h"
#include <iostream>
#include <vector>
#include "../include/client/BinanceClient.h"

CoinManager::CoinManager() : binance_client_(nullptr) {}

void CoinManager::set_binance_client(BinanceClient *client) {
  binance_client_ = client;
}


void CoinManager::add_coins(const std::vector<std::string> &symbols) {
  if (symbols.empty()) {
    std::cout << "Warning: Empty symbols provided" << std::endl;
    return;
  }

  std::vector<std::string> new_symbols;

  for (const std::string& symbol : symbols) {
    if (symbol.empty()) {
      std::cout << "Warning: Skipping empty symbol" << std::endl;
      continue;
    }
    if (!has_coin(symbol)) {
      coins_[symbol] = std::make_unique<Coin>(symbol);
      new_symbols.push_back(symbol);
    }
  }

  if (new_symbols.empty()) {
    std::cout << "empty!!!" << std::endl;
  }


  if (!new_symbols.empty() && binance_client_ && binance_client_->is_connected()) {
    std::cout << "preparing to subscribe";
    std::vector<std::string> stream_names;
    for (const std::string& symbol : new_symbols) {
      stream_names.push_back(symbol + "@trade");
      std::cout << stream_names.back() << std::endl;
    }
    binance_client_->subscribe_to_streams(stream_names);
  } else if (!new_symbols.empty() && !binance_client_) {
    std::cout << "Warning: No BinanceClient available for subscription" << std::endl;
  } else if (!new_symbols.empty() && !binance_client_->is_connected()) {
    std::cout << "Warning: BinanceClient not connected, cannot subscribe" << std::endl;
  }
}

void CoinManager::remove_coins(const std::vector<std::string> &symbols) {
  std::vector<std::string> symbols_to_remove;

  for (const auto& symbol : symbols) {
    if (coins_.find(symbol) != coins_.end()) {
      symbols_to_remove.push_back(symbol);
      coins_.erase(symbol);
      std::cout << "Removed coin: " << symbol << std::endl;
    }
  }

  // Unsubscribe from Binance streams
  if (!symbols_to_remove.empty() && binance_client_ && binance_client_->is_connected()) {
    std::vector<std::string> stream_names;
    for (const std::string& symbol : symbols_to_remove) {
      stream_names.push_back(symbol + "@trade");
      std::cout << stream_names.back() << std::endl;
    }
    binance_client_->unsubscribe_from_streams(stream_names);
  }
}

void CoinManager::update_coin_data(CoinData &data) {
  auto it = coins_.find(data.symbol);
  if (it != coins_.end()) {
    it->second->update_trade(data);
    return;
  }
  std::cout << "Received data for unknown coin: " << data.symbol << std::endl;
}

std::vector<std::string> CoinManager::all_coin_symbols() const{
  std::vector<std::string> symbols;
  symbols.reserve(coins_.size());

  for (const auto& pair : coins_) {
    symbols.push_back(pair.first);
  }
  return symbols;
}


std::vector<Coin*> CoinManager::all_coins() const {
  std::vector<Coin*> coins;
  coins.reserve(coins_.size());

  for (const auto& pair : coins_) {
    coins.push_back(pair.second.get());
  }
  return coins;
}


bool CoinManager::has_coin(const std::string &symbol) const {
  if (coins_.find(symbol) == coins_.end()) {
    return false;
  }
  return true;
}


