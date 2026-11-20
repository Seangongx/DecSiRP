#pragma once

/**
 * @file Decomposition.h
 * Full pipeline: decomposition of a surface into round primitives.
 */

#include <memory>
#include <vector>

#include "InputSurface.h"
#include "InteriorVoxelClustering.h"
#include "VotingScheme.h"

namespace decsirp {

/**
 * User-defined parameters of the method (paper, Sect. 3, "Parameters").
 * Default values are those of the online demo; see Table 1 of the paper
 * for the values used in each experiment.
 */
struct Parameters
{
  double ell = 15.0;               ///< segment length
  CandidateThresholds thresholds;  ///< a, c, d
  double lambda = 0.5;             ///< inverse of the diffusion time
  double rho = 0.001;              ///< threshold of the label extension
};

/**
 * Runs the three stages of the method on an input surface:
 *  - Step I   (Sect. 2.1): voting scheme;
 *  - Step II  (Sect. 2.2): clustering of interior voxels, and mapping of
 *                          the cluster labels back to the surface;
 *  - Step III (Sect. 2.3): label extension.
 *
 * Each step can be re-run independently (e.g. from the viewer after a
 * parameter change); it invalidates the subsequent steps.
 */
class Decomposition
{
public:
  Decomposition(InputSurface surface, const Parameters& parameters);

  void computeVoting();         ///< Step I (uses ell); also runs Step II
  void computeClustering();     ///< Step II (uses a, c, d)
  void computeLabelExtension(); ///< Step III (uses lambda, rho)

  /// Runs Steps I, II and III.
  void run();

  Parameters& parameters() { return myParameters; }
  const InputSurface& surface() const { return mySurface; }
  const VotingScheme& voting() const { return *myVoting; }
  const InteriorVoxelClustering& clustering() const { return *myClustering; }

  /// Labels of the surface points after Step II (NoLabel if unlabeled).
  const std::vector<int>& seeds() const { return mySeeds; }
  /// Final labels sigma(x_i) after Step III (empty before Step III).
  const std::vector<int>& labels() const { return myLabels; }

private:
  InputSurface mySurface;
  Parameters myParameters;
  std::unique_ptr<VotingScheme> myVoting;
  std::unique_ptr<InteriorVoxelClustering> myClustering;
  std::vector<int> mySeeds;
  std::vector<int> myLabels;
};

/**
 * Exports the mesh with facets colored by label (OBJ + MTL).
 * If @a alsoMergedMaterials is true, a second OBJ file, suffixed with
 * "_mergeMat", is written with colors stored directly in the OBJ file.
 */
void exportSegmentedMesh(const InputSurface& surface, const std::vector<int>& labels,
                         const std::string& filename, bool alsoMergedMaterials = false);

} // namespace decsirp
