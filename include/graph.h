#pragma once

#include <string_view>
#include <sys/_types/_int32_t.h>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <threadpool.h>
#include <memory>

namespace rwgraph {
    enum NodeType {
        UNODE,
        VNODE
    };

    class NodeLookup {
        public:
            NodeLookup(
                std::unique_ptr<std::unordered_map<int, std::string>> uNodes,
                std::unique_ptr<std::unordered_map<int, std::string>> vNodes): uNodeLookup(
                    std::move(uNodes)), vNodeLookup(std::move(vNodes)) {}
            std::tuple<NodeType, std::string> getNode(int nodeId) const;

        private:
            std::unique_ptr<std::unordered_map<int, std::string>> uNodeLookup;
            std::unique_ptr<std::unordered_map<int, std::string>> vNodeLookup;
    };

    class Graph {
        public:
            Graph(
                std::unique_ptr<const std::vector<int32_t>> rowPtrs,
                std::unique_ptr<const std::unordered_map<int32_t, int32_t>> nodeIdsToIdx,
                std::unique_ptr<const std::vector<int32_t>> values,
                int numThreads=4)
            : rowPtrs(std::move(rowPtrs))
            , nodeIdsToIdx(std::move(nodeIdsToIdx))
            , values(std::move(values))
            , pool(numThreads)
            , numThreads(numThreads) {}
            static std::unique_ptr<Graph> create_graph(std::unique_ptr<std::unordered_map<int, std::vector<int>>> adjMatrix);
            std::unordered_map<int32_t, int32_t> walkSingleThread(int nodeId, int numWalks, int numSteps) const;
            std::unordered_map<int32_t, int32_t> walk(int nodeId, int numWalks, int numSteps);

        private:
            std::unique_ptr<const std::vector<int32_t>> rowPtrs;
            std::unique_ptr<const std::unordered_map<int32_t, int32_t>> nodeIdsToIdx;
            std::unique_ptr<const std::vector<int32_t>> values;
            rwgraph::ThreadPool pool;
            int numThreads;
    };
}
