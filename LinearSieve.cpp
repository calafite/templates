template<typename T>
struct LinearSieve{
    T n;
    vector<T> primes;
    vector<T> spf;

    LinearSieve(T n) : n(n), spf(n+1, 0){
        for (T i = 2; i <= this->n; i++){
            if (!this->spf[i]){
                this->spf[i] = i;
                this->primes.pb(i);
            }
            for (T p : this->primes){
                if (i*p > this->n) break;
                this->spf[i*p] = p;
                if (p == this->spf[i]) break;
            }
        }
    }

    bool query(T x){
        if (x < 2 || x > this->n) return false;
        return this->spf[x] == x;
    }

    T count(T x){
        x = min(x, this->n);
        return (T)(upper_bound(all(this->primes), x) - this->primes.begin());
    }

    vector<pair<T, int>> factorize(T x){
        vector<pair<T, int>> factors;
        while (x > 1){
            T p = this->spf[x];
            int count = 0;
            do {
                count++;
                x /= p;
            } while (x > 1 && this->spf[x] == p);
            factors.pb({p, count});
        }
        return factors;
    }
};
