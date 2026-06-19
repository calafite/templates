template<typename T>
struct Sieve{
    T n;
    vector<bool> is_prime;
    vector<T> primes;

    Sieve(T n) : n(n), is_prime(n+1, true){
        this->is_prime[0] = this->is_prime[1] = false;
        for (T i = 4; i <= this->n; i += 2) this->is_prime[i] = false;
        for (T p = 3; p*p <= this->n; p += 2){
            if (this->is_prime[p]){
                for (T i = p*p; i <= this->n; i += 2*p) this->is_prime[i] = false;
            }
        }
        if (this->n >= 2) this->primes.pb(2);
        for (T i = 3; i <= this->n; i += 2){
            if (this->is_prime[i]) this->primes.pb(i);
        }
    }

    bool query(T x){
        if (x < 0 || x > this->n) return false;
        return this->is_prime[x];
    }

    T count(T x){
        x = min(x, this->n);
        return (T)(upper_bound(all(this->primes), x) - this->primes.begin());
    }
};
