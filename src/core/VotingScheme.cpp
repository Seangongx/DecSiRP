#include "VotingScheme.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace decsirp {

//------------------------------------------------------------------------------
Segment rasterizeSegment(const RealPoint& x, const RealVector& n, double ell,
                         const Domain& domain)
{
  constexpr double eps = 1e-10;
  constexpr double inf = std::numeric_limits<double>::infinity();

  Segment segment;
  if (n.norm() < eps)
    return segment;
  const RealVector direction = n.getNormalized();

  // Voxel v is the unit cube [v - 1/2, v + 1/2)^3 centered at the integer
  // point v. In the shifted coordinates y = x + (1/2, 1/2, 1/2), it is the
  // cube [v, v + 1)^3, so that the voxel containing x is floor(y).
  const RealPoint y = x + RealPoint::diagonal(0.5);

  // Voxel containing x, entered at t = 0.
  Point current(static_cast<int>(std::floor(y[0])),
                static_cast<int>(std::floor(y[1])),
                static_cast<int>(std::floor(y[2])));
  if (!domain.isInside(current))
    return segment;
  segment.push_back({current, 0.0});

  // Classical 3D DDA (Amanatides & Woo): for each axis k,
  //  - tMax[k] is the value of t at which s_i crosses the next grid plane
  //    orthogonal to axis k;
  //  - tDelta[k] is the increment of t between two such planes;
  //  - step[k] is the direction (+1/-1) of the move along axis k.
  std::array<double, 3> tMax, tDelta;
  std::array<int, 3> step;
  for (int k = 0; k < 3; ++k) {
    if (std::fabs(direction[k]) < eps) {
      tMax[k] = tDelta[k] = inf;
      step[k] = 0;
      continue;
    }
    tDelta[k] = 1.0 / std::fabs(direction[k]);
    const double frac = y[k] - std::floor(y[k]);
    if (direction[k] > 0.0) {
      tMax[k] = (1.0 - frac) * tDelta[k];
      step[k] = +1;
    } else {
      tMax[k] = frac * tDelta[k];
      step[k] = -1;
    }
  }

  while (true) {
    const double t = std::min({tMax[0], tMax[1], tMax[2]});
    if (!std::isfinite(t) || t > ell + eps)
      break;

    // Axes whose grid planes are crossed simultaneously at t.
    std::array<int, 3> crossed{};
    int numCrossed = 0;
    for (int k = 0; k < 3; ++k)
      if (std::fabs(tMax[k] - t) < eps)
        crossed[numCrossed++] = k;
    if (numCrossed == 0)
      break;

    // Supercover: if k planes are crossed at once (the segment goes through
    // a grid edge or vertex), the 2^k - 1 voxels obtained by moving along
    // any non-empty subset of the crossed axes are all added.
    for (int mask = 1; mask < (1 << numCrossed); ++mask) {
      Point v = current;
      for (int j = 0; j < numCrossed; ++j)
        if (mask & (1 << j))
          v[crossed[j]] += step[crossed[j]];
      if (domain.isInside(v))
        segment.push_back({v, t});
    }

    // Move along all crossed axes.
    for (int j = 0; j < numCrossed; ++j) {
      const int k = crossed[j];
      current[k] += step[k];
      tMax[k] += tDelta[k];
    }
    if (!domain.isInside(current))
      break;
  }

  return segment;
}

//------------------------------------------------------------------------------
VotingScheme::VotingScheme(const std::vector<RealPoint>& points,
                           const std::vector<RealVector>& normals,
                           const Domain& domain, double ell)
  : myDomain(domain), myAccumulation(domain), myConfidence(domain)
{
  if (points.size() != normals.size())
    throw std::invalid_argument("VotingScheme: points and normals must have the same size.");

  // The segments are rasterized once and stored, because they are used
  // again for the confidence, the clustering and the mapping back.
  mySegments.reserve(points.size());
  for (std::size_t i = 0; i < points.size(); ++i)
    mySegments.push_back(rasterizeSegment(points[i], normals[i], ell, myDomain));

  computeAccumulation();
  computeConfidence();
}

//------------------------------------------------------------------------------
void VotingScheme::computeAccumulation()
{
  // A(v) = number of segments S_i such that v in S_i.
  for (const Point& v : myDomain)
    myAccumulation.setValue(v, 0);
  for (const Segment& segment : mySegments)
    for (const SegmentVoxel& sv : segment)
      myAccumulation.setValue(sv.voxel, myAccumulation(sv.voxel) + 1);
}

//------------------------------------------------------------------------------
void VotingScheme::computeConfidence()
{
  //        #{ S_i | v in S_i and A(v) = max_{w in S_i} A(w) }
  // C(v) = --------------------------------------------------      (Eq. 1)
  //                              A(v)
  //
  // Each segment votes for all the voxels where the accumulation reaches its
  // maximum along that segment. C(v) is set to 0 where A(v) = 0.
  for (const Point& v : myDomain)
    myConfidence.setValue(v, 0.0);

  for (const Segment& segment : mySegments) {
    if (segment.empty())
      continue;
    int maxA = 0;
    for (const SegmentVoxel& sv : segment)
      maxA = std::max(maxA, myAccumulation(sv.voxel));
    for (const SegmentVoxel& sv : segment)
      if (myAccumulation(sv.voxel) == maxA)
        myConfidence.setValue(sv.voxel, myConfidence(sv.voxel) + 1.0);
  }

  for (const Point& v : myDomain) {
    const int a = myAccumulation(v);
    myConfidence.setValue(v, a > 0 ? myConfidence(v) / static_cast<double>(a) : 0.0);
  }
}

} // namespace decsirp
