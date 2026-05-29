template<typename T, typename F>
struct SparseTable{
	vector<vector<T> > table;
	int length, max_log;
  F operation;

	SparseTable(const vector<T> &vec, const F &operation){
		this->length = vec.size();
		this->max_log = __lg(this->length)+1;
		this->operation = operation;
		this->table.assign(this->max_log+1, vector<T>(this->length));
		for (int j = 0; j<this->length; j++) this->table[0][j] = vec[j];
		for (int i = 1; i<=max_log; i++){
			for (int j = 0; j+(1<<i)<=this->length; j++){
				this->table[i][j] = this->operation(
					this->table[i-1][j],
					this->table[i-1][j+(1<<(i-1))]
				);
			}
		}
	}
  
  // works only for idempotent operations
	T query(int left, int right){
		int i = __lg(right-left+1);
		return this->operation(
			this->table[i][left],
			this->table[i][right-(1<<i)+1]
		);
	}
};
