//
// Created by Lukas Varhol on 20/10/2025.
//

#ifndef MOVINGAVERAGE_H
#define MOVINGAVERAGE_H
#include <cstddef>
#include <deque>

class MovingAverage{
  private:
    std::size_t window_size_;
    std::deque<double> window_;
    double sum_;
  public:
    explicit MovingAverage(std::size_t window_size);
    void update(double new_price);
    double get_value() const;
    bool is_ready() const;
    bool is_price_below_MA(double current_price) const;
    bool is_price_above_MA(double current_price) const;
};

#endif //MOVINGAVERAGE_H
