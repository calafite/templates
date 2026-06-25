#pragma once

#include "../core/fastVector.hpp"
#include <cassert>
#include <cstddef>
#include <utility>

namespace calafite {
    namespace string {

        template<typename Container> core::FastVector<size_t> prefixFunction(const Container& string) {
            size_t sizeValue = string.size();
            core::FastVector<size_t> prefixTable(sizeValue, 0);
            if (sizeValue == 0) return prefixTable;

            const auto* data = string.data();
            for (size_t index = 1; index < sizeValue; ++index) {
                size_t matchLength = prefixTable[index - 1];
                while (matchLength > 0 && data[index] != data[matchLength]) {
                    matchLength = prefixTable[matchLength - 1];
                }
                if (data[index] == data[matchLength]) ++matchLength;
                prefixTable[index] = matchLength;
            }
            return prefixTable;
        }

        template<typename Type> struct KnuthMorrisPratt {
            Type pattern;
            core::FastVector<size_t> prefixTable;
            size_t patternSize;
            size_t currentState = 0;

            KnuthMorrisPratt() : patternSize(0) {}

            explicit KnuthMorrisPratt(Type pattern)
                : pattern(std::move(pattern)), patternSize(this->pattern.size()) {
                prefixTable = prefixFunction(this->pattern);
            }

            template<typename Container> core::FastVector<size_t> search(const Container& text) const {
                core::FastVector<size_t> result;
                size_t textSize = text.size();
                if (patternSize == 0 || textSize == 0 || patternSize > textSize) return result;

                result.reserve(textSize / patternSize + 2);

                const auto* textData = text.data();
                const auto* patternData = pattern.data();
                size_t matchLength = 0;

                for (size_t index = 0; index < textSize; ++index) {
                    while (matchLength > 0 && textData[index] != patternData[matchLength]) {
                        matchLength = prefixTable[matchLength - 1];
                    }
                    if (textData[index] == patternData[matchLength]) {
                        ++matchLength;
                    }
                    if (matchLength == patternSize) {
                        result.pushBack(index - patternSize + 1);
                        matchLength = prefixTable[matchLength - 1];
                    }
                }
                return result;
            }

            template<typename Container> size_t findFirst(const Container& text) const {
                size_t textSize = text.size();
                if (patternSize == 0 || textSize == 0 || patternSize > textSize) return static_cast<size_t>(-1);

                const auto* textData = text.data();
                const auto* patternData = pattern.data();
                size_t matchLength = 0;

                for (size_t index = 0; index < textSize; ++index) {
                    while (matchLength > 0 && textData[index] != patternData[matchLength]) {
                        matchLength = prefixTable[matchLength - 1];
                    }
                    if (textData[index] == patternData[matchLength]) {
                        ++matchLength;
                    }
                    if (matchLength == patternSize) {
                        return index - patternSize + 1;
                    }
                }
                return static_cast<size_t>(-1);
            }

            void reset() { currentState = 0; }

            template<typename CharacterType> bool feed(const CharacterType& character) {
                if (patternSize == 0) return false;
                const auto* patternData = pattern.data();

                while (currentState > 0 && character != patternData[currentState]) {
                    currentState = prefixTable[currentState - 1];
                }
                if (character == patternData[currentState]) ++currentState;
                if (currentState == patternSize) {
                    currentState = prefixTable[currentState - 1];
                    return true;
                }
                return false;
            }
        };

    }
}
