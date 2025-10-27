//
// Created by Lukas Varhol on 21/7/2025.
//
#ifndef COIN_H
#define COIN_H

#include <string>
#include "MovingAverage.h"

struct CoinData {
  std::string symbol;
  double price;
  long trade_id;
  double trade_quantity;
  long trade_time;
};

class Coin {
private:
  std::string const symbol_;
  double price_;
  long last_trade_id_;
  double last_trade_quantity_;
  long last_trade_time_;
  MovingAverage average_manager_; //storing object is fine as copy is not performed (references stored in coin manager)

public:
  Coin(const std::string& symbol);
  void update_trade(CoinData& data);
  std::string symbol() const;
  double price() const;
  long last_trade_id() const;
  double last_trade_quantity() const;
  long last_trade_time() const;
  [[nodiscard]] const MovingAverage& moving_average() const;
};

#endif //COIN_H
