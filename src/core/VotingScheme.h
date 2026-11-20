#pragma once

/**
 * @file VotingScheme.h
 * Voting scheme (paper, Sect. 2.1): segment rasterization, accumulation
 * A(v) and confidence C(v).
 */

#include <vector>

#include <DGtal/images/ImageContainerBySTLVector.h>

#include "Types.h"

namespace decsirp {

/// A voxel v of a rasterized segment S_i.
struct SegmentVoxel
{
  Point voxel; ///< the voxel v
  double t;    ///< parameter t at which s_i enters v, i.e. distance from x_i
};

/**
 * Discretization S_i of a segment s_i = { x_i + t n_i | t in [0, ell] }:
 * the voxels intersected by s_i, sorted by increasing t.
 */
using Segment = std::vector<SegmentVoxel>;

/**
 * Computes the discretization S_i of the segment s_i of length @a ell,
 * starting at @a x and directed along @a n (assumed inward).
 *
 * This is a supercover: all voxels touched by the segment are kept. In
 * particular, when the segment goes exactly through a grid edge or vertex,
 * all voxels sharing that edge or vertex are added. Voxels outside
 * @a domain are discarded and the traversal stops when leaving the domain.
 */
Segment rasterizeSegment(const RealPoint& x, const RealVector& n, double ell,
                         const Domain& domain);

/**
 * Voting scheme of Sect. 2.1.
 *
 * From the pairs (x_i, n_i), it computes:
 *  - the segments S_i (one per pair);
 *  - the accumulation image A, where A(v) is the number of segments S_i
 *    such that v in S_i;
 *  - the confidence image C (Eq. 1), where C(v) is the fraction of the
 *    segments crossing v for which A(v) is maximal along the segment.
 */
class VotingScheme
{
public:
  using AccumulationImage = DGtal::ImageContainerBySTLVector<Domain, int>;
  using ConfidenceImage = DGtal::ImageContainerBySTLVector<Domain, double>;

  /**
   * Runs the voting scheme.
   * @param points surface points x_i.
   * @param normals inward unit normals n_i.
   * @param domain voxel grid.
   * @param ell segment length (scale of the detectable structures).
   */
  VotingScheme(const std::vector<RealPoint>& points,
               const std::vector<RealVector>& normals, const Domain& domain,
               double ell);

  /// Segments S_i, in the same order as the input points x_i.
  const std::vector<Segment>& segments() const { return mySegments; }
  /// Accumulation image A.
  const AccumulationImage& accumulation() const { return myAccumulation; }
  /// Confidence image C.
  const ConfidenceImage& confidence() const { return myConfidence; }

private:
  void computeAccumulation();
  void computeConfidence();

  Domain myDomain;
  std::vector<Segment> mySegments;
  AccumulationImage myAccumulation;
  ConfidenceImage myConfidence;
};

} // namespace decsirp
