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

static int verbose = 0;

auto solveTSP(Matrix input, int id, int numProcs, clk::duration& overhead) {
    Stack pq;
    size_t N = input.size();
    vector<long> solution(N+2, INF);
    Node* root = new Node(input);
    std::cout<<"[Proc "<<id<<"] Processando cidades:";
    auto firstLevel = root->children();
    for(size_t i=id-1; i < firstLevel.size(); i+=numProcs-1){
        std::cout<<" "<<firstLevel[i]->cities.back();
        pq.push(firstLevel[i]);
    }
    std::cout<<"\n"<<flush;
    MPI_Status probe;
    int probe_flag = 0;
    clk::time_point t_start;
    size_t max_stack_size = 0;
    while (!pq.empty()) {
        max_stack_size = std::max(max_stack_size, pq.size());
        t_start = clk::now();
        MPI_Iprobe(0, COST_TAG, MPI_COMM_WORLD, &probe_flag, &probe);
        if(probe_flag) MPI_Recv(solution.data(), solution.size(), MPI_LONG , 0, COST_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        overhead += (clk::now()-t_start);
        auto min = unique_ptr<Node>(pq.top());
        pq.pop();
        if(min->is_feasible(solution[0])) {
            if (min->cities.size() == N+1) {
                solution[0] = min->cost;
                std::copy(min->cities.begin(), min->cities.end(), &solution[1]);
                t_start = clk::now();
                MPI_Send(solution.data(), solution.size(), MPI_LONG, 0, SOLUTION_TAG, MPI_COMM_WORLD);
                overhead += (clk::now()-t_start);
                if (verbose) min->printFoundSolution(id);
            } else {
                for (auto c: min->children()) pq.push(c);
            }
        }
    }
    cout<<"Max stack: "<<max_stack_size<<endl;
    return solution;
}

int main(int argc, char *argv[]){
    if(argc<2) {
        std::cerr<<"Falta arquivo de dados!";
        exit(-1);
    }
    std::cout.sync_with_stdio(false);
    clk::time_point full_start = clk::now();
    auto data_file = filesystem::path{argv[1]};
    TSP::Matrix input(0);
    TSP::Cost correctSolution;
    vector<long> bestSolution(input.size()+2, INF);
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
        std::cout<<"Arquivo: "<<data_file<<"\n";
        std::cout<<"[Proc "<<id<<"] Iniciando TSP com matriz "<<input.size()<<"x"<<input.size()<<".\n";
        std::cout<<"[Proc "<<id<<"] Custo correto:    "<<correctSolution<<"\n";
        cout<<flush;
        MPI_Status probe, msg;
        unordered_set<int> running;
        for(int i=1; i<numProcs; i++) if(active[i]) running.insert(i);
        vector<long> localBest(input.size()+2, INF);
        vector<MPI_Request> requests;
        requests.reserve(numProcs-1);
        while(!running.empty()){
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &probe);
            if(probe.MPI_SOURCE == 0) {
                cerr<<"Erro: Root recebeu mensagem dele mesmo!\n";
                MPI_Abort(MPI_COMM_WORLD, -1);
            }
            MPI_Recv(localBest.data(), localBest.size(), MPI_LONG, probe.MPI_SOURCE, probe.MPI_TAG, MPI_COMM_WORLD, &msg);
            MPI_Waitall(requests.size(), requests.data(), MPI_STATUSES_IGNORE);
            if(localBest[0]<bestSolution[0]) bestSolution = localBest;
            if(msg.MPI_TAG == DONE_TAG) running.erase(msg.MPI_SOURCE);
            if(msg.MPI_TAG == SOLUTION_TAG) {
                requests.clear();
                for (int wid: running){
                    MPI_Request r;
                    MPI_Isend(bestSolution.data(), bestSolution.size(), MPI_LONG, wid, COST_TAG, MPI_COMM_WORLD, &r);
                    requests.push_back(r);
                }
            }
        }
        if(bestSolution[0] != correctSolution){
            std::cout<<"************* Custo INCORRETO! *****************\n";
            std::cout<<"Custo encontrado: "<<bestSolution[0]<<"\n";
            error = -1;
        } else {
            std::cout<<"Custo correto!\nMelhor tour: ";
            printPath(bestSolution.begin()+1, bestSolution.end());
            cout<<"\n";
        }
    } else if((size_t)id < input.size()) {
        clk::duration overhead{};
        auto p_start = clk::now();
        bestSolution = solveTSP(input, id, numProcs, overhead);
        std::cout<<"[Proc "<<id<<"] Terminou com solução: "<<bestSolution[0]<<"\n";
        auto t_start = clk::now();
        MPI_Send(bestSolution.data(), bestSolution.size(), MPI_LONG, 0, DONE_TAG, MPI_COMM_WORLD);
        overhead += (clk::now()-t_start);
        auto p_time = clk::now()-p_start;
        std::cout<<"[Proc "<<id<<"] Tempo principal: "<<p_time/1.s<<"\n";
        std::cout<<"[Proc "<<id<<"]  Overhead total: "<<overhead/1.s<<"\n";
        std::cout<<"[Proc "<<id<<"]      Overhead %: "<<(overhead/1.ms)/(p_time/1.ms)*100.0<<"\n\n";
    } else {
        cout<<"[Proc "<<id<<"] Sem trabalho.\n";
    }
    cout<<flush;
    MPI_Finalize();
    auto total_time = clk::now()-full_start;
    if(id==0) std::cout<<"[Proc "<<id<<"]    Tempo total: "<<total_time/1.s<<"\n";
    return error;
}
