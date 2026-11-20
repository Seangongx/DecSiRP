#pragma once

/**
 * @file Viewer.h
 * Interactive viewer, based on Polyscope, showing the intermediate results
 * of the three steps and allowing to re-run each step with new parameters.
 */

#include <string>

#include "../core/Decomposition.h"

namespace decsirp {

/**
 * Opens a Polyscope window displaying the decomposition.
 *
 * @param decomposition a decomposition on which Steps I and II were run.
 * @param title window title.
 * @param cameraSettingsFile optional file containing camera settings, as
 * printed by the "print camera settings" button.
 */
void showViewer(Decomposition& decomposition, const std::string& title,
                const std::string& cameraSettingsFile = "");

} // namespace decsirp
