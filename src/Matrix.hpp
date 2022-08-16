#include <filesystem>
#include <vector>

namespace TSP {

using Cost = std::uint16_t;
using City = std::size_t;

class Matrix {
  std::vector<Cost> m;
  std::size_t N;
  Cost reduceRow();
  Cost reduceCol();

public:
  Matrix(std::size_t n);
  Cost &operator()(City i, City j);
  Cost operator()(City i, City j) const;
  size_t size() const;
  Cost reduce();
  static constexpr auto INF = std::numeric_limits<Cost>::max();
};

std::tuple<Matrix, Cost> readMatrix(const std::filesystem::path &filename);
} // namespace TSP
