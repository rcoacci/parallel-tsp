#include "BBSolver.hpp"

using namespace TSP;

Node::Node(const Matrix &costMatrix, City start)
    : reduced(costMatrix), cities({start}), cost(0) {
  cities.reserve(costMatrix.size() + 1);
  this->cost += reduced.reduce();
}

Node::Node(const Node &parent, City vertex)
    : reduced(parent.reduced), cities(parent.cities), cost(parent.cost) {
  City pv = parent.cities.back();
  this->cities.push_back(vertex);
  this->cost += parent.reduced(pv, vertex);
  // Adiciona caminho para cidade inicial
  if (cities.size() == reduced.size()) {
    this->cities.push_back(cities.front());
  }
  this->reduced(vertex, cities.front()) = Matrix::INF;
  // Change all entries of row i and column j to INF
  for (size_t k = 0; k < reduced.size(); k++) {

    // Set outgoing edges for the city i to INF
    this->reduced(pv, k) = Matrix::INF;

    // Set incoming edges to city j to INF
    this->reduced(k, vertex) = Matrix::INF;
  }
  this->cost += reduced.reduce();
}

bool Node::operator>(const Node &other) const { return cost > other.cost; }
bool Node::operator<(const Node &other) const { return cost < other.cost; }

std::vector<Node *> Node::children() {
  std::vector<Node *> st;
  for (size_t j = 0; j < reduced.size(); j++) {
    if (this->reduced(this->cities.back(), j) != Matrix::INF) {
      st.push_back(new Node(*this, j));
    }
  }
  std::sort(st.begin(), st.end(), std::greater<Node *>{});
  return st;
}

bool Node::is_feasible(Cost currentBest) { return cost < currentBest; }

void Node::printPath() { TSP::printPath(cities.begin(), cities.end()); }

void Node::printFoundSolution(int id, bool withPath) {
  std::cout << "[Proc " << id << "] Encontrou solucao com custo " << this->cost;
  if (withPath) {
    std::cout << ": ";
    this->printPath();
  }
  std::cout << "\n";
  std::cout << std::flush;
}
