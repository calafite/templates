#pragma once
#include "../core/fvec.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace calafite {
template <typename T> struct LinearSieve {
  static_assert(
      std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) >= 4,
      "T must be a signed integer of at least 32 bits (int, long, long long)");

  T n;
  fvec<T> primes;
  fvec<T> spf;

  LinearSieve(T n) : n(n), spf((n >> 1) + 1, 0) {
    if (n >= 2) {
      primes.reserve(
          static_cast<size_t>(n / log(static_cast<double>(n)) * 1.2) + 16);
      primes.push_back(2);
    }
    for (T i = 3; i <= this->n; i += 2) {
      T half_i = i >> 1;
      if (!spf[half_i]) {
        spf[half_i] = i;
        primes.push_back(i);
      }

      for (size_t j = 1; j < primes.size(); ++j) {
        T p = primes[j];
        if (1LL * p * i > this->n)
          break;
        spf[(i * p) >> 1] = p;
        if (p == spf[half_i])
          break;
      }
    }
  }

  bool query(T x) const {
    if (x < 2 || x > n)
      return false;
    if (x == 2)
      return true;
    if ((x & 1) == 0)
      return false;
    return spf[x >> 1] == x;
  }

  T count(T x) const {
    x = std::min(x, n);
    if (x < 2)
      return 0;
    return static_cast<T>(std::upper_bound(primes.begin(), primes.end(), x) -
                          primes.begin());
  }

  fvec<std::pair<T, int>> factorize(T x) const {
    assert(x >= 1 && x <= n);
    fvec<std::pair<T, int>> factors;

    if ((x & 1) == 0) {
      int e = 0;
      do {
        e++;
        x >>= 1;
      } while ((x & 1) == 0);
      factors.emplace_back(2, e);
    }

    while (x > 1) {
      T p = spf[x >> 1];
      int e = 0;
      do {
        e++;
        x /= p;
      } while (x > 1 && spf[x >> 1] == p);
      factors.emplace_back(p, e);
    }
    return factors;
  }
};
} // namespace calafite
