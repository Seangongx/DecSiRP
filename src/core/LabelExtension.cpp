#include "LabelExtension.h"

#include <cmath>
#include <functional>
#include <stdexcept>
#include <unordered_map>

#include <Eigen/SparseCholesky>

namespace decsirp {

namespace {

/// Undirected mesh edge {a, b}, stored with a <= b.
struct EdgeKey
{
  std::size_t a, b;
  EdgeKey(std::size_t i, std::size_t j) : a(std::min(i, j)), b(std::max(i, j)) {}
  bool operator==(const EdgeKey& other) const { return a == other.a && b == other.b; }
};

struct EdgeKeyHash
{
  std::size_t operator()(const EdgeKey& e) const
  {
    return std::hash<std::size_t>{}(e.a) ^ (std::hash<std::size_t>{}(e.b) << 1);
  }
};

} // namespace

//------------------------------------------------------------------------------
std::vector<DualEdge> dualGraphEdges(const std::vector<Facet>& facets)
{
  // Mesh edge -> incident facets.
  std::unordered_map<EdgeKey, std::vector<std::size_t>, EdgeKeyHash> edgeToFacets;
  for (std::size_t f = 0; f < facets.size(); ++f) {
    const Facet& facet = facets[f];
    const std::size_t n = facet.size();
    if (n < 3)
      continue; // degenerate facet
    for (std::size_t k = 0; k < n; ++k) {
      const std::size_t a = facet[k];
      const std::size_t b = facet[(k + 1) % n];
      if (a != b) // skip degenerate edges
        edgeToFacets[EdgeKey(a, b)].push_back(f);
    }
  }

  // One dual edge per pair of facets sharing a mesh edge
  // (boundary edges, with a single incident facet, yield none).
  std::vector<DualEdge> dualEdges;
  for (const auto& [edge, incident] : edgeToFacets)
    for (std::size_t i = 0; i < incident.size(); ++i)
      for (std::size_t j = i + 1; j < incident.size(); ++j)
        dualEdges.push_back({incident[i], incident[j], 1.0});
  return dualEdges;
}

//------------------------------------------------------------------------------
Eigen::SparseMatrix<double> graphLaplacian(std::size_t n,
                                           const std::vector<DualEdge>& edges)
{
  std::vector<Eigen::Triplet<double>> triplets;
  triplets.reserve(4 * edges.size());
  for (const DualEdge& e : edges) {
    triplets.emplace_back(e.f0, e.f0, e.weight);
    triplets.emplace_back(e.f1, e.f1, e.weight);
    triplets.emplace_back(e.f0, e.f1, -e.weight);
    triplets.emplace_back(e.f1, e.f0, -e.weight);
  }
  Eigen::SparseMatrix<double> L(n, n);
  L.setFromTriplets(triplets.begin(), triplets.end()); // duplicates are summed
  return L;
}

//------------------------------------------------------------------------------
Eigen::VectorXd diffuse(const Eigen::SparseMatrix<double>& L,
                        const Eigen::VectorXd& s, double lambda)
{
  if (L.rows() != L.cols() || L.rows() != s.size())
    throw std::invalid_argument("diffuse: size mismatch.");
  if (!(lambda > 0.0) || !std::isfinite(lambda))
    throw std::invalid_argument("diffuse: lambda must be positive and finite.");

  // L + lambda I is symmetric positive definite.
  Eigen::SparseMatrix<double> M = L;
  for (Eigen::Index i = 0; i < M.rows(); ++i)
    M.coeffRef(i, i) += lambda;
  M.makeCompressed();

  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
  solver.compute(M);
  if (solver.info() != Eigen::Success)
    throw std::runtime_error("diffuse: failed to factorize L + lambda I.");

  Eigen::VectorXd x = solver.solve(lambda * s);
  if (solver.info() != Eigen::Success)
    throw std::runtime_error("diffuse: failed to solve (L + lambda I) x = lambda s.");
  return x;
}

//------------------------------------------------------------------------------
std::vector<int> extendLabels(const Eigen::SparseMatrix<double>& L,
                              const std::vector<int>& seeds, int numberOfLabels,
                              double lambda, double rho)
{
  const Eigen::Index n = static_cast<Eigen::Index>(seeds.size());
  if (numberOfLabels <= 0)
    return seeds;

  // Column l-1 contains x(l), the diffusion of the indicator vector s(l).
  Eigen::MatrixXd x(n, numberOfLabels);
  for (int l = 1; l <= numberOfLabels; ++l) {
    Eigen::VectorXd s(n);
    for (Eigen::Index i = 0; i < n; ++i)
      s[i] = (seeds[i] == l) ? 1.0 : 0.0;
    x.col(l - 1) = diffuse(L, s, lambda);
  }

  std::vector<int> labels(seeds.size(), NoLabel);
  for (Eigen::Index i = 0; i < n; ++i) {
    if (seeds[i] != NoLabel) {
      labels[i] = seeds[i];
      continue;
    }
    Eigen::Index argmax;
    const double max = x.row(i).maxCoeff(&argmax);
    if (max >= rho)
      labels[i] = static_cast<int>(argmax) + 1;
  }
  return labels;
}

} // namespace decsirp
