#pragma once

#include "fvec.hpp"
#include <algorithm>

namespace calafite {

template <typename T> struct Compressor {
  fvec<T> vals;

  Compressor() = default;

  explicit Compressor(const fvec<T> &initial_vals) : vals(initial_vals) {
    build();
  }

  inline void add(const T &x) { vals.push_back(x); }

  inline void build() {
    vals.sort();   
    vals.unique();
  }

  inline int get(const T &x) const {
    return static_cast<int>(std::lower_bound(vals.begin(), vals.end(), x) -
                            vals.begin());
  }

  inline T operator[](int i) const { return vals[i]; }

  inline int size() const { return static_cast<int>(vals.size()); }

  fvec<int> compress_array(const fvec<T> &arr) const {
    fvec<int> res(arr.size());
    for (size_t i = 0; i < arr.size(); i++) {
      res[i] = get(arr[i]);
    }
    return res;
  }
};

} // namespace calafite
