#pragma once
#include <cmath>
#include <algorithm>
#include <cassert>
#include "../core/fvec.hpp"

namespace calafite {
template <typename T> struct LinearSieve {
  static_assert(
      std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) >= 4,
      "T must be a signed integer of at least 32 bits (int, long, long long)");

  T n;
  fvec<T> primes;
  fvec<T> spf;

  LinearSieve(T n) : n(n), spf(n + 1, 0) {
    if (n >= 2)
      primes.reserve(
          static_cast<size_t>(n / log(static_cast<double>(n)) * 1.2) + 16);
    for (T i = 2; i <= this->n; i++) {
      if (!spf[i]) {
        spf[i] = i;
        primes.push_back(i);
      }
      for (T p : primes) {
        if (p > this->n / i)
          break;
        spf[i * p] = p;
        if (p == spf[i])
          break;
      }
    }
  }

  bool query(T x) const {
    if (x < 2 || x > n)
      return false;
    return spf[x] == x;
  }

  T count(T x) const {
    x = std::min(x, n);
    return static_cast<T>(std::upper_bound(primes.begin(), primes.end(), x) -
                          primes.begin());
  }

  fvec<std::pair<T, int>> factorize(T x) const {
    assert(x >= 1 && x <= n);
    fvec<std::pair<T, int>> factors;
    while (x > 1) {
      T p = spf[x];
      int e = 0;
      do {
        e++;
        x /= p;
      } while (x > 1 && spf[x] == p);
      factors.emplace_back(p, e);
    }
    return factors;
  }
};
} // namespace calafite
