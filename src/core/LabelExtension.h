#pragma once

/**
 * @file LabelExtension.h
 * Label extension on the dual graph of the mesh (paper, Sect. 2.3,
 * "Label extension").
 */

#include <vector>

#include <Eigen/Sparse>

#include "Types.h"

namespace decsirp {

/// Edge of the dual graph, between two facets sharing a mesh edge.
struct DualEdge
{
  std::size_t f0;
  std::size_t f1;
  double weight;
};

/**
 * Builds the edges of the dual graph: one node per facet, and one edge
 * between every pair of facets sharing a mesh edge (non-manifold edges
 * shared by k facets yield k(k-1)/2 dual edges). All weights are 1.
 */
std::vector<DualEdge> dualGraphEdges(const std::vector<Facet>& facets);

/// Graph Laplacian L = D - W of a graph with @a n nodes.
Eigen::SparseMatrix<double> graphLaplacian(std::size_t n,
                                           const std::vector<DualEdge>& edges);

/**
 * Diffuses the signal s by one implicit (backward Euler) step of time
 * t = 1/lambda, i.e. solves (L + lambda I) x = lambda s.
 */
Eigen::VectorXd diffuse(const Eigen::SparseMatrix<double>& L,
                        const Eigen::VectorXd& s, double lambda);

/**
 * Extends a partial labeling to the whole graph.
 *
 * For each label l in {1, ..., numberOfLabels}, the indicator vector s(l)
 * (s(l)_i = 1 iff seeds[i] = l) is diffused into x(l). Then:
 *  - a seeded node keeps its seed label;
 *  - any other node i receives argmax_l x(l)_i if max_l x(l)_i >= rho,
 *    and NoLabel otherwise.
 *
 * NB: the paper applies the argmax rule to every node, seeds included.
 *
 * @param L graph Laplacian.
 * @param seeds initial labels (NoLabel for unlabeled nodes).
 * @param numberOfLabels number of labels L.
 * @param lambda inverse of the diffusion time.
 * @param rho threshold on the diffused values.
 */
std::vector<int> extendLabels(const Eigen::SparseMatrix<double>& L,
                              const std::vector<int>& seeds, int numberOfLabels,
                              double lambda, double rho);

} // namespace decsirp
