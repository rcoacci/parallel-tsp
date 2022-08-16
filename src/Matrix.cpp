#include "Matrix.hpp"

#include <fstream>
#include <algorithm>

using namespace TSP;

Matrix::Matrix(size_t n) : m(n * n), N(n) {}

Cost &Matrix::operator()(City i, City j) { return m[i * N + j]; }

Cost Matrix::operator()(City i, City j) const { return m[i * N + j]; }

size_t Matrix::size() const { return N; }

Cost Matrix::reduce() { return reduceRow() + reduceCol(); }

Cost Matrix::reduceRow() {
  Cost rowCost = 0;
  for (size_t i = 0; i < N; i++) {
    Cost min_val = *std::min_element(&(*this)(i, 0), &(*this)(i, N));
    if (min_val != INF && min_val != 0) {
      rowCost += min_val;
      for (size_t j = 0; j < N; j++) {
        if ((*this)(i, j) != INF)
          (*this)(i, j) -= min_val;
      }
    }
  }
  return rowCost;
}

Cost Matrix::reduceCol() {
  Cost colCost = 0;
  for (size_t j = 0; j < N; j++) {
    Cost min_val = INF;
    for (size_t i = 0; i < N; i++) {
      min_val = std::min(min_val, (*this)(i, j));
    }
    if (min_val != INF && min_val != 0) {
      colCost += min_val;
      for (size_t i = 0; i < N; i++) {
        if ((*this)(i, j) != INF)
          (*this)(i, j) -= min_val;
      }
    }
  }
  return colCost;
}

std::tuple<Matrix, Cost>
TSP::readMatrix(const std::filesystem::path &filename) {
  auto ifs = std::ifstream(filename);
  size_t sz = 0;
  ifs >> sz;
  Matrix matrix(sz);
  for (size_t i = 0; i < sz; i++) {
    for (size_t j = 0; j < sz; j++) {
      ifs >> matrix(i, j);
      if (matrix(i, j) == (Cost)-1)
        matrix(i, j) = Matrix::INF;
    }
  }
  Cost minCost = 0;
  ifs >> minCost;
  return std::make_tuple(matrix, minCost);
}
