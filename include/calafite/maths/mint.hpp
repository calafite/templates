#pragma once

#include "../io/io.hpp"
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace calafite {
    namespace mathematics {

        template<uint32_t Mod> struct Modular {
            static_assert(Mod > 0, "Modulus must be strictly positive");
            
            uint32_t value;

            constexpr Modular() : value(0) {}

            template<typename Type, typename = std::enable_if_t<std::is_integral_v<Type>>>
            constexpr Modular(Type initialValue) {
                long long remainder = static_cast<long long>(initialValue) % Mod;
                if (remainder < 0) remainder += Mod;
                value = static_cast<uint32_t>(remainder);
            }

            constexpr Modular operator+() const { return *this; }
            constexpr Modular operator-() const { return Modular() - *this; }

            constexpr Modular& operator+=(const Modular& other) {
                value += other.value;
                if (value >= Mod) value -= Mod;
                return *this;
            }

            constexpr Modular& operator-=(const Modular& other) {
                if (value < other.value) value += Mod;
                value -= other.value;
                return *this;
            }

            constexpr Modular& operator*=(const Modular& other) {
                value = static_cast<uint32_t>((static_cast<uint64_t>(value) * other.value) % Mod);
                return *this;
            }

            constexpr Modular& operator/=(const Modular& other) {
                return *this *= other.inverse();
            }

            constexpr Modular& operator++() {
                value = (value + 1 == Mod) ? 0 : value + 1;
                return *this;
            }

            constexpr Modular operator++(int) {
                Modular copy = *this;
                ++(*this);
                return copy;
            }

            constexpr Modular& operator--() {
                value = (value == 0) ? Mod - 1 : value - 1;
                return *this;
            }

            constexpr Modular operator--(int) {
                Modular copy = *this;
                --(*this);
                return copy;
            }

            constexpr Modular inverse() const {
                long long a = value, b = Mod, u = 1, v = 0;
                while (b != 0) {
                    long long t = a / b;
                    a -= t * b; std::swap(a, b);
                    u -= t * v; std::swap(u, v);
                }
                assert(a == 1 && "Modular inverse does not exist (not coprime).");
                return Modular(u);
            }

            constexpr Modular power(long long exponent) const {
                Modular result(1);
                Modular base(*this);
                if (exponent < 0) {
                    base = base.inverse();
                    exponent = -exponent;
                }
                while (exponent > 0) {
                    if (exponent & 1) result *= base;
                    base *= base;
                    exponent >>= 1;
                }
                return result;
            }

            friend constexpr Modular operator+(Modular left, const Modular& right) { return left += right; }
            friend constexpr Modular operator-(Modular left, const Modular& right) { return left -= right; }
            friend constexpr Modular operator*(Modular left, const Modular& right) { return left *= right; }
            friend constexpr Modular operator/(Modular left, const Modular& right) { return left /= right; }
            friend constexpr bool operator==(const Modular& left, const Modular& right) { return left.value == right.value; }
            friend constexpr bool operator!=(const Modular& left, const Modular& right) { return left.value != right.value; }
        };

        template<uint32_t Mod>
        inline io::Printer& operator<<(io::Printer& printer, const Modular<Mod>& number) {
            return printer << number.value;
        }

        template<uint32_t Mod>
        inline io::Scanner& operator>>(io::Scanner& scanner, Modular<Mod>& number) {
            long long temporary;
            scanner >> temporary;
            number = Modular<Mod>(temporary);
            return scanner;
        }
    }
}
