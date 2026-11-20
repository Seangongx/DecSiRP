/**
 * @file decSiRP.cpp
 * Command-line tool: decomposition of surfaces into round primitives.
 *
 * Implementation of the paper "A Voting Scheme for Decomposing Surfaces into
 * Round Primitives", X. Gong, T. Roussillon, B. Kerautret.
 */

#include <iostream>
#include <string>

// CLI11, see https://github.com/CLIUtils/CLI11
#include "CLI11.hpp"

#include "core/Decomposition.h"
#include "core/InputSurface.h"
#ifdef DECSIRP_WITH_VIEWER
#include "viewer/Viewer.h"
#endif

int main(int argc, char* argv[])
{
  using namespace decsirp;

  CLI::App app;
  app.option_defaults()->always_capture_default(); // show default values in --help
  app.description("Decomposition of surfaces into round primitives using a voting scheme (DecSiRP).");

  // Input.
  std::string meshFile, imageFile;
  double h = 1.0;
  double tRing = 3.0;
  auto meshOpt = app.add_option("-m", meshFile, "input mesh (OBJ, OFF...)")->check(CLI::ExistingFile);
  auto imageOpt = app.add_option("-i", imageFile, "input 3D binary image (VOL...), whose digital surface is processed")->check(CLI::ExistingFile);
  auto hOpt = app.add_option("-g", h, "grid step h (mesh only)");
  auto tRingOpt = app.add_option("-t", tRing, "t-ring used for the normal estimation (image only)");
  meshOpt->excludes(imageOpt);
  hOpt->needs(meshOpt);
  tRingOpt->needs(imageOpt);

  // Parameters (paper, Sect. 3, "Parameters"); default values are given in
  // core/Decomposition.h.
  Parameters p;
  app.add_option("-l", p.ell, "segment length ell");
  app.add_option("-a", p.thresholds.a, "accumulation threshold a");
  app.add_option("-c", p.thresholds.c, "confidence threshold c");
  app.add_option("-d", p.thresholds.d, "reachable distance threshold d");
  app.add_option("--lambda", p.lambda,
                 "label extension: values close to 1 limit the extension, "
                 "values close to 0 allow maximal extension");
  app.add_option("--rho", p.rho, "threshold of the label extension");

  // Output.
  std::string outputFile;
  bool mergedMaterials = false;
  std::string cameraSettingsFile;
  app.add_option("--export-segmentation", outputFile,
                 "run all steps and export the colored mesh (OBJ) without opening the viewer");
  app.add_flag("--mergeMat", mergedMaterials,
               "with --export-segmentation, also export an OBJ file with merged materials");
  app.add_option("-s", cameraSettingsFile, "file storing the camera settings of the viewer");

  app.get_formatter()->column_width(40);
  CLI11_PARSE(app, argc, argv);

  if (meshFile.empty() && imageFile.empty()) {
    std::cerr << "Error: an input mesh (-m) or image (-i) is required." << std::endl;
    return 1;
  }
  const bool batchMode = !outputFile.empty();
#ifndef DECSIRP_WITH_VIEWER
  if (!batchMode) {
    std::cerr << "Error: built without the viewer, --export-segmentation is required." << std::endl;
    return 1;
  }
#endif

  Decomposition decomposition(meshFile.empty() ? loadDigitalSurface(imageFile, tRing)
                                               : loadMesh(meshFile, h),
                              p);

  if (batchMode) {
    decomposition.run();
    exportSegmentedMesh(decomposition.surface(), decomposition.labels(), outputFile,
                        mergedMaterials);
  } else {
#ifdef DECSIRP_WITH_VIEWER
    decomposition.computeVoting();
    const std::string& inputFile = meshFile.empty() ? imageFile : meshFile;
    const std::string name = inputFile.substr(inputFile.find_last_of('/') + 1);
    showViewer(decomposition, "DecSiRP - " + name, cameraSettingsFile);
#endif
  }
  return 0;
}
