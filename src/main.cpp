#include "BBSolver.hpp"
#include <cstdlib>
#include <omp.h>
#include <tuple>
#include <string>

using namespace std;


#ifndef _OPENMP
#define omp_get_thread_num() 0
#endif

static int verbose = 0;
using namespace TSP;

auto solveTSP(const TSP::Matrix& costMatrix, TSP::City start = 0) {
    MinHeap pq;
    Cost finalCost = INF;
    size_t N = costMatrix.size();
    std::cout<<"Iniciando TSP com matriz "<<N<<"x"<<N<<".\n";
    Node* root = new Node(costMatrix, start);
    std::unique_ptr<Node> solution;
    size_t max_stack_size = 0;
    #pragma omp parallel private(pq) reduction(max: max_stack_size)
    {
        #pragma omp for
        for(auto c: root->children()) pq.push(c); // Each thread gets a child of root and runs its subtree
        while (!pq.empty()) {
            max_stack_size = std::max(max_stack_size, pq.size());
            auto min = unique_ptr<Node>(pq.top());
            pq.pop();
            if(min->is_feasible(finalCost)) {
                if (min->cities.size() == costMatrix.size()+1) {
                    #pragma omp critical (crit_sol)
                    {
                        solution = std::move(min);
                        finalCost = solution->cost;
                    }
                    if(verbose) solution->printFoundSolution(omp_get_thread_num());
                } else {
                    auto children = min->children();
                    for(auto c: children){ pq.push(c); }
                }
            }
        }
    }
    cout<<"Max stack: "<<max_stack_size<<endl;
    return *solution;
}

int main(int argc, char *argv[]){
    if(argc<2) {
        std::cerr<<"Falta arquivo de dados!";
        exit(-1);
    }
    auto data_file = filesystem::path{argv[1]};
    if(!filesystem::exists(data_file)){
        std::cerr<<"Falta arquivo de dados!";
        exit(-1);
    }
    std::cout.sync_with_stdio(false);
#ifdef _OPENMP
    long nthreads = omp_get_max_threads();
    if(argc>2){
        nthreads = std::strtol(argv[2], nullptr, 10);
    }
    std::cout<<"Usando "<<nthreads<<" threads.\n";
    omp_set_num_threads(nthreads);
#endif
    TSP::Matrix costMatrix(0);
    TSP::Cost bestSolution;
    std::tie(costMatrix,bestSolution) = TSP::readMatrix(data_file);
    std::cout<<"Custo correto:    "<<bestSolution<<"\n"<<std::flush;
    auto result = solveTSP(costMatrix);
    result.printPath();
    cout<<"\n";
    if(result.cost != bestSolution){
        std::cout<<"************* Custo INCORRETO! *****************\n";
        std::cout<<"Custo correto:    "<<bestSolution<<"\n";
        std::cout<<"Custo encontrado: "<<result.cost<<"\n";
        return -1;
    } else {
        std::cout<<"Custo correto encontrado!\n";
    }
    return 0;
}
