//
// Created by Lukas Varhol on 21/7/2025.
//

#include "../include/common/Coin.h"


Coin::Coin(const std::string &symbol) :
    symbol_(symbol), price_(0), last_trade_id_(0), last_trade_quantity_(0), last_trade_time_(0) {};

void Coin::update_trade(CoinData& data) {
  price_ = data.price;
  last_trade_id_ = data.trade_id;
  last_trade_quantity_ = data.trade_quantity;
  last_trade_time_ = data.trade_time;
}

std::string Coin::symbol() const {
  return symbol_;
}

double Coin::price() const {
  return price_;
}

long Coin::last_trade_id() const {
  return last_trade_id_;
};
double Coin::last_trade_quantity() const {
  return last_trade_quantity_;
}
long Coin::last_trade_time() const {
  return last_trade_time_;
}