//
// Created by Lukas Varhol on 21/7/2025.
//

#include "../include/common/Visualizer.h"
#include <iomanip>
#include <iostream>

void display_prices(CoinManager& manager) {
  std::cout  << "\033[2J\033[H" << std::flush;
  std::cout << "\033[?25l";
  std::cout << "\033[1m" << "=== Live Crypto Tracker ===" << "\033[0m" << std::endl;
  std::cout << std::left << std::setw(10) << "Symbol" << std::setw(15) << "Price (USD)" << "Last Update" << std::endl;
  std::cout << std::string(55, '-') << std::endl;

  std::vector<Coin*> p_coins = manager.all_coins();
  for (const auto p_coin: p_coins) {
    std::cout << std::left << std::setw(10) << p_coin->symbol()
    << std::setw(15) << std::fixed << std::setprecision(6) << p_coin->price()
    << p_coin->last_trade_time() << std::endl;
  }
  std::cout << std::flush;
}
