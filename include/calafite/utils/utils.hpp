#include "../core/fvec.hpp"
#include <algorithm>

namespace calafite {
template <typename T> inline int lb(const fvec<T> &a, const T &x) {
  return static_cast<int>(std::lower_bound(a.begin(), a.end(), x) - a.begin());
}

template <typename T> inline int ub(const fvec<T> &a, const T &x) {
  return static_cast<int>(std::upper_bound(a.begin(), a.end(), x) - a.begin());
}
} // namespace calafite
