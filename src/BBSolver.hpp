#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <vector>
#include <stack>
#include <tuple>
#include <memory>
#include <filesystem>
#include <fstream>

namespace TSP {

using Cost = uint16_t;
static constexpr auto INF = std::numeric_limits<Cost>::max();
using Path = std::vector<size_t>;
using std::size_t;


void printPath(const TSP::Path& solution){
    std::cout<<"Path: ( " <<solution.front();
    std::for_each(solution.cbegin()+1, solution.cend(), [](const auto& item) {
        std::cout << " -> "<<item;
    });
    std::cout << " )\n";
}


struct Matrix {
    std::vector<Cost>m;
    size_t N;
    Matrix(size_t n): m(n*n), N(n){}
    //Matrix(const Matrix& other) = default; //:m(other.m), N(other.N){}
    Cost& operator()(size_t i, size_t j){return m[i*N+j];}
    Cost operator()(size_t i, size_t j) const {return m[i*N+j];}
    size_t size() const {return N;}
};

auto readMatrix(const std::filesystem::path& filename){
    auto ifs = std::ifstream(filename);
    size_t sz = 0;
    ifs>>sz;
    TSP::Matrix matrix(sz);
    for(size_t i = 0; i < sz; i++){
        for(size_t j = 0; j < sz; j++){
            ifs>>matrix(i,j);
            if(matrix(i,j) == (TSP::Cost)-1) matrix(i,j) = TSP::INF;
        }
    }
    TSP::Cost minCost = 0;
    ifs>>minCost;
    return std::make_tuple(matrix, minCost);
}

struct Node {
    Matrix reducedCost;
    Path path;
    size_t vertex;
    Cost cost;
    size_t N;

    Node(const Matrix &costMatrix, size_t start = 0)
        : reducedCost(costMatrix), path({start}), vertex(start), cost(0), N(costMatrix.size()) {
        path.reserve(N+1);
    }

    Node(const Node &parent, size_t vertex)
        : reducedCost(parent.reducedCost), path(parent.path), vertex(vertex), cost(parent.cost), N(parent.N){
        this->path.push_back(vertex);
        this->cost += parent.reducedCost(parent.vertex,vertex);
        // Adiciona caminho para cidade inicial
        if (path.size() == N) {
            this->path.push_back(path.front());
        }
        this->reducedCost(vertex,0) = INF;
        // Change all entries of row i and column j to INF
        for (size_t k = 0; k < N; k++) {

            // Set outgoing edges for the city i to INF
            this->reducedCost(parent.vertex,k) = INF;

            // Set incoming edges to city j to INF
            this->reducedCost(k,vertex) = INF;
        }
    }

    bool operator>(const Node& other) const {return this->cost>other.cost;}
    bool operator<(const Node& other) const {return this->cost<other.cost;}

    Cost reduceRow() {
        Cost rowCost = 0;
        Matrix& reduced = this->reducedCost;
        for (size_t i = 0; i < N; i++) {
            Cost min_val = *std::min_element(&reduced(i,0), &reduced(i,N));
            if (min_val != INF && min_val!=0) {
                rowCost+=min_val;
                for (size_t j = 0; j < N; j++) {
                    if (reduced(i,j) != INF)
                        reduced(i,j) -= min_val;
                }
            }
        }
        return rowCost;
    }

    Cost reduceCol() {
        Cost colCost = 0;
        Matrix& reduced = this->reducedCost;
        for (size_t j = 0; j < N; j++) {
            Cost min_val = INF;
            for (size_t i = 0; i < N; i++) {
                min_val = std::min(min_val, reduced(i,j));
            }
            if(min_val!=INF && min_val!=0){
                colCost+=min_val;
                for (size_t i = 0; i < N; i++) {
                    if (reduced(i,j) != INF)
                        reduced(i,j) -= min_val;
                }
            }
        }
        return colCost;
    }

    void calculateCost() {
        Cost rowCost = reduceRow();
        Cost colCost = reduceCol();
        this->cost+=rowCost+colCost;
    }

    std::vector<Node*> children(){
        std::vector<Node*> st;
        for (size_t j = 0; j < N; j++) {
            if (this->reducedCost(this->vertex,j) != INF) {
                st.push_back(new Node(*this,j));
            }
        }
        std::sort(st.begin(), st.end(), std::less<>());
        return st;
    }
};
using MaxHeap = std::priority_queue<Node *>;
using MinHeap = std::priority_queue<Node*, std::vector<Node*>, std::greater<Node*>>;
using Stack = std::stack<Node *>;

void eval(Node* n, Cost& finalCost, std::unique_ptr<Node>& solution){
    std::unique_ptr<Node> min(n);
    min->calculateCost();
    auto N = min->N;
    if(min->cost < finalCost) {
        if (min->path.size() == N+1) {
            #pragma omp critical (crit_sol)
            {
                solution = std::move(min);
                finalCost = solution->cost;
            }
        } else {
            auto children = min->children();
            #pragma omp parallel master taskloop shared(solution)
            for(auto c: children){
                eval(c, finalCost, solution);
            }
        }
    }
}

auto solveTSP(const Matrix& costMatrix) {
    // Stack pq;
    Cost finalCost = INF;
    size_t N = costMatrix.size();
    std::cout<<"Iniciando TSP com matriz "<<N<<"x"<<N<<".\n";
    std::unique_ptr<Node> root = std::make_unique<Node>(costMatrix);
    root->calculateCost();
    // pq.push(root);
    std::unique_ptr<Node> solution;// = root;
    eval(root.release(), finalCost, solution);
    // while (!pq.empty()) {
    //     auto min = std::unique_ptr<Node>(pq.top());
    //     pq.pop();
    //     auto children = eval(min, finalCost, solution);
    //     for (auto c: children) pq.push(c);
    // }
    return *solution;
}
} // namespace TSP
