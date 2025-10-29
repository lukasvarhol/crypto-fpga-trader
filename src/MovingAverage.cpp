//
// Created by Lukas Varhol on 20/10/2025.
//

#include "../include/common/MovingAverage.h"

MovingAverage::MovingAverage(std::size_t window_size)
  : window_size_(window_size), sum_(0.0){}

void MovingAverage::update(double new_price){
  window_.push_back(new_price);
  sum_ += new_price;

  if (window_.size() > window_size_) {
    sum_ -= window_.front();
    window_.pop_front();
  }
}

double MovingAverage::get_value() const{
  if (window_.empty()) return 0.0;
  return sum_ / window_.size();
}

bool MovingAverage::is_ready() const {
  return window_.size() >= window_size_;
}

bool MovingAverage::is_price_below_MA(double current_price) const{
  return current_price < get_value();
}

bool MovingAverage::is_price_above_MA(double current_price) const{
  return current_price > get_value();
}
