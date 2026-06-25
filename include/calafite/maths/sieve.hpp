#pragma once
#include "../core/fvec.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace calafite {
template <typename T = int> struct Sieve {
  static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) >= 4,
                "T must be a signed integer of at least 32 bits");

  int n;
  fvec<bool> is_prime;
  fvec<int> primes;

  Sieve(int n) : n(n), is_prime((n >> 1) + 1, true) {
    if (is_prime.size() > 0)
      is_prime[0] = false;

    for (int p = 3; 1LL * p * p <= n; p += 2) {
      if (is_prime[p >> 1]) {
        for (long long i = 1LL * p * p; i <= n; i += p * 2) {
          is_prime[i >> 1] = false;
        }
      }
    }

    if (n >= 2) {
      primes.reserve(n / log(n) * 1.2 + 16);
      primes.push_back(2);
      for (int i = 3; i <= n; i += 2)
        if (is_prime[i >> 1])
          primes.push_back(i);
    }
  }

  bool query(T x) const {
    if (x < 2 || x > n)
      return false;
    if (x == 2)
      return true;
    if ((x & 1) == 0)
      return false;
    return is_prime[x >> 1];
  }

  T count(T x) const {
    x = std::min(static_cast<T>(n), x);
    if (x < 2)
      return 0;
    return static_cast<T>(
        std::upper_bound(primes.begin(), primes.end(), static_cast<int>(x)) -
        primes.begin());
  }
};
} // namespace calafite
