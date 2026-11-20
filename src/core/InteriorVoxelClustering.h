#pragma once

/**
 * @file InteriorVoxelClustering.h
 * Clustering of interior voxels (paper, Sect. 2.2) and mapping of the
 * resulting labels back to the surface (paper, Sect. 2.3).
 */

#include <map>
#include <vector>

#include "InputSurface.h"
#include "Types.h"
#include "VotingScheme.h"

namespace decsirp {

/// Thresholds defining the candidate voxels (Sect. 2.2).
struct CandidateThresholds
{
  int a = 1;      ///< threshold on the accumulation: A(v) >= a
  double c = 0.5; ///< threshold on the confidence:   C(v) >= c
  double d = 1.0; ///< threshold on the reachable distance: D(v) > d
};

/**
 * Identifies the interior voxels and groups them into clusters.
 *
 * 1. Candidate voxels are the voxels v such that A(v) >= a, C(v) >= c and
 *    D(v) > d, where the reachable distance D(v) is the distance from v to
 *    the closest point x_i such that v in S_i.
 * 2. A candidate voxel v is interior iff it is not a vertex of the convex
 *    hull H_v of v and of the facets f_i such that v in S_i (Fig. 3).
 * 3. Two interior voxels v, w are adjacent iff |v - w| < D(v) + D(w)
 *    (Fig. 4). Clusters are the connected components of this graph; they
 *    are labeled from 1 to numberOfClusters().
 */
class InteriorVoxelClustering
{
public:
  /// Information attached to a voxel v such that A(v) >= a and C(v) >= c.
  struct VoxelInfo
  {
    Point voxel;
    std::vector<std::size_t> segments; ///< indices i such that v in S_i
    double reachableDistance = 0.0;    ///< D(v)
    bool interior = false;             ///< true iff v is an interior voxel
    int label = NoLabel;               ///< cluster label (interior voxels only)
  };

  InteriorVoxelClustering(const InputSurface& surface,
                          const VotingScheme& voting,
                          const CandidateThresholds& thresholds);

  /// All voxels v such that A(v) >= a and C(v) >= c.
  const std::vector<VoxelInfo>& voxels() const { return myVoxels; }

  /// Number of clusters L.
  int numberOfClusters() const { return myNumberOfClusters; }

  /**
   * Maps the cluster labels back to the surface points (Sect. 2.3,
   * "Mapping back to the surface"): x_i receives the label of the closest
   * interior voxel v to x_i such that v in S_i, i.e. the first interior
   * voxel met along the segment.
   *
   * @return one label per surface point x_i (NoLabel if S_i contains no
   * interior voxel).
   */
  std::vector<int> labelsOnSurface() const;

private:
  void collectVoxels(const CandidateThresholds& thresholds);
  void computeReachableDistances();
  bool isVertexOfConvexHull(const VoxelInfo& info) const;
  void classifyInteriorVoxels(double d);
  void clusterInteriorVoxels();

  const InputSurface& mySurface;
  const VotingScheme& myVoting;

  std::vector<VoxelInfo> myVoxels;
  std::map<Point, std::size_t> myVoxelIndex; ///< voxel -> index in myVoxels
  int myNumberOfClusters = 0;
};

} // namespace decsirp
