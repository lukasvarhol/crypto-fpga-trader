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
  std::cout << std::left
            << std::setw(10) << "Symbol"
            << std::setw(15) << "Price (USD)"
            << std::setw(15) << "Average"
            << std::setw(30) << "Last Update"
            << std::endl;
  std::cout << std::string(55, '-') << std::endl;

  std::vector<Coin*> p_coins = manager.all_coins();

  for (const auto p_coin: p_coins) {
    long long timestamp_ms = p_coin->last_trade_time();
    std::time_t t = static_cast<std::time_t>(timestamp_ms / 1000);
    int ms_part = static_cast<int>(timestamp_ms % 1000);

    std::tm time{};
#ifdef _WIN32
    localtime_s(&time, &t);
#else
    localtime_r(&t, &time);
#endif

    std::ostringstream time_str;
    time_str << std::put_time(&time, "%a %b %d %H:%M:%S")
             << '.' << std::setfill('0') << std::setw(3) << ms_part
             << ' ' << std::put_time(&time, "%Y");


    std::cout << std::left
              << std::setw(10) << p_coin->symbol()
              << std::setw(15) << std::fixed << std::setprecision(6) << p_coin->price()
              << std::setw(15) << std::fixed << std::setprecision(6) << p_coin->moving_average().get_value()
              << std::setw(35) << time_str.str()
              << std::endl;

  }
  std::cout << std::flush;
}
