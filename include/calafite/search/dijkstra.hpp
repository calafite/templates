#pragma once

#include "../core/fastVector.hpp"
#include <cassert>
#include <cstddef>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

namespace calafite {
    namespace search {

        template<typename WeightType>
        struct Dijkstra {
            static constexpr size_t unvisited = static_cast<size_t>(-1);

            core::FastVector<WeightType> distances;
            core::FastVector<size_t> parents;
            WeightType unreachable;

            Dijkstra(size_t start, const core::FastVector<core::FastVector<std::pair<size_t, WeightType>>>& adjacencyList, 
                     bool recordPaths = false, WeightType unreachableValue = std::numeric_limits<WeightType>::max())
                : unreachable(unreachableValue) {
                
                size_t nodeCount = adjacencyList.size();
                assert(start < nodeCount);

                distances.assign(nodeCount, unreachable);
                if (recordPaths) {
                    parents.assign(nodeCount, unvisited);
                }

                using State = std::pair<WeightType, size_t>;
                std::priority_queue<State, std::vector<State>, std::greater<State>> queue;

                distances[start] = WeightType(0);
                queue.push({WeightType(0), start});

                while (!queue.empty()) {
                    State top = queue.top();
                    queue.pop();

                    WeightType currentDist = top.first;
                    size_t current = top.second;

                    if (currentDist > distances[current]) continue;

                    for (const auto& edge : adjacencyList[current]) {
                        size_t neighbor = edge.first;
                        WeightType weight = edge.second;
                        
                        WeightType newDist = currentDist + weight;
                        if (newDist < distances[neighbor]) {
                            distances[neighbor] = newDist;
                            if (recordPaths) parents[neighbor] = current;
                            queue.push({newDist, neighbor});
                        }
                    }
                }
            }

            Dijkstra(const core::FastVector<size_t>& starts, const core::FastVector<core::FastVector<std::pair<size_t, WeightType>>>& adjacencyList, 
                     bool recordPaths = false, WeightType unreachableValue = std::numeric_limits<WeightType>::max())
                : unreachable(unreachableValue) {
                
                size_t nodeCount = adjacencyList.size();

                distances.assign(nodeCount, unreachable);
                if (recordPaths) {
                    parents.assign(nodeCount, unvisited);
                }

                using State = std::pair<WeightType, size_t>;
                std::priority_queue<State, std::vector<State>, std::greater<State>> queue;

                for (size_t start : starts) {
                    assert(start < nodeCount);
                    if (distances[start] == unreachable) {
                        distances[start] = WeightType(0);
                        queue.push({WeightType(0), start});
                    }
                }

                while (!queue.empty()) {
                    State top = queue.top();
                    queue.pop();

                    WeightType currentDist = top.first;
                    size_t current = top.second;

                    if (currentDist > distances[current]) continue;

                    for (const auto& edge : adjacencyList[current]) {
                        size_t neighbor = edge.first;
                        WeightType weight = edge.second;
                        
                        WeightType newDist = currentDist + weight;
                        if (newDist < distances[neighbor]) {
                            distances[neighbor] = newDist;
                            if (recordPaths) parents[neighbor] = current;
                            queue.push({newDist, neighbor});
                        }
                    }
                }
            }

            core::FastVector<size_t> getPath(size_t target) const {
                assert(!parents.empty() && "Paths were not tracked. Initialize the Dijkstra struct with recordPaths = true.");
                assert(target < distances.size());

                if (distances[target] == unreachable) return {};

                core::FastVector<size_t> path;
                for (size_t current = target; current != unvisited; current = parents[current]) {
                    path.pushBack(current);
                }
                
                path.reverse();
                return path;
            }
        };

    }
}
