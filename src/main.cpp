#include "BBSolver.hpp"
#include <cstdlib>
#ifdef _OPENMP
#include <omp.h>
#endif
#include <tuple>
#include <string>
#include <chrono>

using namespace std;
using clk = chrono::steady_clock;

#ifndef _OPENMP
#define omp_get_thread_num() 0
#define omp_get_num_threads() 1
#endif

// static int verbose = 0;
using namespace TSP;

auto solveTSP(const TSP::Matrix& costMatrix, clk::duration& work, TSP::City start = 0) {
    long t_work=0;
    auto _start = clk::now();
    size_t N = costMatrix.size();
    vector<long> solution(N+2, Matrix::INF);
    work+=clk::now()-_start;
    #pragma omp parallel reduction(max:t_work)
    {
        auto t_start = clk::now();
        std::cout<<"Iniciando TSP com matriz "<<N<<"x"<<N<<".\n";
        Node* root = new Node(costMatrix, start);
        auto children = root->children();
        MinHeap pq;
        int id = omp_get_thread_num();
        std::stringstream tmp;
        tmp<<"[Proc "<<id<<"] Processando cidades:";
        for(size_t i=id; i<children.size(); i+=omp_get_num_threads()) {
            tmp<<" "<<children[i]->cities.back();
            pq.push(children[i]); // Each thread gets a child of root and runs its subtree
        }
        tmp<<"\n";
        clk::duration _work = clk::now()-t_start;
        while (!pq.empty()) {
            t_start = clk::now();
            auto min = unique_ptr<Node>(pq.top());
            pq.pop();
            _work +=(clk::now()-t_start);
            if(min->is_feasible(solution[0])) {
                if (min->cities.size() == N+1) {
                    #pragma omp critical (crit_sol)
                    {
                        t_start = clk::now();
                        solution[0] = min->cost;
                        std::copy(min->cities.begin(), min->cities.end(), &solution[1]);
                        _work+=clk::now()-t_start;
                    }
                } else {
                    t_start = clk::now();
                    for(auto c: min->children()) pq.push(c);
                    _work+=(clk::now()-t_start);
                }
            }
        }
        t_work = _work.count();
        tmp<<"[Proc "<<omp_get_thread_num()<<"] Terminou com solução: "<< solution[0];
        tmp<<" após " <<_work/1.s<<"s\n";
        cout<<tmp.str()<<flush;
    }
    work += clk::duration(t_work);
    return solution;
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
    TSP::Matrix costMatrix(0);
    TSP::Cost bestSolution;
    std::tie(costMatrix,bestSolution) = TSP::readMatrix(data_file);
    std::cout<<"Arquivo: "<<data_file<<"\n";
    std::cout<<"Custo correto:    "<<bestSolution<<"\n"<<std::flush;
    auto start = clk::now();
    clk::duration t_work{};
    auto result = solveTSP(costMatrix, t_work);
    auto work = clk::now() - start;
    printPath(result.begin()+1, result.end());
    cout<<"\n";
    if(result[0] != bestSolution){
        std::cout<<"************* Custo INCORRETO! *****************\n";
        std::cout<<"Custo correto:    "<<bestSolution<<"\n";
        std::cout<<"Custo encontrado: "<<result[0]<<"\n";
        return -1;
    } else {
        std::cout<<"Custo correto encontrado!\n";
        std::cout<<"  Tempo total: "<< work/1.s<<"s\n";
        std::cout<<"   Tempo util: "<< t_work/1.s<<"s\n";
        std::cout<<"Overhead em s: "<< (work-t_work)/1.s<<"s\n";
        std::cout<<"   Overhead %: "<<((work-t_work)/1.ms)/(work/1.ms)*100.0<<"\n\n";

    }
    return 0;
}
