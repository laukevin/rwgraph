#include <iostream>
#include <cassert>
#include <condition_variable>
#include <graph.h>
#include <threadpool.h>
#include <string_view>
#include <sys/_types/_int32_t.h>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <memory>

namespace rwgraph {
    std::unique_ptr<Graph> Graph::create_graph(
        std::unique_ptr<std::unordered_map<int, std::vector<int>>> adjMatrix) {
        std::vector<int32_t> rowPtrs;
        std::vector<int32_t> values;

        // Step 1: Extract keys into a vector
        std::vector<int32_t> keys;
        for (const auto& pair : *adjMatrix) {
            keys.push_back(pair.first);
        }
        std::sort(keys.begin(), keys.end());

        std::unordered_map<int32_t, int32_t> nodeIds;
        int rowPtr = 0;
        int nodePosn = 0;
        for (const auto& nodeId: keys) {
            auto neighbors = (*adjMatrix)[nodeId];
            nodeIds[nodeId] = nodePosn;
            nodePosn++;
            rowPtrs.push_back(rowPtr);
            for (const auto& n: neighbors) {
                values.push_back(n);
                rowPtr++;
            }
        }

        return std::make_unique<Graph>(
            std::make_unique<const std::vector<int32_t>>(rowPtrs),
            std::make_unique<const std::unordered_map<int32_t, int32_t>>(nodeIds),
            std::make_unique<const std::vector<int32_t>>(values)
        );
    }

    std::unordered_map<int32_t, int32_t> Graph::walkSingleThread(int nodeId, int numWalks, int numSteps) const {
        thread_local std::mt19937 gen(std::random_device{}());

        std::vector<int> walk;
        for (int w = 0; w < numWalks; w++) {
            int currNodeId = nodeId;
            for (int i = 0; i < numSteps * 2; i++) {
                auto nodeIdx = (*nodeIdsToIdx).at(currNodeId);
                auto start = (*rowPtrs)[nodeIdx];
                auto end = (*rowPtrs)[nodeIdx + 1];
                if (end - start - 1 <= 0) {
                    break;
                }

                std::uniform_int_distribution<> distrib(0, end - start - 1);
                int randomOffset = distrib(gen);

                currNodeId = (*values)[start + randomOffset];
                // only collect the nodes in the walk that are bipartite
                if (i % 2 == 0) {
                    walk.push_back(currNodeId);
                }
            }
        }

        std::unordered_map<int32_t, int32_t> walkCounts;
        for (const auto& nodeId : walk) {
            walkCounts[nodeId] += 1;
        }
        return walkCounts;
    }

    std::unordered_map<int32_t, int32_t> Graph::walk(int nodeId, int numWalks, int numSteps) {
        int walksPerThread = std::max(numWalks / numThreads, 1);
        std::unordered_map<int32_t, int32_t> allResults;
        std::queue<std::unordered_map<int32_t, int32_t>> resultsQueue;
        std::mutex queueMutex;
        std::condition_variable queueCondition;

        for (int i = 0; i < numThreads; i++) {
            auto walkResults = pool.enqueue([this, &resultsQueue, &queueMutex, &queueCondition, i](int nodeId, int walksPerThread, int numSteps) {
                auto walkRes = this->walkSingleThread(nodeId, walksPerThread, numSteps);
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    resultsQueue.push(std::move(walkRes));
                }
                queueCondition.notify_one();
            }, nodeId, walksPerThread, numSteps);
        }

        int numProcessed = 0;
        while (true) {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [&resultsQueue]{ return !resultsQueue.empty(); });
            auto result = std::move(resultsQueue.front());
            resultsQueue.pop();
            lock.unlock();

            for (const auto& [k, v]: result) {
                if (allResults.find(k) == allResults.end()) {
                    allResults[k] = 0;
                }
                allResults[k] += v;
            }
            numProcessed += 1;
            if (numProcessed == numThreads) {
                return allResults;
            }
        }
    }
}
