#pragma once

/**
 * @file Types.h
 * Common types shared by all stages of the DecSiRP pipeline.
 */

#include <cstddef>
#include <vector>

#include <DGtal/helpers/StdDefs.h>

namespace decsirp {

/// Integer point of Z^3, used to identify a voxel.
using Point = DGtal::Z3i::Point;
/// Point of R^3 (e.g. a surface point x_i or a mesh vertex).
using RealPoint = DGtal::Z3i::RealPoint;
/// Vector of R^3 (e.g. an inward normal n_i).
using RealVector = DGtal::Z3i::RealVector;
/// Axis-aligned box of voxels on which all images are defined.
using Domain = DGtal::Z3i::Domain;

/// A facet f_i, given by the indices of its vertices.
using Facet = std::vector<std::size_t>;

/// Label 0 means "no label" everywhere in the pipeline;
/// actual labels range from 1 to the number of clusters.
constexpr int NoLabel = 0;

} // namespace decsirp
