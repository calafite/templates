template<typename T>
struct Sieve{
    T n;
    vector<bool> is_prime;
    vector<T> primes;
    vector<T> spf;

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

    void build_spf(){
        this->spf.assign(this->n+1, 0);
        for (T i = 2; i <= this->n; i += 2) this->spf[i] = 2;
        for (T p : this->primes){
            if (p == 2) continue;
            this->spf[p] = p; 
            if (p*p > this->n) continue;
            for (T i = p*p; i <= this->n; i += 2*p){
                if (!this->spf[i]) this->spf[i] = p;
            }
        }
    }

    map<T, int> factorize(T x){
        map<T, int> factors;
        auto it = factors.end();
        while (x > 1){
            T p = this->spf[x];
            int count = 0;
            do {
                count++;
                x /= p;
            } while (x > 1 && this->spf[x] == p);
            it = factors.emplace_hint(it, p, count);
        }
        return factors;
    }
};
