#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <vector>

namespace TSP {
template <std::size_t N> using Row = std::array<int, N>;
template <std::size_t N> using Matrix = std::array<Row<N>, N>;
using Edge = std::pair<int, int>;
using Path = std::vector<Edge>;
static constexpr int INF = std::numeric_limits<int>::max();
using std::size_t;

template <std::size_t N> struct ReducedMatrixNode {
  Matrix<N> reduced;
  Path path;
  std::size_t level;
  int vertex;
  int cost;

  Row<N> reduceRow() {
    Row<N> minRow;
    for (size_t i = 0; i < N; i++) {
      minRow[i] =
          *std::min_element(this->reduced[i].cbegin(), this->reduced[i].cend());
    }

    for (size_t i = 0; i < N; i++) {
      if (minRow[i] != INF) {
        for (size_t j = 0; j < N; j++) {
          if (this->reduced[i][j] != INF)
            this->reduced[i][j] -= minRow[i];
        }
      }
    }
    return minRow;
  }

  Row<N> reduceCol() {
    Row<N> minCol;
    minCol.fill(INF);
    for (size_t i = 0; i < N; i++) {
      for (size_t j = 0; j < N; j++) {
        minCol[j] = std::min(minCol[j], this->reduced[i][j]);
      }
    }

    for (size_t j = 0; j < N; j++) {
      if (minCol[j] != INF) {
        for (size_t i = 0; i < N; i++) {
          if (this->reduced[i][j] != INF)
            this->reduced[i][j] -= minCol[j];
        }
      }
    }
    return minCol;
  }
  void calculateCost() {
    Row<N> minRow = reduceRow();
    Row<N> minCol = reduceCol();
    for (size_t i = 0; i < N; i++) {
      this->cost += (minRow[i] != INF ? minRow[i] : 0) +
                    (minCol[i] != INF ? minCol[i] : 0);
    }
  }

  void printPath() {
    std::for_each(path.cbegin(), path.cend(), [](Edge item) {
      std::cout << "(" << item.first << ", " << item.second << ") ";
    });
    std::cout << "\n";
  }

  ReducedMatrixNode(const Matrix<N> &costMatrix)
      : reduced(costMatrix), path(), level(0), vertex(0), cost(0) {
    calculateCost();
  }

  ReducedMatrixNode(const ReducedMatrixNode<N> &parent, size_t vertex)
      : reduced(parent.reduced), path(parent.path), level(parent.level + 1),
        vertex(vertex), cost(parent.cost) {
    this->path.push_back(std::make_pair(parent.vertex, vertex));
    this->cost += parent.reduced[parent.vertex][vertex];
    // Change all entries of row i and column j to INF
    for (size_t k = 0; level != 0 && k < N; k++) {

      // Set outgoing edges for the city i to INF
      this->reduced[parent.vertex][k] = INF;

      // Set incoming edges to city j to INF
      this->reduced[k][vertex] = INF;
    }
    // Adiciona caminho para cidade inicial
    if (level == N - 1) {
      this->path.push_back(std::make_pair(vertex, 0));
    }
    this->reduced[vertex][0] = INF;
    calculateCost();
  }
};
// Comparison object to be used to order the heap
template <std::size_t N> struct Min_Heap {
  bool operator()(const ReducedMatrixNode<N> *lhs,
                  const ReducedMatrixNode<N> *rhs) const {
    return lhs->cost > rhs->cost;
  }
};
template <std::size_t N> int solveTSP(Matrix<N> costMatrix) {
  std::priority_queue<ReducedMatrixNode<N> *,
                      std::vector<ReducedMatrixNode<N> *>, Min_Heap<N>>
      queue;
  Path solution;
  int finalCost = 0;
  ReducedMatrixNode<N> *root = new ReducedMatrixNode<N>(costMatrix);
  queue.push(root);
  while (!queue.empty()) {
    ReducedMatrixNode<N> *min = queue.top();
    queue.pop();
    if (min->level == N - 1) {
      finalCost = min->cost;
      min->printPath();
      delete min;
      return finalCost;
    }
    for (size_t j = 0; j < N; j++) {
      if (min->reduced[min->vertex][j] != INF) {
        queue.push(new ReducedMatrixNode<N>(*min, j));
      }
    }
    delete min;
  }
  return finalCost;
}
} // namespace TSP
