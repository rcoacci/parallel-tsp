#include "BBSolver.hpp"
#include <cstdlib>
#include <omp.h>
#include <tuple>
#include <string>

using namespace std;

auto solveTSP(const TSP::Matrix& costMatrix) {
    using namespace TSP;
    Stack pq;
    Cost finalCost = INF;
    size_t N = costMatrix.size();
    std::cout<<"Iniciando TSP com matriz "<<N<<"x"<<N<<".\n";
    Node* root = new Node(costMatrix);
    root->calculateCost();
    std::unique_ptr<Node> solution;
    #pragma omp parallel private(pq)
    {
        #pragma omp for
        for(auto c: root->children()) pq.push(c); // Each thread gets a child of root and runs its subtree
        while (!pq.empty()) {
            auto min = unique_ptr<Node>(pq.top());
            pq.pop();
            if(min->is_feasible(finalCost)) {
                if (min->cities.size() == costMatrix.size()+1) {
                    #pragma omp critical (crit_sol)
                    {
                        solution = std::move(min);
                        finalCost = solution->cost;
                    }
                } else {
                    for (auto c: min->children()) pq.push(c);
                }
            }
        }
    }
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
    std::cout<<"Custo correto:    "<<bestSolution<<"\n";
    auto result = solveTSP(costMatrix);
    result.printPath();
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
