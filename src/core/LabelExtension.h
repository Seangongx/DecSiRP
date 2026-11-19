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
 * Diffuses several signals by one implicit (backward Euler) step of time
 * t = 1/lambda: for each column s of S, solves (L + lambda I) x = lambda s.
 * The matrix L + lambda I is factorized only once.
 *
 * @return the matrix X whose columns are the diffused signals x.
 */
Eigen::MatrixXd diffuse(const Eigen::SparseMatrix<double>& L,
                        const Eigen::MatrixXd& S, double lambda);

/**
 * Extends a partial labeling to the whole graph.
 *
 * For each label l in {1, ..., numberOfLabels}, the indicator vector s(l)
 * (s(l)_i = 1 iff seeds[i] = l) is diffused into x(l), all labels sharing
 * the same factorization of L + lambda I. Then:
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
