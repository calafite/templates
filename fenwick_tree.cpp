struct FenwickTree {
    vll bits;
    ll sz = 0;
    ll log2sz = 1;

    FenwickTree(ll n) {
        this->sz = n;
        this->bits.assign(this->sz + 1, 0);
        while (this->log2sz < this->sz) this->log2sz <<= 1;
    }

    FenwickTree(vll const& a) {
        this->sz = (ll) a.size();
        this->bits.assign(this->sz + 1, 0);
        for (ll i = 1; i <= this->sz; i++) {
            this->bits[i] += a[i - 1];
            ll j = i + (i & -i);
            if (j <= this->sz) this->bits[j] += this->bits[i];
        }
        while (this->log2sz < this->sz) this->log2sz <<= 1;
    }

    ll query(ll i) {
        i++;
        ll ans = 0;
        while (i > 0) {
            ans += this->bits[i];
            i -= (i & -i);
        }
        return ans;
    }

    ll query(ll l, ll r) {
        return this->query(r) - this->query(l - 1);
    }

    void update(ll i, ll add) {
        i++;
        while (i <= this->sz) {
            this->bits[i] += add;
            i += (i & -i);
        }
    }

    void set(ll i, ll val) {
        this->update(i, val - this->query(i, i));
    }

    ll kth(ll k) {
        ll pos = 0;
        for (ll pw = this->log2sz; pw > 0; pw >>= 1) {
            if (pos + pw <= this->sz && this->bits[pos + pw] < k) {
                pos += pw;
                k -= this->bits[pos];
            }
        }
        return pos;
    }
};
