#pragma once

#include <numeric>

namespace calafite {
namespace ops {

#define CALAFITE_MAKE_OP(name, op)                                             \
  struct _##name {                                                             \
    template <typename T, typename U>                                          \
    constexpr auto operator()(const T &a, const U &b) const {                  \
      return a op b;                                                           \
    }                                                                          \
    template <typename U> constexpr auto operator()(const U &b) const {        \
      return [b](const auto &a) { return a op b; };                            \
    }                                                                          \
  };                                                                           \
  inline constexpr _##name name{};

CALAFITE_MAKE_OP(add, +)
CALAFITE_MAKE_OP(sub, -)
CALAFITE_MAKE_OP(mul, *)
CALAFITE_MAKE_OP(div, /)
CALAFITE_MAKE_OP(mod, %)

CALAFITE_MAKE_OP(eq, ==)
CALAFITE_MAKE_OP(neq, !=)
CALAFITE_MAKE_OP(lt, <)
CALAFITE_MAKE_OP(gt, >)
CALAFITE_MAKE_OP(leq, <=)
CALAFITE_MAKE_OP(geq, >=)

CALAFITE_MAKE_OP(band, &)
CALAFITE_MAKE_OP(bor, |)
CALAFITE_MAKE_OP(bxor, ^)

#undef CALAFITE_MAKE_OP

inline constexpr auto min = [](const auto &a, const auto &b) {
  return a < b ? a : b;
};
inline constexpr auto max = [](const auto &a, const auto &b) {
  return a > b ? a : b;
};

struct _gcd {
  template <typename T, typename U>
  constexpr auto operator()(const T &a, const U &b) const {
    return std::gcd(a, b);
  }
  template <typename U> constexpr auto operator()(const U &b) const {
    return [b](const auto &a) { return std::gcd(a, b); };
  }
};
inline constexpr _gcd gcd{};

struct _lcm {
  template <typename T, typename U>
  constexpr auto operator()(const T &a, const U &b) const {
    return std::lcm(a, b);
  }
  template <typename U> constexpr auto operator()(const U &b) const {
    return [b](const auto &a) { return std::lcm(a, b); };
  }
};
inline constexpr _lcm lcm{};

inline constexpr auto is_even = [](const auto &a) { return a % 2 == 0; };
inline constexpr auto is_odd = [](const auto &a) { return a % 2 != 0; };
inline constexpr auto is_pos = [](const auto &a) { return a > 0; };
inline constexpr auto is_neg = [](const auto &a) { return a < 0; };
inline constexpr auto is_zero = [](const auto &a) { return a == 0; };

} // namespace ops
} // namespace calafite
