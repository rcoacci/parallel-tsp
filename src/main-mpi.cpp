#include "BBSolver.hpp"
#include <cstdlib>
#include <mpi.h>
#include <tuple>
#include <string>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <unordered_set>

using namespace std;
using namespace TSP;
using clk = chrono::steady_clock;

enum {
    COST_TAG = 10,
    SOLUTION_TAG,
    DONE_TAG
};

auto solveTSP(Matrix input, int id, int numProcs, clk::duration& work) {
    auto t_start = clk::now();
    MinHeap pq;
    size_t N = input.size();
    vector<long> solution(N+2, Matrix::INF);
    Node* root = new Node(input);
    std::stringstream tmp;
    tmp<<"[Proc "<<id<<"] Processando cidades:";
    auto firstLevel = root->children();
    for(size_t i=id-1; i < firstLevel.size(); i+=numProcs-1){
        tmp<<" "<<firstLevel[i]->cities.back();
        pq.push(firstLevel[i]);
    }
    tmp<<"\n";
    MPI_Status probe;
    int probe_flag = 0;
    work+=clk::now()-t_start;
    while (!pq.empty()) {
        MPI_Iprobe(0, COST_TAG, MPI_COMM_WORLD, &probe_flag, &probe);
        if(probe_flag) MPI_Recv(solution.data(), solution.size(), MPI_LONG , 0, COST_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        t_start = clk::now();
        auto min = unique_ptr<Node>(pq.top());
        pq.pop();
        work+=clk::now()-t_start;
        if(min->is_feasible(solution[0])) {
            if (min->cities.size() == N+1) {
                t_start = clk::now();
                solution[0] = min->cost;
                std::copy(min->cities.begin(), min->cities.end(), &solution[1]);
                work+=clk::now()-t_start;
                MPI_Send(solution.data(), solution.size(), MPI_LONG, 0, SOLUTION_TAG, MPI_COMM_WORLD);
            } else {
                t_start = clk::now();
                for (auto c: min->children()) pq.push(c);
                work+=clk::now()-t_start;
            }
        }
    }
    cout<<tmp.str()<<flush;
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
    TSP::Matrix input(0);
    TSP::Cost correctSolution;
    vector<long> bestSolution(input.size()+2, Matrix::INF);
    std::tie(input,correctSolution) = readMatrix(data_file);
    int numProcs = 0;
    int id = -1;
    MPI_Init(&argc, &argv);
    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_ARE_FATAL);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    int error = 0;
    int is_active = (size_t)id < input.size();
    vector<int> active(numProcs, 0);
    MPI_Gather(&is_active, 1, MPI_INT, &active[0], 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(id==0){
        auto root_start = clk::now();
        std::cout<<"Arquivo: "<<data_file<<"\n";
        std::cout<<"[Proc "<<id<<"] Iniciando TSP com matriz "<<input.size()<<"x"<<input.size()<<".\n";
        std::cout<<"[Proc "<<id<<"] Custo correto:    "<<correctSolution<<"\n";
        cout<<flush;
        MPI_Status probe, msg;
        unordered_set<int> running;
        for(int i=1; i<numProcs; i++) if(active[i]) running.insert(i);
        vector<long> localBest(input.size()+2, Matrix::INF);
        vector<MPI_Request> requests;
        long max_work_count=0;
        requests.reserve(numProcs-1);
        while(!running.empty()){
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &probe);
            if(probe.MPI_SOURCE == 0) {
                cerr<<"Erro: Root recebeu mensagem dele mesmo!\n";
                MPI_Abort(MPI_COMM_WORLD, -1);
            }
            if(probe.MPI_TAG == DONE_TAG){
                long work;
                MPI_Recv(&work, 1, MPI_LONG, probe.MPI_SOURCE, probe.MPI_TAG, MPI_COMM_WORLD, &msg);
                if(work>max_work_count) max_work_count = work;
                running.erase(msg.MPI_SOURCE);
            }
            if(probe.MPI_TAG == SOLUTION_TAG) {
                MPI_Recv(localBest.data(), localBest.size(), MPI_LONG, probe.MPI_SOURCE, probe.MPI_TAG, MPI_COMM_WORLD, &msg);
                MPI_Waitall(requests.size(), requests.data(), MPI_STATUSES_IGNORE);
                requests.clear();
                if(localBest[0]<bestSolution[0]) {
                    bestSolution = localBest;
                    for (int wid: running){
                        MPI_Request r;
                        MPI_Isend(bestSolution.data(), bestSolution.size(), MPI_LONG, wid, COST_TAG, MPI_COMM_WORLD, &r);
                        requests.push_back(r);
                    }
                }
            }
        }
        if(bestSolution[0] != correctSolution){
            std::cout<<"************* Custo INCORRETO! *****************\n";
            std::cout<<"Custo encontrado: "<<bestSolution[0]<<"\n";
            error = -1;
        } else {
            auto root_total = clk::now() - root_start;
            auto overhead = root_total - clk::duration(max_work_count);
            std::cout<<"Custo correto!\nMelhor tour: ";
            printPath(bestSolution.begin()+1, bestSolution.end());
            cout<<"\n";
            std::cout<<"[Proc "<<id<<"] Tempo principal: "<<root_total/1.s<<"s\n";
            std::cout<<"[Proc "<<id<<"]      Tempo util: "<<clk::duration(max_work_count)/1.s<<"s\n";
            std::cout<<"[Proc "<<id<<"]   Overhead em s: "<<overhead/1.s<<"s\n";
            std::cout<<"[Proc "<<id<<"]      Overhead %: "<<(overhead/1.ms)/(root_total/1.ms)*100.0<<"%\n\n";
        }
    } else if((size_t)id < input.size()) {
        clk::duration overhead{}, work{};
        auto p_start = clk::now();
        bestSolution = solveTSP(input, id, numProcs, work);
        auto work_count = work.count();
        MPI_Send(&work_count, 1, MPI_LONG, 0, DONE_TAG, MPI_COMM_WORLD);
        auto p_time = clk::now() - p_start;
        overhead = p_time-work;
        std::cout<<"[Proc "<<id<<"] Terminou com solução: "<<bestSolution[0]<<"\n";
        std::cout<<"[Proc "<<id<<"]      Tempo principal: "<<p_time/1.s<<"s\n";
        std::cout<<"[Proc "<<id<<"]        Overhead em s: "<<overhead/1.s<<"s\n";
        std::cout<<"[Proc "<<id<<"]           Overhead %: "<<(overhead/1.ms)/(p_time/1.ms)*100.0<<"%\n\n";
    } else {
        cout<<"[Proc "<<id<<"] Sem trabalho.\n";
    }
    cout<<flush;
    MPI_Finalize();
    return error;
}
