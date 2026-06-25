#pragma once
#include <utility>

namespace calafite {

template <typename Fun> 
struct FixPoint {
  Fun f_;

  template <typename G> 
  explicit FixPoint(G&& g) : f_(std::forward<G>(g)) {}

  template <typename... Args> 
  decltype(auto) operator()(Args&&... args) {
    return f_(*this, std::forward<Args>(args)...);
  }
};

template <typename Fun>
FixPoint(Fun) -> FixPoint<Fun>;

} // namespace calafite
