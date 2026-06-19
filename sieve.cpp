#pragma once
#include <bits/stdc++.h>
using namespace std;

template <typename T> struct Sieve {
  static_assert(is_integral_v<T> && is_signed_v<T> && sizeof(T) >= 4,
                "T must be a signed integer of at least 32 bits");

  T n;
  vector<bool> is_prime;
  vector<T> primes;

  Sieve(T n) : n(n), is_prime(n + 1, true) {
    is_prime[0] = is_prime[1] = false;
    for (T i = 4; i <= n; i += 2)
      is_prime[i] = false;
    for (T p = 3; p <= n / p; p += 2) { 
      if (is_prime[p])
        for (T i = p * p; i <= n; i += p + p)
          is_prime[i] = false;
    }
    if (n >= 2)
      primes.push_back(2);
    for (T i = 3; i <= n; i += 2)
      if (is_prime[i])
        primes.push_back(i);
  }

  bool query(T x) const {
    if (x < 0 || x > n)
      return false;
    return is_prime[x];
  }

  T count(T x) const {
    x = min(x, n);
    return (T)(upper_bound(primes.begin(), primes.end(), x) - primes.begin());
  }
};
