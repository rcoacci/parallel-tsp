#include <algorithm>
#include <iostream>
#include <queue>
#include <stack>
#include "Matrix.hpp"

namespace TSP {

using Path = std::vector<City>;

template<typename Iterator>
void printPath(const Iterator& begin, const Iterator& end){
    std::cout<<*begin;
    std::for_each(begin+1, end, [](const auto& n){ std::cout<<"->"<<n; });
}

struct Node {
    Matrix reduced;
    Path cities;
    Cost cost;

    Node(const Matrix &costMatrix, City start = 0);

    Node(const Node &parent, City vertex);

    bool operator>(const Node& other) const;
    bool operator<(const Node& other) const;

    std::vector<Node*> children();

    bool is_feasible(Cost currentBest);

    void printPath();
    void printFoundSolution(int id, bool withPath=false);

};
using Stack = std::stack<Node *>;
using MinHeap = std::priority_queue<Node*, std::vector<Node*>, std::less<Node*>>;

} // namespace TSP

template<>
struct std::less<TSP::Node*>{
    inline bool operator()(const TSP::Node* lhs, const TSP::Node* rhs){return *lhs<*rhs;}
};

template<>
struct std::greater<TSP::Node*>{
    inline bool operator()(const TSP::Node* lhs, const TSP::Node* rhs){return *lhs>*rhs;}
};
