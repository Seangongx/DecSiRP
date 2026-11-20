#include "InteriorVoxelClustering.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <queue>
#include <set>

#include <DGtal/geometry/tools/QuickHull.h>

namespace decsirp {

//------------------------------------------------------------------------------
InteriorVoxelClustering::InteriorVoxelClustering(const InputSurface& surface,
                                                 const VotingScheme& voting,
                                                 const CandidateThresholds& thresholds)
  : mySurface(surface), myVoting(voting)
{
  collectVoxels(thresholds);
  computeReachableDistances();
  classifyInteriorVoxels(thresholds.d);
  clusterInteriorVoxels();
}

//------------------------------------------------------------------------------
void InteriorVoxelClustering::collectVoxels(const CandidateThresholds& thresholds)
{
  // Builds the voxel <-> segment incidences, restricted to the voxels v such
  // that A(v) >= a and C(v) >= c.
  const auto& A = myVoting.accumulation();
  const auto& C = myVoting.confidence();
  const auto& segments = myVoting.segments();

  for (std::size_t i = 0; i < segments.size(); ++i) {
    std::set<Point> voxelsOfSi;
    for (const SegmentVoxel& sv : segments[i])
      if (A(sv.voxel) >= thresholds.a && C(sv.voxel) >= thresholds.c)
        voxelsOfSi.insert(sv.voxel);

    for (const Point& v : voxelsOfSi) {
      auto [it, inserted] = myVoxelIndex.emplace(v, myVoxels.size());
      if (inserted)
        myVoxels.push_back(VoxelInfo{v, {}, 0.0, false, NoLabel});
      myVoxels[it->second].segments.push_back(i);
    }
  }
}

//------------------------------------------------------------------------------
void InteriorVoxelClustering::computeReachableDistances()
{
  // D(v) = min { |v - x_i| : v in S_i }.
  for (VoxelInfo& info : myVoxels) {
    const RealPoint v(info.voxel);
    double minSquaredDistance = std::numeric_limits<double>::max();
    for (std::size_t i : info.segments)
      minSquaredDistance = std::min(minSquaredDistance, (v - mySurface.points[i]).squaredNorm());
    info.reachableDistance = std::sqrt(minSquaredDistance);
  }
}

//------------------------------------------------------------------------------
bool InteriorVoxelClustering::isVertexOfConvexHull(const VoxelInfo& info) const
{
  // With a single facet, the facets cannot surround v.
  if (info.segments.size() < 2)
    return true;

  // Input points: v first, then the vertices of the facets f_i, v in S_i.
  std::vector<RealPoint> points;
  points.push_back(RealPoint(info.voxel));
  for (std::size_t i : info.segments)
    for (std::size_t k : mySurface.facets[i])
      points.push_back(mySurface.vertices[k]);

  // Coordinates are rounded to 1/precision by the kernel.
  using Kernel = DGtal::ConvexHullRationalKernel<3>;
  using QuickHull = DGtal::QuickHull<Kernel>;
  const std::uint64_t precision = 1000;
  QuickHull hull{Kernel(precision)};
  hull.setInput(points);
  hull.computeConvexHull();

  QuickHull::IndexRange pointToVertex;
  hull.getPoint2Vertex(pointToVertex);
  if (pointToVertex.empty())
    return true;
  // input2comp maps input indices to processed indices (duplicates removed).
  return pointToVertex[hull.input2comp[0]] != QuickHull::UNASSIGNED;
}

//------------------------------------------------------------------------------
void InteriorVoxelClustering::classifyInteriorVoxels(double d)
{
  // A voxel is interior iff it is a candidate (A and C thresholds were
  // already applied in collectVoxels) and not a vertex of H_v.
  std::size_t count = 0;
  for (VoxelInfo& info : myVoxels) {
    info.interior = info.reachableDistance > d && !isVertexOfConvexHull(info);
    if (info.interior)
      ++count;
  }
  DGtal::trace.info() << "Number of interior voxels = " << count << std::endl;
}

//------------------------------------------------------------------------------
void InteriorVoxelClustering::clusterInteriorVoxels()
{
  // Breadth-first traversal of the graph whose nodes are the interior voxels
  // and whose edges link v and w iff |v - w| < D(v) + D(w).
  // Naive O(n^2) neighbor search: the number of interior voxels is small.
  auto adjacent = [](const VoxelInfo& v, const VoxelInfo& w) {
    const double radiusSum = v.reachableDistance + w.reachableDistance;
    return (RealPoint(v.voxel) - RealPoint(w.voxel)).squaredNorm() < radiusSum * radiusSum;
  };

  const std::size_t n = myVoxels.size();
  std::vector<bool> visited(n, false);
  auto toVisit = [&](std::size_t k) { return myVoxels[k].interior && !visited[k]; };

  myNumberOfClusters = 0;
  for (std::size_t start = 0; start < n; ++start) {
    if (!toVisit(start))
      continue;

    const int label = ++myNumberOfClusters;
    std::queue<std::size_t> queue;
    queue.push(start);
    visited[start] = true;

    while (!queue.empty()) {
      const std::size_t current = queue.front();
      queue.pop();
      myVoxels[current].label = label;

      for (std::size_t k = 0; k < n; ++k) {
        if (toVisit(k) && adjacent(myVoxels[current], myVoxels[k])) {
          visited[k] = true;
          queue.push(k);
        }
      }
    }
  }
  DGtal::trace.info() << "Number of clusters = " << myNumberOfClusters << std::endl;
}

//------------------------------------------------------------------------------
std::vector<int> InteriorVoxelClustering::labelsOnSurface() const
{
  const auto& segments = myVoting.segments();
  std::vector<int> labels(segments.size(), NoLabel);

  for (std::size_t i = 0; i < segments.size(); ++i) {
    // Voxels of S_i are sorted by increasing distance t to x_i: the first
    // interior voxel met is the closest one.
    for (const SegmentVoxel& sv : segments[i]) {
      auto it = myVoxelIndex.find(sv.voxel);
      if (it != myVoxelIndex.end() && myVoxels[it->second].interior) {
        labels[i] = myVoxels[it->second].label;
        break;
      }
    }
  }
  return labels;
}

} // namespace decsirp
