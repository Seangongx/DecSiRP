#include "Viewer.h"

#include <fstream>
#include <iostream>

#include <DGtal/io/viewers/PolyscopeViewer.h>

namespace decsirp {

namespace {

//------------------------------------------------------------------------------
/// Shows the accumulation A and the confidence C on the voxel grid (Step I).
void showVoting(const Decomposition& decomposition)
{
  const Domain& domain = decomposition.surface().domain;
  const Point lb = domain.lowerBound();
  const Point ub = domain.upperBound();
  const Point size = ub - lb + Point::diagonal(1);

  auto grid = polyscope::registerVolumeGrid(
      "Grid", glm::uvec3{size[0], size[1], size[2]},
      glm::vec3{lb[0] + 0.5, lb[1] + 0.5, lb[2] + 0.5},
      glm::vec3{ub[0] + 0.5, ub[1] + 0.5, ub[2] + 0.5});
  grid->setEnabled(false);

  const auto& A = decomposition.voting().accumulation();
  const auto& C = decomposition.voting().confidence();
  std::vector<float> accumulation, confidence;
  accumulation.reserve(domain.size());
  confidence.reserve(domain.size());
  for (const Point& v : domain) {
    accumulation.push_back(A(v));
    confidence.push_back(C(v));
  }
  grid->addNodeScalarQuantity("Accumulation A", accumulation)->setEnabled(true);
  grid->addNodeScalarQuantity("Confidence C", confidence)->setEnabled(true);
}

//------------------------------------------------------------------------------
/// Shows the interior voxels and the facets labeled after Step II.
void showClusters(const Decomposition& decomposition)
{
  // Remove the structures of a previous run.
  polyscope::removePointCloud("Interior voxels", false);
  polyscope::removeSurfaceMesh("Labeled facets", false);

  std::vector<Point> interiorVoxels;
  std::vector<double> reachableDistances;
  std::vector<int> voxelLabels;
  for (const auto& info : decomposition.clustering().voxels()) {
    if (info.interior) {
      interiorVoxels.push_back(info.voxel);
      reachableDistances.push_back(info.reachableDistance);
      voxelLabels.push_back(info.label);
    }
  }
  if (interiorVoxels.empty())
    return;

  auto cloud = polyscope::registerPointCloud("Interior voxels", interiorVoxels);
  cloud->addScalarQuantity("Reachable distance D", reachableDistances);
  cloud->addScalarQuantity("Labels", voxelLabels);

  const InputSurface& surface = decomposition.surface();
  const std::vector<int>& seeds = decomposition.seeds();
  std::vector<Facet> seededFacets;
  std::vector<int> seededLabels;
  for (std::size_t i = 0; i < seeds.size(); ++i) {
    if (seeds[i] != NoLabel) {
      seededFacets.push_back(surface.facets[i]);
      seededLabels.push_back(seeds[i]);
    }
  }
  auto facets = polyscope::registerSurfaceMesh("Labeled facets", surface.vertices, seededFacets);
  facets->addFaceScalarQuantity("Labels", seededLabels);
  facets->setEnabled(true);

  if (auto labels = polyscope::getSurfaceMesh("Mesh")->getQuantity("Labels"))
    labels->setEnabled(false);
}

//------------------------------------------------------------------------------
/// Shows the final labels sigma(x_i) on the mesh (Step III).
void showLabels(const Decomposition& decomposition)
{
  polyscope::getSurfaceMesh("Mesh")
      ->addFaceScalarQuantity("Labels", decomposition.labels())
      ->setEnabled(true);
  if (polyscope::hasSurfaceMesh("Labeled facets"))
    polyscope::getSurfaceMesh("Labeled facets")->setEnabled(false);
}

//------------------------------------------------------------------------------
void loadCameraSettings(const std::string& filename)
{
  std::ifstream file(filename);
  std::string json;
  if (file.is_open() && std::getline(file, json)) {
    std::cout << "# Camera settings set to: " << json << std::endl;
    polyscope::view::setViewFromJson(json, false);
  }
}

/// ImGui slider on a double.
bool slider(const char* label, double& value, double min, double max)
{
  return ImGui::SliderScalar(label, ImGuiDataType_Double, &value, &min, &max);
}

} // namespace

//------------------------------------------------------------------------------
void showViewer(Decomposition& decomposition, const std::string& title,
                const std::string& cameraSettingsFile)
{
  polyscope::options::programName = title;
  DGtal::PolyscopeViewer viewer;
  polyscope::options::groundPlaneMode = polyscope::GroundPlaneMode::None;
  polyscope::view::setNavigateStyle(polyscope::NavigateStyle::Free);
  polyscope::loadColorMap("Random", "random.png");
  loadCameraSettings(cameraSettingsFile);

  const InputSurface& surface = decomposition.surface();
  polyscope::registerSurfaceMesh("Mesh", surface.vertices, surface.facets);
  auto points = polyscope::registerPointCloud("Surface points x_i", surface.points);
  points->addVectorQuantity("Normals n_i", surface.normals);
  points->setEnabled(false);

  showVoting(decomposition);
  showClusters(decomposition);

  Parameters& p = decomposition.parameters();
  polyscope::state::userCallback = [&]() {
    if (ImGui::Button("print camera settings"))
      std::cout << polyscope::view::getViewAsJson() << std::endl;

    ImGui::Separator();
    ImGui::Text("Step I: voting scheme");
    slider("ell (segment length)", p.ell, 1.0, 100.0);
    if (ImGui::Button("compute A and C")) {
      decomposition.computeVoting();
      showVoting(decomposition);
      showClusters(decomposition);
    }

    ImGui::Separator();
    ImGui::Text("Step II: clustering of interior voxels");
    ImGui::SliderInt("a (accumulation)", &p.thresholds.a, 1, 100);
    slider("c (confidence)", p.thresholds.c, 0.001, 1.0);
    slider("d (reachable distance)", p.thresholds.d, 0.0, 100.0);
    if (ImGui::Button("cluster")) {
      decomposition.computeClustering();
      showClusters(decomposition);
    }

    ImGui::Separator();
    ImGui::Text("Step III: label extension");
    slider("lambda", p.lambda, 0.01, 1.0);
    slider("rho", p.rho, 0.0, 1.0);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.55f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.65f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.45f, 0.2f, 1.0f));
    if (ImGui::Button("extend labels")) {
      decomposition.computeLabelExtension();
      showLabels(decomposition);
    }
    ImGui::PopStyleColor(3);

    ImGui::Separator();
    ImGui::Text("IO");
    if (ImGui::Button("save segmented mesh (result.obj)"))
      exportSegmentedMesh(surface, decomposition.labels(), "result.obj");
  };

  // Printed for the IPOL demo script, which takes a screenshot at this point.
  std::cout << "[display ready]" << std::endl;
  viewer.show();
}

} // namespace decsirp
