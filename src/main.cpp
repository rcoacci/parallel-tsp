#include "BBSolver.hpp"
#include <cstdlib>
#include <omp.h>
#include <tuple>
#include <string>
#include <chrono>

using namespace std;
using clk = chrono::steady_clock;

#ifndef _OPENMP
#define omp_get_thread_num() 0
#define omp_get_num_threads() 1
#endif

static int verbose = 0;
using namespace TSP;

auto solveTSP(const TSP::Matrix& costMatrix, TSP::City start = 0) {
    size_t N = costMatrix.size();
    std::cout<<"Iniciando TSP com matriz "<<N<<"x"<<N<<".\n";
    Node* root = new Node(costMatrix, start);
    std::unique_ptr<Node> solution;
    auto children = root->children();
    #pragma omp parallel
    {
        auto t_start = clk::now();
        std::stringstream tmp;
        tmp<<"[Proc "<<omp_get_thread_num()<<"] Processando cidades:";
        MinHeap pq;
        int id = omp_get_thread_num();
        for(size_t i=id; i<children.size(); i+=omp_get_num_threads()) {
            tmp<<" "<<children[i]->cities.back();
            pq.push(children[i]); // Each thread gets a child of root and runs its subtree
        }
        #pragma omp critical (print)
        cout<<tmp.str()<<endl;
        while (!pq.empty()) {
            auto min = unique_ptr<Node>(pq.top());
            pq.pop();
            if(solution == nullptr || min->is_feasible(solution->cost)) {
                if (min->cities.size() == costMatrix.size()+1) {
                    #pragma omp critical (crit_sol)
                    solution = std::move(min);
                    if(verbose) solution->printFoundSolution(omp_get_thread_num());
                } else {
                    for(auto c: min->children()){
                        pq.push(c);
                    }
                }
            }
        }
        tmp.str("");
        tmp<<"[Proc "<<omp_get_thread_num()<<"] Terminou após: "
            <<(clk::now()-t_start)/1.s<<"\n";
        cout<<tmp.str()<<flush;
    }
    return *solution;
}

int main(int argc, char *argv[]){
    std::ios_base::sync_with_stdio(false);
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
    clk::time_point full_start = clk::now();
    TSP::Matrix costMatrix(0);
    TSP::Cost bestSolution;
    std::tie(costMatrix,bestSolution) = TSP::readMatrix(data_file);
    std::cout<<"Arquivo: "<<data_file<<"\n";
    std::cout<<"Custo correto:    "<<bestSolution<<"\n"<<std::flush;
    auto start = clk::now();
    auto result = solveTSP(costMatrix);
    auto work = clk::now() - start;
    result.printPath();
    cout<<"\n";
    if(result.cost != bestSolution){
        std::cout<<"************* Custo INCORRETO! *****************\n";
        std::cout<<"Custo correto:    "<<bestSolution<<"\n";
        std::cout<<"Custo encontrado: "<<result.cost<<"\n";
        return -1;
    } else {
        std::cout<<"Custo correto encontrado!\n";
        std::cout<<"Tempo de processamento: "<< work/1.s<<"\n";
    }
    auto total_time = clk::now()-full_start;
    cout<<"Tempo total: "<<total_time/1.s<<"\n";
    return 0;
}
