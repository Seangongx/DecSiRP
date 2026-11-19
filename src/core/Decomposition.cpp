#include "Decomposition.h"

#include <algorithm>
#include <fstream>

#include <DGtal/io/colormaps/GradientColorMap.h>
#include <DGtal/io/writers/MeshWriter.h>
#include <DGtal/shapes/Mesh.h>

#include "LabelExtension.h"

namespace decsirp {

//------------------------------------------------------------------------------
Decomposition::Decomposition(InputSurface surface, const Parameters& parameters)
  : mySurface(std::move(surface)), myParameters(parameters)
{}

//------------------------------------------------------------------------------
void Decomposition::computeVoting()
{
  DGtal::trace.beginBlock("Step I: voting scheme");
  myClustering.reset(); // depends on myVoting
  myVoting = std::make_unique<VotingScheme>(mySurface.points, mySurface.normals,
                                            mySurface.domain, myParameters.ell);
  DGtal::trace.endBlock();
  computeClustering();
}

//------------------------------------------------------------------------------
void Decomposition::computeClustering()
{
  DGtal::trace.beginBlock("Step II: clustering of interior voxels");
  myClustering = std::make_unique<InteriorVoxelClustering>(mySurface, *myVoting,
                                                           myParameters.thresholds);
  mySeeds = myClustering->labelsOnSurface();
  myLabels.clear();
  DGtal::trace.endBlock();
}

//------------------------------------------------------------------------------
void Decomposition::computeLabelExtension()
{
  DGtal::trace.beginBlock("Step III: label extension");
  const auto L = graphLaplacian(mySurface.facets.size(), dualGraphEdges(mySurface.facets));
  myLabels = extendLabels(L, mySeeds, myClustering->numberOfClusters(),
                          myParameters.lambda, myParameters.rho);
  DGtal::trace.endBlock();
}

//------------------------------------------------------------------------------
void Decomposition::run()
{
  computeVoting();
  computeLabelExtension();
}

//------------------------------------------------------------------------------
void exportSegmentedMesh(const InputSurface& surface, const std::vector<int>& labels,
                         const std::string& filename, bool alsoMergedMaterials)
{
  if (labels.size() != surface.facets.size()) {
    DGtal::trace.error() << "Run the label extension before exporting the segmented mesh."
                         << std::endl;
    return;
  }

  const int maxLabel = *std::max_element(labels.cbegin(), labels.cend());
  DGtal::GradientColorMap<int, DGtal::CMAP_VIRIDIS> colorMap(0, std::max(1, maxLabel + 1));

  DGtal::Mesh<RealPoint> mesh(true);
  for (const RealPoint& vertex : surface.vertices)
    mesh.addVertex(vertex);
  for (std::size_t f = 0; f < surface.facets.size(); ++f)
    mesh.addFace(surface.facets[f], colorMap(labels[f]) * 1.5); // brightened colors

  if (alsoMergedMaterials) {
    std::string mergedFilename = filename;
    mergedFilename.insert(mergedFilename.find_last_of("."), "_mergeMat");
    std::ofstream objStream(mergedFilename);
    std::ofstream mtlStream; // not opened: colors are written in the OBJ file
    DGtal::MeshWriter<RealPoint>::export2OBJ_colors(objStream, mtlStream, "", mesh);
  }
  mesh >> filename;
}

} // namespace decsirp
