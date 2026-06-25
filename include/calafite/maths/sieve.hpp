#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include "../core/fvec.hpp"

namespace calafite {
template <typename T = int> struct Sieve {
  static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) >= 4,
                "T must be a signed integer of at least 32 bits");

  int n;
  fvec<bool> is_prime;
  fvec<int> primes;

  Sieve(int n) : n(n), is_prime(n + 1, true) {
    if (n >= 0)
      is_prime[0] = false;
    if (n >= 1)
      is_prime[1] = false;

    for (int i = 4; i <= n; i += 2)
      is_prime[i] = false;

    for (int p = 3; 1LL * p * p <= n; p += 2) {
      if (is_prime[p])
        for (int i = p * p; i <= n; i += p * 2)
          is_prime[i] = false;
    }

    if (n >= 2) {
      primes.reserve(n / log(n) * 1.2 + 16);
      primes.push_back(2);
      for (int i = 3; i <= n; i += 2)
        if (is_prime[i])
          primes.push_back(i);
    }
  }

  bool query(T x) const {
    if (x < 0 || x > n)
      return false;
    return is_prime[x];
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
