#include <igl/viewer/Viewer.h>
#include <directional/read_raw_field.h>
#include <directional/read_singularities.h>
#include <directional/singularity_spheres.h>
#include <directional/glyph_lines_raw.h>

int N;
Eigen::MatrixXi F, EV, FE, EF;
Eigen::MatrixXd V, C;
Eigen::MatrixXd rawField;
Eigen::VectorXi singIndices, singPositions;
igl::viewer::Viewer viewer;

Eigen::MatrixXd positiveIndexColors, negativeIndexColors;
Eigen::RowVector3d rawGlyphColor;
bool drawSingularities=false;


void update_mesh()
{

  C.resize(F.rows(),3);
  C.col(0)=Eigen::VectorXd::Constant(F.rows(),1.0);
  C.col(1)=Eigen::VectorXd::Constant(F.rows(),1.0);
  C.col(2)=Eigen::VectorXd::Constant(F.rows(),1.0);
  
  Eigen::MatrixXd fullV=V;
  Eigen::MatrixXi fullF=F;
  Eigen::MatrixXd fullC=C;
  
  if (drawSingularities)
    directional::singularity_spheres(V, F, singPositions, singIndices, positiveIndexColors, negativeIndexColors, false, true, fullV, fullF, fullC);
  
  directional::glyph_lines_raw(V, F, rawField, rawGlyphColor, false, true, fullV, fullF, fullC);
  
  viewer.data.clear();
  viewer.data.set_face_based(true);
  viewer.data.set_mesh(fullV, fullF);
  viewer.data.set_colors(fullC);
}


bool key_down(igl::viewer::Viewer& viewer, int key, int modifiers)
{
 
  switch (key)
  {
    case '1': drawSingularities=!drawSingularities; update_mesh(); break;
  }
  return true;
}



int main()
{
  std::cout <<"1  Show/hide Singularities" << std::endl;

  igl::readOFF(TUTORIAL_SHARED_PATH "/bumpy.off", V, F);
  directional::read_raw_field(TUTORIAL_SHARED_PATH "/bumpy.rawfield", N, rawField);
  directional::read_singularities(TUTORIAL_SHARED_PATH "/bumpy.sings", N, singPositions, singIndices);
  
  // Set colors for Singularities
  positiveIndexColors.resize(4,3);
  positiveIndexColors << .25, 0, 0,
  .5,  0, 0,
  .75, 0, 0,
  1,   0, 0;
  
  negativeIndexColors.resize(4,3);
  negativeIndexColors << 0, .25, 0,
  0, .5,  0,
  0, .75, 0,
  0, 1,   0;
  
  rawGlyphColor <<0.0, 0.2, 1.0;
  update_mesh();
  viewer.callback_key_down = &key_down;
  viewer.launch();
}

