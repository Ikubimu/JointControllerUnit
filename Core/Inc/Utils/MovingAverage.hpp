#ifndef UTILS_MOVING_AVERAGE_HPP
#define UTILS_MOVING_AVERAGE_HPP

#include <cstddef>

template<typename T, size_t SIZE>
class MovingAverage {
public:
    MovingAverage() : index(0), count(0), sum(0), last(T(0)) {
        for (size_t i = 0; i < SIZE; ++i)
            buffer[i] = T(0);
    }

    void add(T value) {
        sum -= buffer[index];
        buffer[index] = value;
        sum += value;
        last = value;
        index = (index + 1) % SIZE;
        if (count < SIZE) ++count;
    }

    T get() const {
        if (count == 0) return T(0);
        return sum / static_cast<T>(count);
    }

    T lastValue() const {
        return last;
    }

    void reset() {
        index = 0;
        count = 0;
        sum = T(0);
        last = T(0);
        for (size_t i = 0; i < SIZE; ++i)
            buffer[i] = T(0);
    }

private:
    T buffer[SIZE];
    size_t index;
    size_t count;
    T sum;
    T last;
};

#endif
