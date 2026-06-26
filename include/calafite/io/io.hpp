#pragma once

#include "../core/fastVector.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <io.h>
#define CALAFITE_READ(buffer, size) _read(0, buffer, static_cast<unsigned int>(size))
#define CALAFITE_WRITE(buffer, size) _write(1, buffer, static_cast<unsigned int>(size))
#else
#include <unistd.h>
#define CALAFITE_READ(buffer, size) read(0, buffer, size)
#define CALAFITE_WRITE(buffer, size) write(1, buffer, size)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define CALAFITE_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define CALAFITE_UNLIKELY(x) (x)
#endif

namespace calafite {
    namespace io {

        struct Printer {
            static constexpr size_t BUFFER_SIZE = 1 << 17;
            char buffer[BUFFER_SIZE];
            char* pointer = buffer;
            char numberLookupTable[200];

            Printer() {
                for (size_t index = 0; index < 100; ++index) {
                    numberLookupTable[index * 2] = static_cast<char>('0' + index / 10);
                    numberLookupTable[index * 2 + 1] = static_cast<char>('0' + index % 10);
                }
            }

            ~Printer() { flush(); }

            inline void flush() {
                if (pointer != buffer) {
                    CALAFITE_WRITE(buffer, pointer - buffer);
                    pointer = buffer;
                }
            }

            inline void putChar(char character) {
                if (CALAFITE_UNLIKELY(pointer + 1 >= buffer + BUFFER_SIZE)) flush();
                *pointer++ = character;
            }

            template<typename Type>
            inline std::enable_if_t<std::is_integral_v<Type>, Printer&> operator<<(Type value) {
                if (CALAFITE_UNLIKELY(pointer + 32 >= buffer + BUFFER_SIZE)) flush();

                using UnsignedType = std::make_unsigned_t<Type>;
                UnsignedType unsignedValue = value;
                if constexpr (std::is_signed_v<Type>) {
                    if (value < 0) {
                        *pointer++ = '-';
                        unsignedValue = 0 - unsignedValue;
                    }
                }

                if (unsignedValue == 0) {
                    *pointer++ = '0';
                    return *this;
                }

                char temporary[24];
                size_t index = 24;

                while (unsignedValue >= 100) {
                    index -= 2;
                    UnsignedType remainder = unsignedValue % 100;
                    unsignedValue /= 100;
                    temporary[index] = numberLookupTable[remainder * 2];
                    temporary[index + 1] = numberLookupTable[remainder * 2 + 1];
                }
                
                if (unsignedValue < 10) {
                    temporary[--index] = static_cast<char>('0' + unsignedValue);
                } else {
                    index -= 2;
                    temporary[index] = numberLookupTable[unsignedValue * 2];
                    temporary[index + 1] = numberLookupTable[unsignedValue * 2 + 1];
                }

                size_t length = 24 - index;
                std::memcpy(pointer, temporary + index, length);
                pointer += length;
                return *this;
            }

            inline void writeSeq(int start, int step, int count) {
                for (int i = 0, v = start; i < count; ++i, v += step) {
                    if (CALAFITE_UNLIKELY(pointer + 11 >= buffer + BUFFER_SIZE)) flush();
    
                    unsigned u = static_cast<unsigned>(v);
                    char tmp[10]; int len = 0;
                    
                    while (u >= 100) {
                        unsigned r = u % 100; u /= 100;
                        tmp[len++] = numberLookupTable[r * 2 + 1];
                        tmp[len++] = numberLookupTable[r * 2];
                    }

                    if (u >= 10) {
                        tmp[len++] = numberLookupTable[u * 2 + 1];
                        tmp[len++] = numberLookupTable[u * 2];
                    } else {
                        tmp[len++] = static_cast<char>('0' + u);
                    }

                    for (int j = len - 1; j >= 0; --j) *pointer++ = tmp[j];
                    *pointer++ = ' ';
                }
            }

            inline Printer& operator<<(char character) {
                putChar(character);
                return *this;
            }

            inline Printer& operator<<(const char* string) {
                assert(string != nullptr);
                while (*string) putChar(*string++);
                return *this;
            }

            inline Printer& operator<<(const std::string& string) {
                for (char character : string) putChar(character);
                return *this;
            }

            template<typename Type1, typename Type2>
            inline Printer& operator<<(const std::pair<Type1, Type2>& pair) {
                return *this << pair.first << ' ' << pair.second;
            }

            template<typename Type>
            inline Printer& operator<<(const std::vector<Type>& vector) {
                for (size_t index = 0; index < vector.size(); ++index) {
                    *this << vector[index];
                    if (index + 1 < vector.size()) putChar(' ');
                }
                return *this;
            }

            template<typename Type>
            inline Printer& operator<<(const core::FastVector<Type>& vector) {
                for (size_t index = 0; index < vector.size(); ++index) {
                    *this << vector[index];
                    if (index + 1 < vector.size()) putChar(' ');
                }
                return *this;
            }

            template<typename Type, size_t Size>
            inline Printer& operator<<(const std::array<Type, Size>& array) {
                for (size_t index = 0; index < Size; ++index) {
                    *this << array[index];
                    if (index + 1 < Size) putChar(' ');
                }
                return *this;
            }
        };

        inline Printer out;

        struct Scanner {
            static constexpr size_t BUFFER_SIZE = 1 << 17;
            char buffer[BUFFER_SIZE];
            char* pointer = buffer;
            char* end = buffer;
            bool endOfFileFlag = false;

            inline bool reload() {
                out.flush();
                int result = CALAFITE_READ(buffer, BUFFER_SIZE);
                if (result <= 0) {
                    endOfFileFlag = true;
                    return false;
                }
                pointer = buffer;
                end = buffer + result;
                return true;
            }

            inline int getChar() {
                if (CALAFITE_UNLIKELY(pointer == end)) {
                    if (endOfFileFlag || !reload()) return EOF;
                }
                return static_cast<unsigned char>(*pointer++);
            }

            inline int skipWhitespace() {
                int character;
                while ((character = getChar()) != EOF && character <= ' ') {}
                return character;
            }

            explicit operator bool() const { return !endOfFileFlag; }

            template<typename Type>
            inline std::enable_if_t<std::is_integral_v<Type>, Scanner&> operator>>(Type& value) {
                value = 0;
                int character = skipWhitespace();
                if (CALAFITE_UNLIKELY(character == EOF)) return *this;

                bool negative = false;
                if constexpr (std::is_signed_v<Type>) {
                    if (character == '-') {
                        negative = true;
                        character = getChar();
                    }
                }

                for (; static_cast<unsigned char>(character - '0') < 10; character = getChar()) {
                    value = value * 10 + (character - '0');
                }

                if constexpr (std::is_signed_v<Type>) {
                    if (negative) value = -value;
                }
                return *this;
            }

            inline Scanner& operator>>(char& character) {
                int result = skipWhitespace();
                if (result != EOF) character = static_cast<char>(result);
                return *this;
            }

            inline Scanner& operator>>(std::string& string) {
                string.clear();
                int character = skipWhitespace();
                if (CALAFITE_UNLIKELY(character == EOF)) return *this;
                do {
                    string.push_back(static_cast<char>(character));
                    character = getChar();
                } while (character != EOF && character > ' ');
                return *this;
            }

            inline Scanner& operator>>(char* string) {
                assert(string != nullptr);
                int character = skipWhitespace();
                if (CALAFITE_UNLIKELY(character == EOF)) {
                    *string = '\0';
                    return *this;
                }
                do {
                    *string++ = static_cast<char>(character);
                    character = getChar();
                } while (character != EOF && character > ' ');
                *string = '\0';
                return *this;
            }

            template<typename Type1, typename Type2>
            inline Scanner& operator>>(std::pair<Type1, Type2>& pair) {
                return *this >> pair.first >> pair.second;
            }

            template<typename Type>
            inline Scanner& operator>>(std::vector<Type>& vector) {
                for (Type& element : vector) *this >> element;
                return *this;
            }

            template<typename Type>
            inline Scanner& operator>>(core::FastVector<Type>& vector) {
                for (Type& element : vector) *this >> element;
                return *this;
            }

            template<typename Type, size_t Size>
            inline Scanner& operator>>(std::array<Type, Size>& array) {
                for (Type& element : array) *this >> element;
                return *this;
            }
        };

        inline Scanner in;

        template<typename... Args> inline void read(Args&... arguments) {
            (in >> ... >> arguments);
        }

        template<typename First, typename... Rest>
        inline void print(const First& first, const Rest&... rest) {
            out << first;
            ((out << ' ' << rest), ...);
        }
        inline void print() {}

        template<typename... Args> inline void println(const Args&... arguments) {
            print(arguments...);
            out << '\n';
        }
    }
}
