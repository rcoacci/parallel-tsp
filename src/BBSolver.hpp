#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <vector>

namespace TSP {
template <std::size_t N> using Row = std::array<uint16_t, N>;
template <std::size_t N> using Matrix = std::array<Row<N>, N>;
using Edge = std::pair<size_t, size_t>;
using Path = std::vector<Edge>;
static constexpr auto INF = std::numeric_limits<uint16_t>::max();
using std::size_t;

template <std::size_t N>
struct Node {
    Matrix<N> reducedCost;
    Path path;
    std::size_t level;
    size_t vertex;
    int cost;

    Row<N> reduceRow() {
        Row<N> minRow;
        for (size_t i = 0; i < N; i++) {
            minRow[i] =
                *std::min_element(this->reducedCost[i].cbegin(), this->reducedCost[i].cend());
        }

        for (size_t i = 0; i < N; i++) {
            if (minRow[i] != INF) {
                for (size_t j = 0; j < N; j++) {
                    if (this->reducedCost[i][j] != INF)
                        this->reducedCost[i][j] -= minRow[i];
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
                minCol[j] = std::min(minCol[j], this->reducedCost[i][j]);
            }
        }

        for (size_t j = 0; j < N; j++) {
            if (minCol[j] != INF) {
                for (size_t i = 0; i < N; i++) {
                    if (this->reducedCost[i][j] != INF)
                        this->reducedCost[i][j] -= minCol[j];
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

    void printPath() const {
        std::for_each(path.cbegin(), path.cend(), [](Edge item) {
            std::cout << "(" << item.first << ", " << item.second << ") ";
        });
        std::cout << "\n";
    }

    Node(const Matrix<N> &costMatrix)
        : reducedCost(costMatrix), path(), level(0), vertex(0), cost(0) {
        calculateCost();
    }

    Node(const Node<N> &parent, size_t vertex)
        : reducedCost(parent.reducedCost), path(parent.path), level(parent.level + 1),
          vertex(vertex), cost(parent.cost) {
        this->path.push_back(std::make_pair(parent.vertex, vertex));
        this->cost += parent.reducedCost[parent.vertex][vertex];
        // Change all entries of row i and column j to INF
        if(level!=0){
            for (size_t k = 0; k < N; k++) {

                // Set outgoing edges for the city i to INF
                this->reducedCost[parent.vertex][k] = INF;

                // Set incoming edges to city j to INF
                this->reducedCost[k][vertex] = INF;
            }
        }
                // Adiciona caminho para cidade inicial
        if (level == N - 1) {
            this->path.push_back(std::make_pair(vertex, 0));
        }
        this->reducedCost[vertex][0] = INF;
        calculateCost();
    }
};
// Comparison object to be used to order the heap
template <std::size_t N>
struct Min_Heap {
    bool operator()(const Node<N> *lhs,
                    const Node<N> *rhs) const {
        return lhs->cost > rhs->cost;
    }
};
template <std::size_t N>
int solveTSP(Matrix<N> costMatrix, bool showPath) {
    std::priority_queue<Node<N> *, std::vector<Node<N> *>, Min_Heap<N>>
        queue;
    Path solution;
    int finalCost = 0;
    std::cout<<"Iniciando TSP com matrix "<<N<<"x"<<N<<".\n";
    std::cout<<"Tamanho do nó: "<<sizeof(Node<N>)<<" bytes.\n";
    std::cout<<"Tamanho de 1 nivel: "<<sizeof(Node<N>)*N<<" bytes.\n";

    Node<N> *root = new Node<N>(costMatrix);
    queue.push(root);
    while (!queue.empty()) {
        std::cout<<"Tamanho da fila: "<<queue.size()<<".\n";
        const Node<N> *min = queue.top();
        queue.pop();
        if (min->level == N - 1) {
            finalCost = min->cost;
            if(showPath) min->printPath();
            delete min;
            return finalCost;
        }
        for (size_t j = 0; j < N; j++) {
            if (min->reducedCost[min->vertex][j] != INF) {
                Node<N>* c = new Node<N>(*min, j);
                queue.push(c);
            }
        }
        delete min;
    }
    return finalCost;
}
} // namespace TSP
