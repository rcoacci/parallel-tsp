#include "BBSolver.hpp"
//#include "nn_tsp.hpp"
#include <cstdlib>
#include <omp.h>
#include <tuple>
#include <string>
#include <filesystem>
#include <fstream>
#include <mpi/mpi.h>

using namespace std;
using namespace TSP;

auto solve(Node* root){
    std::stack<Node*> st;
    size_t N = root->N;
    Node* solution = root;
    Cost bestCost = INF;
    st.push(root);
    size_t max_q_size = std::numeric_limits<size_t>::min();
    while (!st.empty()) {
        max_q_size = std::max(max_q_size, st.size());
        Node *min = st.top();
        st.pop();
        if (min->path.size() == N+1) {
            if(min->cost < bestCost) {
                delete solution;
                solution = min;
                bestCost = solution->cost;
            }
        } else if(min->cost < bestCost){
            MaxHeap queue; // Usa uma max heap para ordenar os filhos por maior custo
            for (size_t j = 0; j < N; j++) {
                if (min->reducedCost(min->vertex,j) != INF) {
                    Node* c = new Node(*min, j);
                    queue.push(c);
                }
            }
            // Adiciona os filhos na pilha em ordem do maior custo para o menor custo.
            // Dessa forma o filho de menor custo vai estar no topo na proxima iteracao.
            // Essa é uma maneira "hibrida" de fazer um Depth First com caracteristicas de "Best First" sem usar muita memoria
            while(!queue.empty()){
                st.push(queue.top());
                queue.pop();
            }
        }
        if(min!=solution) delete min;
    }
    return solution;
}

int main(int argc, char *argv[]){

    TSP::Matrix input(0);
    TSP::Cost bestSolution;
    MPI_Init(&argc, &argv);
    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_ARE_FATAL);
    int numProcs = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    int id = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if(argc<2) {
        std::cerr<<"Falta arquivo de dados!";
        exit(-1);
    }

    auto data_file = filesystem::path{argv[1]};
    std::tie(input,bestSolution) = readMatrix(data_file);
    Node *root = new Node(input);

    // MinHeap queue;
    // for (size_t j = id; j < input.N; j+=numProcs) {
    //     std::cout<<"[Proc "<<id<<"]: Executando "<<j<<"\n";
    //     if (root->reducedCost(root->vertex,j) != INF) {
    //         Node* c = new Node(*root, j);
    //         queue.push(solve(c));
    //     }
    // }
    Node* localBest = solve(root);
    std::cout<<"[Proc "<<id<<"]: Melhor "<<localBest->cost<<"\n";
    vector<TSP::Cost> solutions(numProcs);
    MPI_Gather(&localBest->cost, 1, MPI_UINT16_T, solutions.data(), 1, MPI_UINT16_T, 0,MPI_COMM_WORLD);
    MPI_Finalize();
    if(id==0){
        TSP::Cost result = *std::min_element(solutions.begin(),solutions.end());
        if(result != bestSolution){
            std::cout<<"************* Custo INCORRETO! *****************\n";
            std::cout<<"Custo correto:    "<<bestSolution<<"\n";
            std::cout<<"Custo encontrado: "<<result<<"\n";
            return -1;
        } else {
            std::cout<<"Custo correto!\n";
        }
    }
    return 0;
}
