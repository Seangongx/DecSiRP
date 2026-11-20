#include "InputSurface.h"

#include <cassert>
#include <map>
#include <set>

#include <DGtal/helpers/Shortcuts.h>
#include <DGtal/helpers/ShortcutsGeometry.h>
#include <DGtal/io/readers/MeshReader.h>
#include <DGtal/shapes/Mesh.h>

namespace decsirp {

using SH3 = DGtal::Shortcuts<DGtal::Z3i::KSpace>;
using SHG3 = DGtal::ShortcutsGeometry<DGtal::Z3i::KSpace>;

//------------------------------------------------------------------------------
InputSurface loadMesh(const std::string& filename, double h)
{
  DGtal::trace.beginBlock("Load mesh");

  InputSurface surface;
  const double scale = 1.0 / h;

  DGtal::Mesh<RealPoint> mesh(true);
  mesh << filename;

  // Surface points and inward normals.
  // Vertices 1 and 2 are swapped so that the normal of a counter-clockwise
  // facet points inward.
  for (unsigned int f = 0; f < mesh.nbFaces(); ++f) {
    const auto& face = mesh.getFace(f);
    const RealPoint p0 = mesh.getVertex(face.at(0));
    const RealPoint p1 = mesh.getVertex(face.at(2));
    const RealPoint p2 = mesh.getVertex(face.at(1));

    surface.normals.push_back(((p1 - p0).crossProduct(p2 - p0)).getNormalized());
    surface.points.push_back((p0 + p1 + p2) * scale / 3.0);
  }

  // Voxel grid: bounding box of the scaled mesh, with a margin of 2 voxels.
  const auto bb = mesh.getBoundingBox();
  surface.domain = Domain(bb.first * scale - RealPoint::diagonal(2.0),
                          bb.second * scale + RealPoint::diagonal(2.0));
  DGtal::trace.info() << surface.domain << std::endl;

  // Copy of the (scaled) mesh.
  surface.vertices.reserve(mesh.nbVertex());
  for (auto it = mesh.vertexBegin(); it != mesh.vertexEnd(); ++it)
    surface.vertices.push_back((*it) * scale);
  surface.facets.reserve(mesh.nbFaces());
  for (auto it = mesh.faceBegin(); it != mesh.faceEnd(); ++it)
    surface.facets.push_back(*it);

  DGtal::trace.info() << "Mesh #" << surface.vertices.size() << " vertices #"
                      << surface.facets.size() << " faces" << std::endl;
  DGtal::trace.endBlock();
  return surface;
}

//------------------------------------------------------------------------------
InputSurface loadDigitalSurface(const std::string& filename, double tRing)
{
  DGtal::trace.beginBlock("Load digital surface");

  InputSurface surface;

  auto params = SH3::defaultParameters() | SHG3::defaultParameters();
  params("t-ring", tRing);
  auto image = SH3::makeBinaryImage(filename, params);
  auto K = SH3::getKSpace(image);
  auto digitalSurface = SH3::makeLightDigitalSurface(image, K, params);
  auto surfels = SH3::getSurfelRange(digitalSurface, params);
  auto outwardNormals = SHG3::getCTrivialNormalVectors(digitalSurface, surfels, params);

  surface.domain = image->domain();
  DGtal::trace.info() << surface.domain << std::endl;

  // Vertices: pointels of the surface. A pointel with Khalimsky coordinates
  // p is embedded at p - (0.5, 0.5, 0.5), so that voxels are centered at
  // integer points.
  std::set<Point> pointels;
  for (const auto& s : *digitalSurface)
    for (const auto& v : SH3::getPrimalVertices(K, s, true))
      pointels.insert(K.uCoords(v));

  std::map<Point, std::size_t> vertexIndex;
  for (const Point& p : pointels) {
    surface.vertices.emplace_back(p[0] - 0.5, p[1] - 0.5, p[2] - 0.5);
    vertexIndex.emplace(p, surface.vertices.size() - 1);
  }

  // Facets: one quadrangle per surfel.
  for (const auto& s : *digitalSurface) {
    Facet facet;
    for (const auto& v : SH3::getPrimalVertices(K, s, true)) {
      auto it = vertexIndex.find(K.uCoords(v));
      assert(it != vertexIndex.end());
      facet.push_back(it->second);
    }
    surface.facets.push_back(facet);
  }

  DGtal::trace.info() << "Mesh #" << surface.vertices.size() << " vertices #"
                      << surface.facets.size() << " faces" << std::endl;

  // Surface points: surfel centers.
  for (const Facet& facet : surface.facets) {
    RealPoint x = RealPoint::zero;
    for (std::size_t k = 0; k < 4; ++k)
      x += surface.vertices[facet[k]];
    surface.points.push_back(x / 4.0);
  }

  // Inward normals.
  surface.normals.reserve(outwardNormals.size());
  for (const auto& n : outwardNormals)
    surface.normals.push_back(-n);

  DGtal::trace.endBlock();
  return surface;
}

} // namespace decsirp
