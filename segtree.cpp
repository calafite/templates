template<typename T, typename F>
struct SegTree{
	int n;
	vector<T> t;
	T neutral;
	F combine;

	SegTree(int _n, T _neutral, F _combine)
		: n(_n), neutral(_neutral), combine(_combine){
		t.assign(4*n, neutral);
	}

	SegTree(const vector<T> &_a, T _neutral, F _combine)
		: n((int)_a.size()), neutral(_neutral), combine(_combine){
		t.assign(4*n, neutral);
		build(1, 0, n-1, _a);
	}
	void build(int pos, int tl, int tr, const vector<T> &a){
		if (tl==tr){
			this->t[pos] = a[tl];
			return;
		}
		int tm = tl+(tr-tl)/2;
		this->build(2*pos, tl, tm, a);
		this->build(2*pos+1, tm+1, tr, a);
		this->t[pos] = this->combine(this->t[2*pos], this->t[2*pos+1]);
	}

	void update(int i, T val, int pos, int tl, int tr){
		if (tl==tr){
			this->t[pos] = val;
			return;
		}
		int tm = tl+(tr-tl)/2;
		if (i<=tm) this->update(i, val, 2*pos, tl, tm);
		else this->update(i, val, 2*pos+1, tm+1, tr);
		this->t[pos] = this->combine(this->t[2*pos], this->t[2*pos+1]);
	}

	void update(int i, T val){
		this->update(i, val, 1, 0, this->n-1);
	}

	T query(int l, int r, int pos, int tl, int tr){
		if (r<tl||tr<l) return this->neutral;
		if (l<=tl&&tr<=r) return this->t[pos];
		int tm = tl+(tr-tl)/2;
		return this->combine(this->query(l, r, 2*pos, tl, tm), this->query(l, r, 2*pos+1, tm+1, tr));
	}

	T query(int l, int r){
		return this->query(l, r, 1, 0, this->n-1);
	}
};

