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
using City = size_t;
static constexpr auto INF = std::numeric_limits<Cost>::max();
using Path = std::vector<City>;


class Matrix {
    std::vector<Cost>m;
    size_t N;
public:
    Matrix(size_t n): m(n*n), N(n){}
    Cost& operator()(City i, City j){return m[i*N+j];}
    Cost operator()(City i, City j) const {return m[i*N+j];}
    size_t size() const {return N;}
};

auto readMatrix(const std::filesystem::path& filename){
    auto ifs = std::ifstream(filename);
    size_t sz = 0;
    ifs>>sz;
    Matrix matrix(sz);
    for(size_t i = 0; i < sz; i++){
        for(size_t j = 0; j < sz; j++){
            ifs>>matrix(i,j);
            if(matrix(i,j) == (Cost)-1) matrix(i,j) = TSP::INF;
        }
    }
    Cost minCost = 0;
    ifs>>minCost;
    return std::make_tuple(matrix, minCost);
}

struct Node {
    Matrix reduced;
    Path cities;
    Cost cost;

    Node(const Matrix &costMatrix, City start = 0)
        : reduced(costMatrix), cities({start}), cost(0){
        cities.reserve(costMatrix.size()+1);
    }

    Node(const Node &parent, City vertex)
        : reduced(parent.reduced), cities(parent.cities), cost(parent.cost){
        City pv = parent.cities.back();
        this->cities.push_back(vertex);
        this->cost += parent.reduced(pv,vertex);
        // Adiciona caminho para cidade inicial
        if (cities.size() == reduced.size()) {
            this->cities.push_back(cities.front());
        }
        this->reduced(vertex, cities.front()) = INF;
        // Change all entries of row i and column j to INF
        for (size_t k = 0; k < reduced.size(); k++) {

            // Set outgoing edges for the city i to INF
            this->reduced(pv,k) = INF;

            // Set incoming edges to city j to INF
            this->reduced(k,vertex) = INF;
        }
    }

    bool operator>(const Node& other) const {return this->cost>other.cost;}
    bool operator<(const Node& other) const {return this->cost<other.cost;}

    Cost reduceRow() {
        Cost rowCost = 0;
        for (size_t i = 0; i < reduced.size(); i++) {
            Cost min_val = *std::min_element(&reduced(i,0), &reduced(i,reduced.size()));
            if (min_val != INF && min_val!=0) {
                rowCost+=min_val;
                for (size_t j = 0; j < reduced.size(); j++) {
                    if (reduced(i,j) != INF)
                        reduced(i,j) -= min_val;
                }
            }
        }
        return rowCost;
    }

    Cost reduceCol() {
        Cost colCost = 0;
        for (size_t j = 0; j < reduced.size(); j++) {
            Cost min_val = INF;
            for (size_t i = 0; i < reduced.size(); i++) {
                min_val = std::min(min_val, reduced(i,j));
            }
            if(min_val!=INF && min_val!=0){
                colCost+=min_val;
                for (size_t i = 0; i < reduced.size(); i++) {
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
        for (size_t j = 0; j < reduced.size(); j++) {
            if (this->reduced(this->cities.back(),j) != INF) {
                st.push_back(new Node(*this,j));
            }
        }
        std::sort(st.begin(), st.end(), std::less<>());
        return st;
    }

    bool is_feasible(Cost currentBest){
        calculateCost();
        return cost < currentBest;
    }

    void printPath(){
        std::cout<<"Path: ( " <<cities.front();
        std::for_each(cities.cbegin()+1, cities.cend(), [](const auto& item) {
            std::cout << " -> "<<item;
        });
        std::cout << " )\n";
    }

};
using Stack = std::stack<Node *>;
} // namespace TSP
