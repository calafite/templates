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
#define CALAFITE_LIKELY(x)   __builtin_expect(!!(x), 1)
#else
#define CALAFITE_UNLIKELY(x) (x)
#define CALAFITE_LIKELY(x)   (x)
#endif

namespace calafite {
    namespace io {

        class Printer {
        public:
            static constexpr size_t BUFFER_SIZE = 1 << 17;

        private:
            char buffer[BUFFER_SIZE];
            char* pointer = buffer;
            char numberLookupTable[200];

        public:
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

                char* p = pointer;
                using UnsignedType = std::make_unsigned_t<Type>;
                UnsignedType unsignedValue = value;

                if constexpr (std::is_signed_v<Type>) {
                    if (value < 0) {
                        *p++ = '-';
                        unsignedValue = 0 - unsignedValue;
                    }
                }

                if (unsignedValue == 0) {
                    *p++ = '0';
                    pointer = p;
                    return *this;
                }

                char temporary[32];
                size_t index = 32;

                if constexpr (sizeof(Type) > 4) {
                    while (unsignedValue >= 1000000000ULL) {
                        uint32_t chunk = static_cast<uint32_t>(unsignedValue % 1000000000ULL);
                        unsignedValue /= 1000000000ULL;
                        for (int i = 0; i < 4; ++i) {
                            index -= 2;
                            uint32_t remainder = chunk % 100;
                            chunk /= 100;
                            temporary[index] = numberLookupTable[remainder * 2];
                            temporary[index + 1] = numberLookupTable[remainder * 2 + 1];
                        }
                        temporary[--index] = static_cast<char>('0' + chunk);
                    }
                }
                
                uint32_t val32 = static_cast<uint32_t>(unsignedValue);

                while (val32 >= 100) {
                    index -= 2;
                    uint32_t remainder = val32 % 100;
                    val32 /= 100;
                    temporary[index] = numberLookupTable[remainder * 2];
                    temporary[index + 1] = numberLookupTable[remainder * 2 + 1];
                }
                
                if (val32 < 10) {
                    temporary[--index] = static_cast<char>('0' + val32);
                } else {
                    index -= 2;
                    temporary[index] = numberLookupTable[val32 * 2];
                    temporary[index + 1] = numberLookupTable[val32 * 2 + 1];
                }

                size_t len = 32 - index;
                std::memcpy(p, &temporary[index], len);
                pointer = p + len;
                
                return *this;
            }

            inline void writeSeq(int start, int step, int count) {
                char* p = pointer; 
                
                for (int i = 0, v = start; i < count; ++i, v += step) {
                    if (CALAFITE_UNLIKELY(p + 11 >= buffer + BUFFER_SIZE)) {
                        pointer = p; 
                        flush();
                        p = pointer; 
                    }
    
                    uint32_t u = static_cast<uint32_t>(v);
                    char tmp[12];
                    size_t index = 12;
                    
                    while (u >= 100) {
                        index -= 2;
                        uint32_t r = u % 100; u /= 100;
                        tmp[index] = numberLookupTable[r * 2];
                        tmp[index + 1] = numberLookupTable[r * 2 + 1];
                    }

                    if (u >= 10) {
                        index -= 2;
                        tmp[index] = numberLookupTable[u * 2];
                        tmp[index + 1] = numberLookupTable[u * 2 + 1];
                    } else {
                        tmp[--index] = static_cast<char>('0' + u);
                    }

                    size_t len = 12 - index;
                    std::memcpy(p, &tmp[index], len);
                    p += len;
                    *p++ = ' ';
                }
                
                pointer = p;
            }

            inline Printer& operator<<(char character) {
                putChar(character);
                return *this;
            }

            inline Printer& operator<<(const char* string) {
                assert(string != nullptr);
                size_t len = std::strlen(string);
                if (CALAFITE_UNLIKELY(pointer + len >= buffer + BUFFER_SIZE)) {
                    flush();
                    if (len >= BUFFER_SIZE) {
                        CALAFITE_WRITE(string, len);
                        return *this;
                    }
                }
                std::memcpy(pointer, string, len);
                pointer += len;
                return *this;
            }

            inline Printer& operator<<(const std::string& string) {
                size_t len = string.length();
                if (CALAFITE_UNLIKELY(pointer + len >= buffer + BUFFER_SIZE)) {
                    flush();
                    if (len >= BUFFER_SIZE) {
                        CALAFITE_WRITE(string.data(), len);
                        return *this;
                    }
                }
                std::memcpy(pointer, string.data(), len);
                pointer += len;
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

        class Scanner {
        public:
            static constexpr size_t BUFFER_SIZE = 1 << 17;

        private:
            char buffer[BUFFER_SIZE];
            char* pointer = buffer;
            char* end = buffer;
            bool endOfFileFlag = false;

        public:
            Scanner() = default;

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
                while (true) {
                    char* p = pointer;
                    while (p < end && *p <= ' ') ++p;
                    pointer = p;
                    if (p < end) return static_cast<unsigned char>(*pointer++);
                    if (!reload()) return EOF;
                }
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

                if (CALAFITE_LIKELY(end - pointer >= 32)) {
                    value = character - '0';
                    char* p = pointer;
                    while (static_cast<unsigned char>(*p - '0') < 10) {
                        value = value * 10 + (*p - '0');
                        ++p;
                    }
                    pointer = p;
                } else {
                    for (; static_cast<unsigned char>(character - '0') < 10; character = getChar()) {
                        value = value * 10 + (character - '0');
                    }
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
                
                string.push_back(static_cast<char>(character));

                while (true) {
                    char* p = pointer;
                    char* start = p;
                    while (p < end && *p > ' ') ++p;
                    
                    if (p > start) {
                        string.append(start, p - start);
                        pointer = p;
                    }
                    
                    if (p == end) {
                        if (!reload()) break;
                    } else {
                        break;
                    }
                }
                return *this;
            }

            inline Scanner& operator>>(char* string) {
                assert(string != nullptr);
                int character = skipWhitespace();
                if (CALAFITE_UNLIKELY(character == EOF)) {
                    *string = '\0';
                    return *this;
                }
                
                *string++ = static_cast<char>(character);
                
                while (true) {
                    char* p = pointer;
                    char* start = p;
                    while (p < end && *p > ' ') ++p;
                    
                    size_t len = p - start;
                    if (len > 0) {
                        std::memcpy(string, start, len);
                        string += len;
                        pointer = p;
                    }
                    
                    if (p == end) {
                        if (!reload()) break;
                    } else {
                        break;
                    }
                }
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
