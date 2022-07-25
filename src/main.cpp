#include "BBSolver.hpp"
#include <cstdlib>
#include <omp.h>
#include <tuple>
#include <string>

using namespace std;

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
    TSP::Node result = TSP::solveTSP(costMatrix);
    TSP::printPath(result.path);
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
