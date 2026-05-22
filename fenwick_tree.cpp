struct FenwickTree {
    vector<long long> bits;
    long long sz = 0;
    long long log2sz = 1;

    FenwickTree(long long n) {
        this->sz = n;
        this->bits.assign(this->sz + 1, 0);
        while (this->log2sz < this->sz) this->log2sz <<= 1;
    }

    FenwickTree(const vector<long long>& a) {
        this->sz = (long long) a.size();
        this->bits.assign(this->sz + 1, 0);
        for (long long i = 1; i <= this->sz; i++) {
            this->bits[i] += a[i - 1];
            long long j = i + (i & -i);
            if (j <= this->sz) this->bits[j] += this->bits[i];
        }
        while (this->log2sz < this->sz) this->log2sz <<= 1;
    }

    long long query(long long i) const {
        i++;
        long long ans = 0;
        while (i > 0) {
            ans += this->bits[i];
            i -= (i & -i);
        }
        return ans;
    }

    long long query(long long l, long long r) const {
        return this->query(r) - this->query(l - 1);
    }

    void update(long long i, long long add) {
        i++;
        while (i <= this->sz) {
            this->bits[i] += add;
            i += (i & -i);
        }
    }

    void set(long long i, long long val) {
        this->update(i, val - this->query(i, i));
    }

    long long kth(long long k) const {
        long long pos = 0;
        for (long long pw = this->log2sz; pw > 0; pw >>= 1) {
            if (pos + pw <= this->sz && this->bits[pos + pw] < k) {
                pos += pw;
                k -= this->bits[pos];
            }
        }
        return pos;
    }
};
