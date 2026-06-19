
template<typename T>
struct Sieve {
    T n;
    vector<bool> is_prime;
    vector<T> primes;
    vector<T> spf;

    Sieve(T n) : n(n), is_prime(n + 1, true){
        is_prime[0] = is_prime[1] = false;
        for (T p = 2; p * p <= n; p++){
            if (is_prime[p]){
                for (T i = p * p; i <= n; i += p) is_prime[i] = false;
            }
        }
        for (T i = 2; i <= n; i++){
            if (is_prime[i]) primes.pb(i);
        }
    }

    bool query(T x){
        if (x < 0 || x > n) return false;
        return is_prime[x];
    }

    T count(T x){
        x = min(x, n);
        return (T)upper_bound(all(primes), x) - primes.begin();
    }

    void build_spf(){
        spf.assign(n + 1, 0);
        for (T i = 2; i <= n; i++){
            if (!spf[i]){
                for (T j = i; j <= n; j += i){
                    if (!spf[j]) spf[j] = i;
                }
            }
        }
    }

    map<T, int> factorize(T x){
        map<T, int> factors;
        while (x > 1){
            factors[spf[x]]++;
            x /= spf[x];
        }
        return factors;
    }
};
