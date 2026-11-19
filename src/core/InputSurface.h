#pragma once

/**
 * @file InputSurface.h
 * Input data of the method (paper, Sect. 2.1, "Input Data").
 */

#include <string>
#include <vector>

#include "Types.h"

namespace decsirp {

/**
 * Oriented surface given as a set of pairs (x_i, n_i), together with the
 * facets f_i they come from.
 *
 * For each index i:
 *  - x_i = points[i] is the barycenter of the facet f_i = facets[i];
 *  - n_i = normals[i] is the unit normal of f_i, pointing inward.
 *
 * The facets are needed by the interior voxel test (Sect. 2.2) and by the
 * label extension on the dual graph (Sect. 2.3).
 *
 * All coordinates are expressed in voxel units: a mesh is scaled by 1/h,
 * where h is the grid step, so that voxels are unit cubes centered at
 * integer points.
 */
struct InputSurface
{
  std::vector<RealPoint> vertices; ///< mesh vertices
  std::vector<Facet> facets;       ///< facets f_i (indices into vertices)
  std::vector<RealPoint> points;   ///< surface points x_i
  std::vector<RealVector> normals; ///< inward unit normals n_i
  Domain domain;                   ///< voxel grid containing the surface

  /// Number of pairs (x_i, n_i), equal to the number of facets.
  std::size_t size() const { return points.size(); }
};

/**
 * Loads a polygonal mesh (any format readable by DGtal, e.g. OBJ or OFF).
 *
 * Facets are expected to be triangles, consistently oriented (counter-
 * clockwise when seen from outside). Only the first three vertices of each
 * facet are used to compute x_i and n_i.
 *
 * @param filename mesh file.
 * @param h grid step: the mesh is scaled by 1/h.
 */
InputSurface loadMesh(const std::string& filename, double h);

/**
 * Loads a 3D binary image (any format readable by DGtal, e.g. VOL) and
 * extracts its digital surface. Facets are surfels, x_i are the surfel
 * centers and n_i are estimated from the local geometry by averaging
 * trivial normals over a t-ring (paper, Sect. 3).
 *
 * @param filename image file.
 * @param tRing radius of the neighborhood used for normal estimation.
 */
InputSurface loadDigitalSurface(const std::string& filename, double tRing);

} // namespace decsirp
