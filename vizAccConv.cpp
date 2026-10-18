#include <iostream>
#include <sstream>

#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/helpers/Shortcuts.h>
#include <DGtal/helpers/ShortcutsGeometry.h>
#include <DGtal/shapes/SurfaceMesh.h>
#include <DGtal/io/writers/SurfaceMeshWriter.h>
#include <DGtal/io/viewers/PolyscopeViewer.h>

//We use CLI11 to manage options
//see https://github.com/CLIUtils/CLI11
//and https://cliutils.gitlab.io/CLI11Tutorial/
#include "ext/CLI11.hpp"

//project-specific
#define VERBOSE
#include "managers/SurfaceManager.ih"

#include "Accumulator.ih"
#include "InteriorVoxelComponents.ih"
#include "targetedClustering/DualGraphFromManifoldMesh.ih"



//--------------------------------------------------------------------------------
//-- main class
//--------------------------------------------------------------------------------
class PolyscopeManager {

public: 
  //--- useful types
  typedef DGtal::Shortcuts<DGtal::Z3i::KSpace> SH3;
  typedef SH3::KSpace KSpace; 
  typedef SH3::Space Space;
  typedef SH3::Domain Domain; 
  typedef SH3::Point Point;
  typedef SH3::Vector Vector;
  typedef SH3::RealPoint RealPoint;
  typedef SH3::RealVector RealVector;
  typedef DGtal::SurfaceMesh< RealPoint, RealVector > Mesh;
  typedef DGtal::SurfaceMeshWriter< RealPoint, RealVector > MeshWriter;
  typedef std::size_t Index;
  typedef std::vector< std::size_t > Indices;
  
  //--- Other useful alias
  typedef std::set<Point> PointCloud;

  using Image = ImageContainerBySTLVector<Domain, int>;
  using Clustering =
    InteriorVoxelComponents<Point, RealPoint>;

  //--------------------------------------------------------------------------------
  //- members of the main class
  //--------------------------------------------------------------------------------
  SurfaceManager<KSpace> mySurface; //surface
  Accumulator<Domain>* myAcc;       //accumulator
  Clustering* myClust;             //clustering
  //and associated parameters
  float myMaxLength;
  float myBeta;
  float myConfidenceThreshold;
  int myAccumulationThreshold;
  float myDistThreshold;
  float myProbThreshold; 
  //extra data for visualization
  std::vector<Point> myInteriorVoxels;
  std::map<int,int> myIdxIntVoxToIdxInfo; 

  //--------------------------------------------------------------------------------
  //- ctor
  //--------------------------------------------------------------------------------
  PolyscopeManager(double length, double beta, double accT, double confT, double dT, double probT):
    mySurface(), myAcc(nullptr), myClust(nullptr),
    myMaxLength(length), myBeta(beta), myAccumulationThreshold(accT),
    myConfidenceThreshold(confT), myDistThreshold(dT), myProbThreshold(probT), 
    myInteriorVoxels(), myIdxIntVoxToIdxInfo()
  {
  }

  void initFromPolynomial(std::string const& polynomial, double const& gridstep, 
			  std::string const& cameraSettings = "")
  {
    mySurface.buildFromPolynomial(polynomial, gridstep);
    init();
    show(cameraSettings); 
  }

  void initFromMesh(std::string const& file, double const& gridstep, 
		    std::string const& cameraSettings = "")
  {
    mySurface.buildFromMesh(file, gridstep);
    init();
    show(cameraSettings); 
  }
  
  void initFromImage(std::string const& file,  
		     std::string const& cameraSettings = "")
  {
    mySurface.buildFromImage( file );
    init();
    show(cameraSettings); 
  }

  //--------------------------------------------------------------------------------
  void init() { 

    //computations
    DGtal::trace.beginBlock( "Accumulation" ); 

    myAcc = new Accumulator(mySurface.myPoints, mySurface.myNormals, mySurface.myDomain, myMaxLength);
    DGtal::trace.info() << myAcc->getImage() << std::endl; 

    DGtal::trace.beginBlock( "Confidence scores" ); 

    //myAcc->computeConfidenceScore(Accumulator<Domain>::ConfidenceMode::Base);
    //myAcc->computeConfidenceScore(Accumulator<Domain>::ConfidenceMode::AlternativeA);
    myAcc->computeConfidenceScore(Accumulator<Domain>::ConfidenceMode::AlternativeB, myBeta);

    DGtal::trace.info() << myAcc->getConfidenceScores() << std::endl; 

    DGtal::trace.endBlock(); 

    clustering(); 
  }

  //--------------------------------------------------------------------------------
  void reset() { 
    delete(myAcc);
    delete(myClust); 
    init(); 
    showAccConf();
    showClusters(); 
  }
  
  //--------------------------------------------------------------------------------
  void clustering() {
    
    DGtal::trace.beginBlock( "Clustering of interior voxels" ); 

    auto keepVoxel = [&](const Point& p)
    {
      return myAcc->getImage()(p) >= myAccumulationThreshold
        && myAcc->getConfidenceScores()(p) >= myConfidenceThreshold;
    };
    
    myClust = new Clustering(mySurface.myPoints, myAcc->rasterizedSegments,
			     mySurface.myVertices, mySurface.myFaces, keepVoxel,
			     myDistThreshold);
    
    DGtal::trace.info()
      << "Number of connected components : "
      << myClust->numberOfComponents()
      << std::endl;

    DGtal::trace.endBlock(); 
    
  }

  //--------------------------------------------------------------------------------
  void redoClustering() {
    delete(myClust);
    clustering();
    showClusters(); 
  }

    //--------------------------------------------------------------------------------
  void expansion() {

    int nn = myClust->numberOfComponents(); 

    //build seeds
    std::vector<int> seeds(mySurface.myFaces.size(), 0);

    auto callback = [&seeds](int faceIdx, int componentIdx) {
      seeds[faceIdx] = componentIdx; 
    };
    myClust->getFacetsComponents(callback);

    //build dual graph
    auto dualEdges = buildDualGraph(mySurface.myFaces);

    //the graph laplacian
    auto L = buildLaplacian(mySurface.myFaces.size(),
			    dualEdges);

    //and segment
    auto segmentation = targetedClustering(L, seeds, nn, myProbThreshold);

    polyscope::getSurfaceMesh("Mesh")->addFaceScalarQuantity("Classes", segmentation); 

  }
  
  //--------------------------------------------------------------------------------
  void show(std::string const& cameraSettings) { 

    //polyscope init
    DGtal::PolyscopeViewer viewer;
    polyscope::loadColorMap("Random", "../random.png");
    setCameraSettings(cameraSettings); 
    polyscope::options::groundPlaneMode = polyscope::GroundPlaneMode::None;
	
    //mesh
    auto surface = polyscope::registerSurfaceMesh("Mesh",
						  mySurface.myVertices,
						  mySurface.myFaces);

    //points and normal
    auto points = polyscope::registerPointCloud("Points",
						mySurface.myPoints); 
    points->addVectorQuantity("Normals", mySurface.myNormals); 
    points->setEnabled(false);
    
    //acc/conf values
    showAccConf();

    //clusters
    showClusters(); 
    
    //call back
    polyscope::state::userCallback =  [&]() {

      //---- mouse ----
      ImGuiIO &io = ImGui::GetIO();
      mouseSelection(io);

      //buttons
      if (ImGui::Button("getViewAsJson")) {
	std::cout << polyscope::view::getViewAsJson() << std::endl; 
      }
      
      ImGui::SliderFloat("length", &myMaxLength, 1.0, 100.0);
      ImGui::SliderFloat("beta  ", &myBeta, 1.0, 100.0);

      if (ImGui::Button("reset (acc/conf)")) {
	reset();
      }
      
      ImGui::SliderInt("Acc.  threshold", &myAccumulationThreshold, 1, 100);
      ImGui::SliderFloat("Conf. threshold", &myConfidenceThreshold, 0.001, 1.0);
      ImGui::SliderFloat("dist. threshold", &myDistThreshold, 0.0, 100.0);
      
      if (ImGui::Button("clustering")) {
	redoClustering();  
      }

      ImGui::SliderFloat("prob. threshold", &myProbThreshold, 0.0, 1.0);

      if (ImGui::Button("expand")) {
	expansion();  
      }
      
    };
    
    viewer.show();
      
  }

  //--------------------------------------------------------------------------------
  void showAccConf() {
    
    // register the grid
    Point lb = mySurface.myDomain.lowerBound();
    Point ub = mySurface.myDomain.upperBound();
    Vector d = ub-lb + Point::diagonal(1); 
    glm::uvec3 dims{d[0],d[1],d[2]}; 
    glm::vec3 bound_low{lb[0]+0.5, lb[1]+0.5, lb[2]+0.5};
    glm::vec3 bound_high{ub[0]+0.5, ub[1]+0.5, ub[2]+0.5};
    uint32_t nData = d[0]*d[1]*d[2];
    
    auto grid = polyscope::registerVolumeGrid("Grid", dims, bound_low, bound_high);
    grid->setEnabled(false); 

    // add acc values
    std::vector<float> vacc;
    vacc.reserve(nData);
    for (auto it = mySurface.myDomain.begin();
	 it != mySurface.myDomain.end(); ++it) {
      vacc.push_back(myAcc->getImage()(*it)); 
    }
    auto gridvacc = grid->addNodeScalarQuantity("Acc", vacc);
    gridvacc->setEnabled(true);

    // add conf values
    std::vector<float> vconf;
    vconf.reserve(nData);
    for (auto it = mySurface.myDomain.begin();
	 it != mySurface.myDomain.end(); ++it) {
      vconf.push_back(myAcc->getConfidenceScores()(*it)); 
    }
    auto gridvconf = grid->addNodeScalarQuantity("Conf", vconf);
    gridvconf->setEnabled(true);
  
  }

  //--------------------------------------------------------------------------------
  void showClusters() {

    //clusters / interior voxels
    myInteriorVoxels.clear();
    myIdxIntVoxToIdxInfo.clear(); 
      
    std::vector<double> ds;
    std::vector<int> cs; 
    const auto& voxels = myClust->getVoxels();
    for (std::size_t i = 0; i < voxels.size(); i++) {
      auto v = voxels[i]; 
      if (v.kept) {
	myInteriorVoxels.push_back(v.voxel);
	myIdxIntVoxToIdxInfo.emplace(myInteriorVoxels.size()-1, i); 
	ds.push_back(v.distance);
	cs.push_back(v.component); 
      }
    }

    if (myInteriorVoxels.size() > 0) {
      auto pc = polyscope::registerPointCloud("Interior Voxels", myInteriorVoxels); 
      pc->addScalarQuantity("Distances", ds);
      pc->addScalarQuantity("Components", cs);

      //clusters / associated faces
      std::vector<std::vector<std::size_t> > facesInteriorVoxels;
      std::vector<int> facesComponent; 

      auto callback = [this, &facesInteriorVoxels, &facesComponent](int faceIdx, int componentIdx) {
	  facesInteriorVoxels.push_back(mySurface.myFaces[faceIdx]);
	  facesComponent.push_back(componentIdx); 
	};
      myClust->getFacetsComponents(callback);      

      std::cout << facesInteriorVoxels.size() << std::endl; 
      std::cout << facesComponent.size() << std::endl; 
      auto fptr = polyscope::registerSurfaceMesh("Faces / Interior Voxels",
						 mySurface.myVertices,
						 facesInteriorVoxels);
      fptr->addFaceScalarQuantity("Components", facesComponent); 
    }    
  }
  
  //--------------------------------------------------------------------------------
  //- interaction
  //--------------------------------------------------------------------------------

  void isInterior(const Clustering::VoxelInfo& info) {
    
    std::vector<RealPoint> points;
    points.reserve(info.origins.size() + 1);

    //first point is info.voxel
    points.push_back(RealPoint(info.voxel));

    //others are vertices of associated facets
    for (const auto& sidx : info.segments) {
      auto face = mySurface.myFaces[sidx];
      for (const auto& vidx : face) {
	points.push_back(mySurface.myVertices[vidx]);
      }
    }

    auto pc = polyscope::registerPointCloud("Input Points", points); 

    if (points.size() > 2) {
      
      using Kernel3D = DGtal::ConvexHullRationalKernel<3>;
      using QuickHull3D = DGtal::QuickHull<Kernel3D>;
      Kernel3D kernel(1000);
      QuickHull3D hull(kernel);

      hull.setInput(points);
      hull.computeConvexHull();

      std::vector< RealPoint > positions;
      hull.getVertexPositions( positions );
      std::vector< std::vector< std::size_t > > facets;
      hull.getFacetVertices( facets );
      auto ch = polyscope::registerSurfaceMesh("Convex hull", positions, facets);    
      QuickHull3D::IndexRange p2v;
      hull.getPoint2Vertex( p2v );
      std::cout << (p2v[hull.input2comp[0]] == QuickHull3D::UNASSIGNED) << std::endl; 

    } else {
      std::cout << "0" << std::endl; 
    }
  }
  
  //--------------------------------------------------------------------------------
  void mouseSelection(ImGuiIO const& io) {

    if (io.MouseDoubleClicked[0]) { // if the left mouse button was double-clicked

      // get selection
      std::pair<polyscope::Structure*, size_t> selection = 
        polyscope::pick::evaluatePickQuery(io.MousePos.x, io.MousePos.y);

      if (selection.first != nullptr) {
		
	if (selection.first->getName() == "Interior Voxels") {
	  auto idx = selection.second; 
	  std::clog << "# Select " << idx << "-th point in "
		    << selection.first->getName() << ", " << selection.first->typeName() << std::endl;
	  std::clog << "# point " << myInteriorVoxels[idx] << std::endl;
	  auto it = myIdxIntVoxToIdxInfo.find(idx);
	  if (it != myIdxIntVoxToIdxInfo.end()) {
	    auto v = myClust->myVoxels[it->second]; 
	    std::clog << "# "
		      << v.voxel << " "
		      << v.interior << " "
		      << v.distance << " "
		      << v.origins.size() << std::endl;
	    isInterior(v); 
	  }
	} else {
	  DGtal::trace.error() << "# Structure not supported" << std::endl; 
	}
      }
      
    }
  }
  
  
  //--------------------------------------------------------------------------------
  void setCameraSettings(std::string const& cameraSettings) {

    std::string line;
    std::ifstream file(cameraSettings);
    if (file.is_open()) {
      std::getline(file,line);
      file.close();
      std::cout << "# Camera settings set to: "
		<< line << std::endl; 
      polyscope::view::setViewFromJson(line, false);
    }
  }

  
}; 
  
//--------------------------------------------------------------------------------
//-- main 
//--------------------------------------------------------------------------------
int main (int argc, char* argv[]) {

  // parse command line using CLI
  CLI::App app;
  
  app.description("Visualization of accumulation values and confidence scores, \
 identification of interior and voxels and segmentation \n");

  std::string polynomial = "2*x+6*y+15*z"; //or "x^2+y^2+2*z^2-x*y*z+z^3-100"
  auto polynomialList = DGtal::Shortcuts<DGtal::Z3i::KSpace>::getPolynomialList();  
  std::stringstream ss;
  ss << "A polynomial";
  if (polynomialList.size() > 0) {
    ss << " (or a name in the following list: "
       << polynomialList.begin()->first; 
    for (auto it = ++polynomialList.begin(), itEnd = polynomialList.end();
	 it != itEnd; ++it) {
      ss << ", " << it->first; 
    }
    ss << ")"; 
  }

  auto p_opt = app.add_option("-p", polynomial, ss.str());
  double gridstep = 1.0;
  auto g_opt = app.add_option("-g", gridstep, "Grid step");
  
  std::string image = "_default_";
  auto i_opt = app.add_option("-i", image, "image filename");
  std::string mesh = "_default_"; 
  auto m_opt = app.add_option("-m", mesh, "mesh filename");

  //either -p, -g or -i or -m -g
  i_opt->excludes(p_opt);
  i_opt->excludes(g_opt); 
  i_opt->excludes(m_opt);
  m_opt->excludes(p_opt);
  m_opt->excludes(i_opt);

  //parameters
  double maxLength = 5.0; 
  double beta = 5.0;
  double confThreshold = 0.01;
  double accThreshold = 5; 
  double distThreshold = 1.0;
  double probThreshold = 0.8; 
  
  app.add_option("-l", maxLength, "maximal ray length");
  app.add_option("--beta", maxLength, "beta parameter of the softmax function");
  app.add_option("-a", accThreshold, "accumulation threshold");
  app.add_option("-c", confThreshold, "confidence threshold");
  app.add_option("-d", distThreshold, "distance threshold");
  app.add_option("--proba", probThreshold, "probability threshold");
  
  std::string cameraSettings = ""; 
  app.add_option("-s", cameraSettings,
		 "name of a file storing the camera settings");
  
  app.get_formatter()->column_width(40);
  CLI11_PARSE(app, argc, argv);
  // END parse command line using CLI

  // polyscope session
  PolyscopeManager pm(maxLength, beta,
		       accThreshold, confThreshold,
		       distThreshold, probThreshold);
  
  if ( app.count("-i") != 0 ) {
    pm.initFromImage(image, cameraSettings);
  } else if ( app.count("-m") != 0 ) {
    pm.initFromMesh(mesh, gridstep, cameraSettings);
  } else {
    pm.initFromPolynomial(polynomial, gridstep, cameraSettings);
  }

  
  return 0;
}
