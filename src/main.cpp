#include "tspSolver.hpp"

int main(int argc, char *argv[]){
    using TSP::INF;
    TSP::Matrix<4> cost = { INF, 10, 15, 20,
                            10, INF, 35, 25,
                            15, 35, INF, 30,
                            20, 25, 30, INF };
    int sol = TSP::solveTSP(cost);
    std::cout<<"Total cost: "<<sol<<"\n";
    return 0;
}
