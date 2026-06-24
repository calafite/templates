#pragma once

#include <cmath>
#include <numeric>

namespace calafite {
namespace ops {

namespace eps {
inline constexpr double default_eps = 1e-9;

#define CALAFITE_MAKE_EPS_OP(name, expr)                                       \
  struct _##name {                                                             \
    double e;                                                                  \
    constexpr _##name(double e = default_eps) : e(e) {}                        \
    template <typename T, typename U>                                          \
    constexpr bool operator()(const T &a, const U &b) const {                  \
      return expr;                                                             \
    }                                                                          \
    template <typename U> constexpr auto operator()(const U &b) const {        \
      return [b, e = e](const auto &a) { return _##name{e}(a, b); };           \
    }                                                                          \
    constexpr _##name with_eps(double new_e) const { return _##name{new_e}; }  \
  };                                                                           \
  inline constexpr _##name name{default_eps};

CALAFITE_MAKE_EPS_OP(eq, (std::abs(a - b) < e))
CALAFITE_MAKE_EPS_OP(neq, (std::abs(a - b) >= e))
CALAFITE_MAKE_EPS_OP(lt, (a < b - e))
CALAFITE_MAKE_EPS_OP(gt, (a > b + e))
CALAFITE_MAKE_EPS_OP(leq, (a < b + e))
CALAFITE_MAKE_EPS_OP(geq, (a > b - e))

#undef CALAFITE_MAKE_EPS_OP

#define CALAFITE_MAKE_EPS_UNARY(name, expr)                                    \
  struct _##name {                                                             \
    double e;                                                                  \
    constexpr _##name(double e = default_eps) : e(e) {}                        \
    constexpr bool operator()(double a) const { return expr; }                 \
    constexpr _##name with_eps(double new_e) const { return _##name{new_e}; }  \
  };                                                                           \
  inline constexpr _##name name{default_eps};

CALAFITE_MAKE_EPS_UNARY(is_zero, (std::abs(a) < e))
CALAFITE_MAKE_EPS_UNARY(is_pos, (a > e))
CALAFITE_MAKE_EPS_UNARY(is_neg, (a < -e))

#undef CALAFITE_MAKE_EPS_UNARY
} // namespace eps

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
