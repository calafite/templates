#pragma once

// core
#include "core/arena.hpp"
#include "core/fastVector.hpp"
#include "core/ops.hpp"
#include "core/iterator.hpp"

// container
#include "container/hashMap.hpp"
#include "container/priorityQueue.hpp"
#include "container/circularBuffer.hpp"

// dynamic programming
#include "dp/knapsack.hpp"
#include "dp/lcs.hpp"
#include "dp/lis.hpp"
#include "dp/sos.hpp"

// data structures
#include "ds/compressor.hpp"
#include "ds/disjointSetUnion.hpp"
#include "ds/fenwickTree.hpp"
#include "ds/lazySegmentTree.hpp"
#include "ds/segmentTree.hpp"
#include "ds/sparseTable.hpp"
#include "ds/trie.hpp"
#include "ds/monotonicStack.hpp"
#include "ds/cartesianTree.hpp"

// custom io
#include "io/io.hpp"

// mathematics
#include "maths/linearSieve.hpp"
#include "maths/matrix.hpp"
#include "maths/mint.hpp"
#include "maths/sieve.hpp"

// search algorithms
#include "search/dijkstra.hpp"
#include "search/graphBFS.hpp"
#include "search/graphDFS.hpp"
#include "search/treeBFS.hpp"
#include "search/treeDFS.hpp"

// string algorithms
#include "string/KnuthMorrisPratt.hpp"
#include "string/zAlgorithm.hpp"

// utilities
#include "utils/fixPoint.hpp"
#include "utils/utils.hpp"

// macros
#define _all(x) (x).begin(), (x).end()
#define _debug(x) cerr << #x << " = " << x << "\n"
#define _upto(var, start, end) for (auto var = start; var < end; var++)
#define _downto(var, start, end) for (auto var = start; var > end; var--)
#define _foreach(element, collection) for (auto &element : collection)

#define CALAFITE_SETUP                                          \
    using namespace std;                                        \
    using namespace calafite;                                   \
    using ll   = long long;                                     \
    using ull  = unsigned long long;                            \
    using vi   = ::calafite::core::FastVector<int>;             \
    using vll  = ::calafite::core::FastVector<ll>;              \
    using pii  = pair<int, int>;                                \
    using pll  = pair<ll, ll>;                                  \
    static constexpr int  INF   =  0x3f3f3f3f;                  \
    static constexpr int  NINF  = -0x3f3f3f3f;                  \
    static constexpr ll   LLINF =  0x3f3f3f3f3f3f3f3fLL;        \
    static constexpr ll   LNINF = -0x3f3f3f3f3f3f3f3fLL;        \
    using calafite::io::in;                                     \
    using calafite::io::out;                                    \
    using calafite::io::print;                                  \
    using calafite::io::println;                                \
    using calafite::io::read;                                   \
    
