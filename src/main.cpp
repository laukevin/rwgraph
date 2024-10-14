#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <exception>
#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <cxxopts.hpp>
#include <graph.h>

// Todo
//   use shared memory
//   add pybindings

/*
 * Reads in the data stored as a 1 way adjacency list and returns
 # the adjacency list going both ways.
 * The maps std::unordered_map<int, int> and
 * std::unordered_map<int, std::string> are translations for nodes U and V
 * to consecutive elements in the adjacency graph std::unordered_map<int, std::vector<int>>.
 *
 * textData points to a text file of the form
 * uId vIdx1 vIdx2 vIdx3 vIdx4 vIdx5
 * ...
 * The return types will be a map of
 *
 */
std::tuple<
    std::unique_ptr<std::unordered_map<int, std::vector<int>>>,
    std::unique_ptr<std::unordered_map<int, std::string>>,
    std::unique_ptr<std::unordered_map<int, std::string>>
> readGraph(std::string& textData) {
    std::ifstream file(textData);

    if (!file) {
        std::cerr << "File not found" + textData << std::endl;
        throw std::runtime_error("File not found");
    }

    std::string line;

    auto idxToUNode = std::make_unique<std::unordered_map<int, std::string>>();
    std::unordered_map<int, std::string> idxToVNode;
    std::unordered_map<std::string, int> vNodeToIdx;
    int uNodeIdx = 0;
    int vNodeIdx = 0;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string uNode;
        iss >> uNode;
        (*idxToUNode)[uNodeIdx] = uNode;
        uNodeIdx++;

        std::string neighbor;
        while (iss >> neighbor) {
            if (vNodeToIdx.find(neighbor) == vNodeToIdx.end()) {
                idxToVNode[vNodeIdx] = neighbor;
                vNodeToIdx[neighbor] = vNodeIdx;
                vNodeIdx++;
            }
        }
    }

    file.clear();
    file.seekg(0);

    auto adjMatrix = std::make_unique<std::unordered_map<int, std::vector<int>>>(uNodeIdx + vNodeIdx + 1);
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int node;
        iss >> node;

        std::vector<int> neighbors;
        std::string neighbor;
        while (iss >> neighbor) {
            int neighborId = vNodeToIdx[neighbor] + uNodeIdx;
            // Mapping of U nodes to V
            (*adjMatrix)[node].push_back(neighborId);
            // Mapping of V nodes back to U
            (*adjMatrix)[neighborId].push_back(node);
        }
    }

    int numBelow = 0;
    int numAbove = 0;
    for (const auto& [k,v]: *adjMatrix) {
        if (k < uNodeIdx) {
            numBelow++;
        } else {
            numAbove++;
        }
    }

    auto shiftedIdxToVNode = std::make_unique<std::unordered_map<int, std::string>>();
    for (const auto& [vNodeId, vNodeStr] : idxToVNode) {
        int shiftedVIdx = vNodeId + uNodeIdx;
        (*shiftedIdxToVNode)[shiftedVIdx] = vNodeStr;
    }

    return std::make_tuple(
        std::move(adjMatrix),
        std::move(idxToUNode),
        std::move(shiftedIdxToVNode)
    );
}

bool readInputs(int argc, char*argv[], bool debug, std::string& inputDataPath, std::string& outputDataPath) {
    try {
        cxxopts::Options options("program", "A brief description of the program");
        options.add_options()
            ("i,input", "Input data path", cxxopts::value<std::string>())
            ("o,output", "Output data path", cxxopts::value<std::string>())
            ("h,help", "Print help");

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return false;
        }

        if (debug) {
            inputDataPath = result["input"].as<std::string>();
            outputDataPath = result["output"].as<std::string>();
            std::cout << "Input data path: " << inputDataPath << std::endl;
            std::cout << "Output data path: " << outputDataPath << std::endl;
        }
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        return false;
    }
    return true;
}


int main(int argc, char* argv[]) {
    std::string inputDataPath;
    std::string outputDataPath;
    bool validInputs = readInputs(argc, argv, true, inputDataPath, outputDataPath);
    if (!validInputs) {
       return 1;
    }

    // inputDataPath is a list of nodes u => list of v
    auto [adjMatrix, uNodeIndex, vNodeIndex] = readGraph(inputDataPath);
    auto graph = rwgraph::Graph::create_graph(std::move(adjMatrix));

    auto results = graph->walk(25, 100, 3);
    for (const auto& [nodeId, counts] : results) {
        std::cout << nodeId << " " << counts << std::endl;
    }

    std::cout << "Number unodes: " <<  uNodeIndex->size() << std::endl;
}
