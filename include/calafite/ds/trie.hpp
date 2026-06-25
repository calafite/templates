#pragma once

#include "../core/fastVector.hpp"
#include <cassert>
#include <cstddef>

namespace calafite {
    namespace ds {

        template<size_t AlphabetSize = 26, int Offset = 'a'>
        struct Trie {
            struct Node {
                int next[AlphabetSize];
                int words = 0;
                int prefixes = 0;

                Node() {
                    for (size_t index = 0; index < AlphabetSize; ++index) {
                        next[index] = -1;
                    }
                }
            };

            core::FastVector<Node> nodes;

            Trie() {
                nodes.emplaceBack(); 
            }

            inline int getCharIndex(int character) const {
                return character - Offset;
            }

            template<typename Container>
            void insert(const Container& sequence) {
                int current = 0;
                nodes[current].prefixes++;
                
                for (auto character : sequence) {
                    int edge = getCharIndex(character);
                    assert(edge >= 0 && edge < static_cast<int>(AlphabetSize));
                    
                    if (nodes[current].next[edge] == -1) {
                        nodes[current].next[edge] = static_cast<int>(nodes.size());
                        nodes.emplaceBack();
                    }
                    
                    current = nodes[current].next[edge];
                    nodes[current].prefixes++;
                }
                
                nodes[current].words++;
            }

            template<typename Container>
            bool erase(const Container& sequence) {
                if (count(sequence) == 0) return false;
                
                int current = 0;
                nodes[current].prefixes--;
                
                for (auto character : sequence) {
                    int edge = getCharIndex(character);
                    current = nodes[current].next[edge];
                    nodes[current].prefixes--;
                }
                
                nodes[current].words--;
                return true;
            }

            template<typename Container>
            int count(const Container& sequence) const {
                int current = 0;
                
                for (auto character : sequence) {
                    int edge = getCharIndex(character);
                    if (edge < 0 || edge >= static_cast<int>(AlphabetSize)) return 0;
                    if (nodes[current].next[edge] == -1) return 0;
                    current = nodes[current].next[edge];
                }
                
                return nodes[current].words;
            }

            template<typename Container>
            int countPrefix(const Container& sequence) const {
                int current = 0;
                
                for (auto character : sequence) {
                    int edge = getCharIndex(character);
                    if (edge < 0 || edge >= static_cast<int>(AlphabetSize)) return 0;
                    if (nodes[current].next[edge] == -1) return 0;
                    current = nodes[current].next[edge];
                }
                
                return nodes[current].prefixes;
            }

            void clear() {
                nodes.clear();
                nodes.emplaceBack();
            }
        };

    }
}
