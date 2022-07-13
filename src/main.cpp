#include "BBSolver.hpp"
#include <cstdlib>
#include <omp.h>


int main(int argc, char *argv[]){
    long nthreads = omp_get_max_threads();
    if(argc>0){
        nthreads = std::strtol(argv[1], nullptr, 10);
    }
    std::cout<<"Usando "<<nthreads<<" threads.\n";
    omp_set_num_threads(nthreads);
    using TSP::INF;
    #include "gr17.h"
    int sol = TSP::solveTSP(costMatrix, false);
    if(sol != best_solution){
        std::cout<<"************* Custo INCORRETO! *****************\n";
        std::cout<<"Custo correto:    "<<best_solution<<"\n";
        std::cout<<"Custo encontrado: "<<sol<<"\n";
        return -1;
    } else {
        std::cout<<"Custo correto!\n";
    }
    return 0;
}
