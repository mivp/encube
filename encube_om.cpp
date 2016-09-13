/*
 * encube_om.cpp
 *
 * Copyright (c) 2015-2016 Dany Vohl, David G. Barnes, Christopher J. Fluke,
 *                         Yuri Benovitski, Tsz Ho Wong, Owen L Kaluza, Toan D. Nguyen.
 *
 * This file is part of encube.
 *
 * encube is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * encube is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with encube.  If not, see <http://www.gnu.org/licenses/>.
 *
 * We would appreciate it if research outcomes using encube would
 * provide the following acknowledgement:
 *
 * "Visual analytics of multidimensional data was conducted with encube."
 *
 * and a reference to
 *
 * Dany Vohl, David G. Barnes, Christopher J. Fluke, Govinda Poudel, Nellie Georgiou-Karistianis,
 * Amr H. Hassan, Yuri Benovitski, Tsz Ho Wong, Owen L Kaluza, Toan D. Nguyen, C. Paul Bonnington. (2016).
 * Large-scale comparative visualisation of sets of multidimensional data. PeerJ Computer Science, In Press.
 *
 */

/*
 * Previously: vr.cpp
 *
 * Example use - 3d texture volume rendering - of S2PLOT API embedded in OmegaLib
 *
 * David Barnes, May 2014
 *
 * Copyright and license to be advised
 */

//#define USE_S2_VR

#include "s2omega.cpp"

#include <float.h>
#include "utility.h"
#include "hdsupport.h"
#include "hdglobals.h"
#include "float16.c"
#include "json.h"

extern "C" {
#include "libxrw.h"
}

#include "shaderService.c"
#include "routines.h"


//#include "hdsupport.c"

void initGL(void);
#define USE_GLEW
//-shaderService *vrShaders = NULL;
extern bool bufferAllFlag; // = false;
int show_volume = 1;

//float volumeScale = 2.0f;

// parameters from web
extern AABB clpb; // = make_aabb(make_float3(0), make_float3(1));
extern AABB aabb;// = clpb;

// parameters from web
//--float fov = 30.0;
//--float focalLength = 1.0 / tan(0.5 * fov * PI/180.0);;
float focalLength = 3.6;
//float bbmin[3] = {-1, -1, -1}, bbmax[3] = {1, 1, 1};
float3 bbmin = make_float3(-1);
float3 bbmax = make_float3(1);
float3 clpbmin = make_float3(0);
float3 clpbmax = make_float3(1);
//float viewport_width, viewport_height;

// END: parameters from web

#define GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX          0x9047
#define GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    0x9048
#define GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  0x9049
#define GPU_MEMORY_INFO_EVICTION_COUNT_NVX            0x904A
#define GPU_MEMORY_INFO_EVICTED_MEMORY_NVX            0x904B

void oglDraw(int *eye);

void initShaders(shaderService **ss, char *filename) {
  if(*ss != NULL) destroyShaderService(*ss);
  *ss = (shaderService*)malloc(sizeof(shaderService));
  initShaderService(*ss, filename, 0, 1);
}

// a type to store a volume and its texture
typedef struct {
  int Nx, Ny, Nz;
  float wdx, wdy, wdz; // world dx, dy, dz
  float ***array;
  float dmin, dmax; // for current mapping to texid
  float data_power; // for current mapping to texid
  float amin, amax; // alpha min, max
  int cmapidx;
  int c1, c2; // colourmap start, end
  float valpha; // overall volume alpha
  int texid;

  GLuint brainTex_id;
  GLfloat *brainTexData;

  int refresh; // use to force refresh of texture
  int quality; // 1 = no subsampling, 2 = draw half the planes, ... 


} VolData;

void volToTexture(VolData *vd);

// a global volume and texture
VolData myVolData;

//#define NSTEPS 1
//#define FNAMEPATTERN "xrw_fullsize/%02d.xrw"
#define NSTEPS 1
#define FNAMEPATTERN "xrw_64th/%02d.xrw"
VolData steppedVolData[NSTEPS];
int _step = 0;

// helper functions
VolData initVolume_xrw(char *fname);
void textureVolumeRender(void);
void vr(double *, int *);

void cube2texture(VolData *vd);
void s2textureVolumeRender(void);

class vrApp;

// ApplicationBase entry point
int main(int argc, char** argv) {
  Application<vrApp> app("vr");
  return omain(app, argc, argv);
}

class vrRenderPass : public s2omegaRenderPass {
public:
  vrRenderPass(Renderer *client, vrApp *app);
  virtual void s2main();
};

class vrApp : public s2omegaApplication {
public: 
  vrApp() : s2omegaApplication() { }
  virtual void initialize();
  virtual void onMenuItemEvent(MenuItem *mi);
  virtual void initializeRenderer(Renderer *r) {
    r->addRenderPass(new vrRenderPass(r, this));
  }
  MenuItem *myColormapMI, *myDataPowerMI, *myAlphaMI, *myQualityMI;
};

vrRenderPass::vrRenderPass(Renderer *client, vrApp *app) :
  s2omegaRenderPass(client, app) {
}

void vrApp::initialize() {
  s2omegaApplication::initialize();
  ui::Menu *menu = myMenuManager->getMainMenu();

  myColormapMI = menu->addItem(MenuItem::Button);
  myColormapMI->setText("Next colormap");
  myColormapMI->setListener(this);

  MenuItem *myDataPowerMI_lab = menu->addItem(MenuItem::Label);
  myDataPowerMI_lab->setText("Data scaling strength");
  myDataPowerMI = menu->addItem(MenuItem::Slider);
  ui::Slider *tmpSlider = myDataPowerMI->getSlider();
  tmpSlider->setTicks(11);
  tmpSlider->setValue(4);
  myDataPowerMI->setListener(this);

  MenuItem *myAlphaMI_lab = menu->addItem(MenuItem::Label);
  myAlphaMI_lab->setText("Overall volume opacity");
  myAlphaMI = menu->addItem(MenuItem::Slider);
  tmpSlider = myAlphaMI->getSlider();
  tmpSlider->setTicks(11);
  tmpSlider->setValue(5);
  myAlphaMI->setListener(this);

  MenuItem *myQualityMI_lab = menu->addItem(MenuItem::Label);
  myQualityMI_lab->setText("dQuality [lower is better]");
  myQualityMI = menu->addItem(MenuItem::Slider);
  tmpSlider = myQualityMI->getSlider();
  tmpSlider->setTicks(10);
  tmpSlider->setValue(2);
  myQualityMI->setListener(this);

}


void vrApp::onMenuItemEvent(MenuItem *mi) {
  s2omegaApplication::onMenuItemEvent(mi);
  int i;
  if (mi == myColormapMI) {
    myVolData.cmapidx++;
    myVolData.refresh = 1;
    for (i = 0; i < NSTEPS; i++) {
      if (i != _step) {
	steppedVolData[i].cmapidx++;
	steppedVolData[i].refresh = 1;
      }
    }
  } else if (mi == myDataPowerMI) {
    ui::Slider *slider = myDataPowerMI->getSlider();
    int val = slider->getValue();
    myVolData.data_power = (float)(val - 5.0);
    myVolData.refresh = 1;
    for (i = 0; i < NSTEPS; i++) {
      if (i != _step) {
	steppedVolData[i].data_power = (float)(val - 5.0);
	steppedVolData[i].refresh = 1;
      }
    }
  } else if (mi == myAlphaMI) {
    ui::Slider *alphaSlider = myAlphaMI->getSlider();
    int val = alphaSlider->getValue();
    myVolData.valpha = (float)val / 10.0;
    for (i = 0; i < NSTEPS; i++) {
      if (i != _step) {
	steppedVolData[i].valpha = (float)val / 10.0;
      }
    }
  } else if (mi == myQualityMI) {
    ui::Slider *tmpSlider = myQualityMI->getSlider();
    int val = tmpSlider->getValue();
    myVolData.quality = val + 1;
    for (i = 0; i < NSTEPS; i++) {
      if (i != _step) {
	steppedVolData[i].quality = val + 1;
      }
    }
  }
}

void vrRenderPass::s2main() {

  // load individual volume
  //myVolData = initVolume_xrw((char *)"MeanImage.xrw");
  //myVolData = initVolume_xrw((char *)"andreas00.xrw");
  //myVolData = initVolume_xrw((char *)"xrw_64th/10.xrw");

  // load sequence of volumes
  int i;
  char fname[200];
  for (i = 0; i < NSTEPS; i++) {
    sprintf(fname, FNAMEPATTERN, i);
    steppedVolData[i] = initVolume_xrw((char *)fname);
  }

  myVolData = steppedVolData[_step];



  float sx = 1.0;
  float sy = myVolData.wdy*(float)myVolData.Ny / (myVolData.wdx *(float)myVolData.Nx);
  float sz = myVolData.wdz*(float)myVolData.Nz / (myVolData.wdx *(float)myVolData.Nx);
  s2svp(-sx,sx, -sy,sy, -sz,sz);

  s2swin(0, myVolData.Nx, 0, myVolData.Ny, 0, myVolData.Nz);
  s2sci(S2_PG_WHITE);
  s2slw(1.0);
  s2box((char *)"BCDET",0,0,(char *)"BCDET",0,0,(char *)"BCDET",0,0);	/* Draw coordinate box */  

  cs2scb((void *)vr);

  initGL();
#if defined(USE_GLEW)
  if(glewIsSupported("GL_NVX_gpu_memory_info")) {
    int avaiMem = 0;
    glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &avaiMem);
    if(avaiMem > gMemRequired)
      bufferAllFlag = true;
  }
#endif

#if !defined(USE_S2_VR)
  cs2socb((void *)oglDraw);
#endif

}

#if (00)
void initGL() {
  fprintf(stderr, "in initGL\n");
#if defined(USE_GLEW)
  glewInit();
#endif
  glDisable(GL_CULL_FACE);
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glDepthFunc(GL_LEQUAL);
  glDepthMask(GL_TRUE);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  //density = 20.0;
  initShaders(&vrShaders, "raycasting");

  vp.resolution = make_float3(256);
  //    vp.resolution[0] = vp.resolution[1] = vp.resolution[2] = 256;
  vp.density = 1.0;
  vp.colourmap = 0;
  vp.brightness = 1.0;
  vp.contrast = 1.0;
  vp.power = 1.0;
  vp.samples = 256;
  vp.isovalue = 0.6;
  vp.isosmooth = 0.5;
  vp.isocolour[0] = .7; vp.isocolour[1] = .4; vp.isocolour[2] = .4; vp.isocolour[3] = .3;
  vp.drawWalls = 0;

  fprintf(stderr, "ex initGL\n");
}
#endif

void volumeRendering(int texid, int rgid, shaderService *ss, float3 box, AABB clip, float* viewport, volumeProperty _vp);

#if (11) // USE hdsupport.cpp version
// opengl callback (for shader...)
void oglDraw(int *eye) {
  if (!eye) {
    fprintf(stderr, "Comatose error: no eye specified for OpenGL callback\n");
    return;
  }

  fprintf(stderr, "oglDraw...\n");

  //Get which eye being rendered left/right
  char eyec = (char)*eye;
  int eyei = 0;
  if (eyec == 'l') eyei = -1;
  if (eyec == 'r') eyei = 1;
  
  //return;

  //Calculate viewport eye shift in pixels
  GLfloat viewport[4];
  glGetFloatv(GL_VIEWPORT, viewport);
  float eye_sep_ratio = camera.eyesep / camera.focallength;
  float eye_shift = eyei * eye_sep_ratio * viewport[3] * 0.6 / tan(camera.aperture * M_PI/180.0);
  //fprintf(stderr, "EYE %d CAMERA EYESEP RATIO %f SHIFT %f\n", eyei, eye_sep_ratio, eye_shift);
  viewport[0] += eye_shift;   //Horizontal shift for parallel camera stereo

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  float3 vpmin, vpmax;
  s2qvp(&(vpmin.x), &(vpmax.x), &(vpmin.y), &(vpmax.y), &(vpmin.z), &(vpmax.z));
  float3 box = (vpmax-vpmin);
  float3 scl = box / (swinMax - swinMin);
  float3 trns = (vpmax+vpmin)/2 - (swinMax+swinMin)/2;

  glTranslatef(trns.x, trns.y, trns.z);
  //glScalef(1./scl.x, 1./scl.y,1./scl.z);

  if (show_volume) {
    AABB clip = clpb;
    intersect(&clip, &aabb, &clpb);
    volumeRendering(myVolData.brainTex_id, -1, vrShaders, box, clip, viewport, vp);
  }

  glPopMatrix();
  fprintf(stderr, " <<< oglDraw\n");
}
#endif

void buildVolumeTex() {
  fprintf(stderr, "in buildVolumeTex\n");
  int xx = myVolData.Nx;
  int yy = myVolData.Ny;
  int zz = myVolData.Nz;
  if(myVolData.brainTex_id == 0) {
    glGenTextures(1, &(myVolData.brainTex_id));
  }
  glBindTexture(GL_TEXTURE_3D, myVolData.brainTex_id);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, xx, yy, zz, 0, GL_RED, GL_FLOAT, myVolData.brainTexData);
  //    GL_Error_Check;
  fprintf(stderr, "ex buildVolumeTex\n");
}

#if (11)
// vol rendering based on LavaVU::Volumes::render DGB 20160204
//void Volumes::render(int i)
void volumeRendering(int texid, int rgid, shaderService *ss, float3 box, AABB clip, float* vviewport, volumeProperty _vp)
{
  //float dims[3] = {geom[i]->vertices[1][0] - geom[i]->vertices[0][0],
  //                geom[i]->vertices[1][1] - geom[i]->vertices[0][1],
  //                 geom[i]->vertices[1][2] - geom[i]->vertices[0][2]
  //                };
  //float dims[3] = {_vp.resolution.x, _vp.resolution.y, _vp.resolution.z};

  // very very close, just gaps at rear of cube when strafing LR/UD
  //#define OMEGA_APERTURE 30.0
  //#define S2OMEGA_SCALE 1.0

  // OK IMPORTANT OBSERVATION: resize window to different aspect ratio and S2BOX is distorted.
  // SO S2PLOT is the problem, not the rendering. Need to call s2svp if Omega window changes size.
  // S2PLOT unaware of OMEGA display aspect ratio ... ?

  // AHEM: OMEGALIB does not support WINDOW RESIZE. Try ohello, resize the window change its aspect
  // and the cube stretches. Naughty. 

  #define OMEGA_APERTURE 31.5
  #define S2OMEGA_SCALE 1.02777

  float _sc = _s2om_scale;
  float3 dmin, dmax;
  s2qvp(&(dmin.x),&(dmax.x), &(dmin.y),&(dmax.y), &(dmin.z),&(dmax.z));
  float3 boxO = (dmax-dmin);
  float3 swinMin = {-1.,-1.,-1.};
  float3 swinMax = {1.,1.,1.};
  float3 scl = boxO / (swinMax-swinMin);
  
  float dims[3] = {2.*_sc*scl.x*S2OMEGA_SCALE, 2.*_sc*scl.y*S2OMEGA_SCALE, 2.*_sc*scl.z*S2OMEGA_SCALE};

  //assert(prog);
  //GL_Error_Check;
  assert(ss->prgObject);
  //GL_Error_Check;
  glUseProgram(ss->prgObject);
  //GL_Error_Check;

  //Uniform variables
  float viewport[4];
  glGetFloatv(GL_VIEWPORT, viewport);
  //TextureData* voltexture = geom[i]->texture;
  //if (!voltexture) voltexture = geom[i]->draw->defaultTexture;
  //if (!voltexture) abort_program("No volume texture loaded!\n");
  //float res[3] = {(float)voltexture->width, (float)voltexture->height, (float)voltexture->depth};
  float res[3] = {(float)myVolData.Nx-1, (float)myVolData.Ny-1, (float)myVolData.Nz-1};
  glUniform3fv(glGetUniformLocation(ss->prgObject, "uResolution"), 1, res);
  glUniform4fv(glGetUniformLocation(ss->prgObject, "uViewport"), 1, viewport);

  //User settings TO HERE 
  //json::Object props = geom[i]->draw->properties;
  //bool hasColourMap = geom[i]->draw->colourMaps[lucColourValueData]
  //                    && geom[i]->draw->colourMaps[lucColourValueData]
  //                    && props["colourmap"].ToBool(true);
  bool hasColourMap = false;

  //Use per-object clip box if set, otherwise use global clip
  //float bbMin[3] = {props["xmin"].ToFloat(Geometry::properties["xmin"].ToFloat(0.01)),
  //                  props["ymin"].ToFloat(Geometry::properties["ymin"].ToFloat(0.01)),
  //                  props["zmin"].ToFloat(Geometry::properties["zmin"].ToFloat(0.01))
  //                 };
  //float bbMax[3] = {props["xmax"].ToFloat(Geometry::properties["xmax"].ToFloat(0.99)),
  //                  props["ymax"].ToFloat(Geometry::properties["ymax"].ToFloat(0.99)),
  //                  props["zmax"].ToFloat(Geometry::properties["zmax"].ToFloat(0.99))
  //                 };
  float bbMin[3] = {0.01, 0.01, 0.01};
  float bbMax[3] = {0.99, 0.99, 0.99};

  glUniform3fv(glGetUniformLocation(ss->prgObject, "uBBMin"), 1, bbMin);
  glUniform3fv(glGetUniformLocation(ss->prgObject, "uBBMax"), 1, bbMax);
  glUniform1i(glGetUniformLocation(ss->prgObject, "uEnableColour"), hasColourMap ? 1 : 0);
  //glUniform1f(prog->uniforms[uPower"], props["power"].ToFloat(1.0));
  glUniform1f(glGetUniformLocation(ss->prgObject, "uPower"), 1.0);
  //glUniform1i(prog->uniforms["uSamples"], props["samples".ToInt(256));
  glUniform1i(glGetUniformLocation(ss->prgObject, "uSamples"), 256);
  //glUniform1f(prog->uniforms["uDensityFactor"], props["density"].ToFloat(5.0) * props["opacity"].ToFloat(1.0));
  glUniform1f(glGetUniformLocation(ss->prgObject, "uDensityFactor"), 0.5);
  //glUniform1f(prog->uniforms["uIsoValue"], props["isovalue"].ToFloat(0));
  glUniform1f(glGetUniformLocation(ss->prgObject, "uIsoValue"), 0.0);
  //Colour colour = Colour_FromJson(props, "colour", 220, 220, 200, 255);
  //colour.a = 255.0 * props["isoalpha"].ToFloat(colour.a/255.0);
  Colour colour = {220, 220, 200, 255};
  //Colour_SetUniform(glGetUniformLocation(ss->prgObject,"uIsoColour"), colour);
  //glUniform1f(prog->uniforms["uIsoSmooth"], props["isosmooth"].ToFloat(0.1));
  glUniform1f(glGetUniformLocation(ss->prgObject, "uIsoSmooth"), 0.1);
  //glUniform1i(prog->uniforms["uIsoWalls"], props["isowalls"].ToInt(0));
  glUniform1i(glGetUniformLocation(ss->prgObject, "uIsoWalls"), 0);
  //glUniform1i(prog->uniforms["uFilter"], props["tricubicfilter"].ToInt(0));
  glUniform1i(glGetUniformLocation(ss->prgObject, "uFilter"), 0);
  //density min max
  //float dminmax[2] = {props["dminclip"].ToFloat(0.0),
  //                    props["dmaxclip"].ToFloat(1.0)};
  float dminmax[2] = {0., 0.1};
  glUniform2fv(glGetUniformLocation(ss->prgObject, "uDenMinMax"), 1, dminmax);
  //GL_Error_Check;

  //float focalLength = 1.0 / tan(0.5 * camera.aperture * PI/180.0);;
  focalLength = 1.0 / tan(0.5 * OMEGA_APERTURE * PI/180.0);
  glUniform1f(glGetUniformLocation(ss->prgObject, "uFocalLength"), focalLength);

    glUniform1f(glGetUniformLocation(ss->prgObject, "uDensityFactor"), _vp.density);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uBrightness"), _vp.brightness);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uContrast"), _vp.contrast);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uPower"), _vp.power);
    glUniform1i(glGetUniformLocation(ss->prgObject, "uSamples"), _vp.samples);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uIsoValue"), _vp.isovalue);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uIsoSmooth"), _vp.isosmooth);
    glUniform4fv(glGetUniformLocation(ss->prgObject, "uIsoColour"), 1, _vp.isocolour);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uEnableColour"), 0);
    //glUniform1i(glGetUniformLocation(ss->prgObject, "uTransferFunction"), 1);
    glUniform1i(glGetUniformLocation(ss->prgObject, "uIsoWalls"), _vp.drawWalls);
    glUniform3fv(glGetUniformLocation(ss->prgObject, "uResolution"), 1, _vp.resolution.a);

  //Field data requires normalisation to [0,1]
  //Pass minimum,maximum in place of colourmap calibrate
  float range[2] = {0.0, 1.0};
  //if (geom[i]->colourData())
  //{
  //  range[0] = geom[i]->colourData()->minimum;
  //  range[1] = geom[i]->colourData()->maximum;
  //}
  glUniform2fv(glGetUniformLocation(ss->prgObject, "uRange"), 1, range);
  //GL_Error_Check;

  //Gradient texture
  //if (hasColourMap)
  //{
  //  glActiveTexture(GL_TEXTURE0);
  //  glUniform1i(prog->uniforms["uTransferFunction"], 0);
  //  glBindTexture(GL_TEXTURE_2D, geom[i]->draw->colourMaps[lucColourValueData]->texture->id);
  //}

  //Volume texture
  glActiveTexture(GL_TEXTURE1);
  //glBindTexture(GL_TEXTURE_3D, voltexture->id);
  glBindTexture(GL_TEXTURE_3D, texid);
  glUniform1i(glGetUniformLocation(ss->prgObject, "uVolume"), 1);
  //GL_Error_Check;

  //Get the matrices to send as uniform data
  float mvMatrix[16];
  float nMatrix[16];
  float pMatrix[16];
  float invPMatrix[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, nMatrix);
  //Apply scaling to fit bounding box (maps volume dimensions to [0,1] cube)
  glPushMatrix();
#ifndef USE_OMEGALIB
  //Get modelview without focal point / rotation centre adjustment
  //glLoadIdentity();
  //view->apply(false);
#endif
  //printf("DIMS: %f,%f,%f TRANS: %f,%f,%f SCALE: %f,%f,%f\n", dims[0], dims[1], dims[2], -dims[0]*0.5, -dims[1]*0.5, -dims[2]*0.5, 1.0/dims[0], 1.0/dims[1], 1.0/dims[2]);
  glTranslatef(-dims[0]*0.5, -dims[1]*0.5, -dims[2]*0.5);  //Translate to origin
  glScalef(1.0/dims[0], 1.0/dims[1], 1.0/dims[2]);
  //glScalef(1.0/(view->scale[0]*view->scale[0]), 1.0/(view->scale[1]*view->scale[1]), 1.0/(view->scale[2]*view->scale[2]));
  glGetFloatv(GL_MODELVIEW_MATRIX, mvMatrix);
  glPopMatrix();
  glGetFloatv(GL_PROJECTION_MATRIX, pMatrix);
  //if (!gluInvertMatrixf(pMatrix, invPMatrix)) abort_program("Uninvertable matrix!");
  //GL_Error_Check;

  //Projection and modelview matrices
  glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uPMatrix"), 1, GL_FALSE, pMatrix);
  glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uInvPMatrix"), 1, GL_FALSE, invPMatrix);
  glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uMVMatrix"), 1, GL_FALSE, mvMatrix);
  nMatrix[12] = nMatrix[13] = nMatrix[14] = 0; //Removing translation works as long as no non-uniform scaling
  glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uNMatrix"), 1, GL_FALSE, nMatrix);
  //GL_Error_Check;

  //State...
  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);  //No depth testing to allow multi-pass blend!
  glDisable(GL_MULTISAMPLE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  //glUseProgram(0);

  //Draw two triangles to fill screen
  glBegin(GL_TRIANGLES);
  #define MYONE 1.0
  glVertex2f(-MYONE, -MYONE);
  glVertex2f(-MYONE, MYONE);
  glVertex2f(MYONE, -MYONE);
  glVertex2f(-MYONE,  MYONE);
  glVertex2f(MYONE, MYONE);
  glVertex2f(MYONE, -MYONE);
  glEnd();

  
  glPopAttrib();
  //GL_Error_Check;
  glActiveTexture(GL_TEXTURE0);
}

#endif

// main S2PLOT dynamic callback - create the texture on first call, draw it
void vr(double *, int *) {

  steppedVolData[_step] = myVolData;
  _step = (_step + 1) % NSTEPS;
  myVolData = steppedVolData[_step];

#if !defined(USE_S2_VR)
  if (!myVolData.brainTex_id) {
    buildVolumeTex();
  }
#else
  if (myVolData.texid < 0) {
    myVolData.texid = ss2c3dt(myVolData.Nx, myVolData.Ny, myVolData.Nz);
    myVolData.refresh = 0;
  }
  if (myVolData.refresh) {
    cube2texture(&myVolData);
    myVolData.refresh = 0;
  }

  s2textureVolumeRender();
#endif

}

VolData initVolume_xrw(char *fname) {
  XRAW_STRUCT *xrw = loadXraw(fname);
  showXraw(xrw);
  int stride[] = {2, 3, 2};
  //int stride[] = {1,1,1};
  VOL_STRUCT *vol = Xraw2Xvol(xrw, stride);
  showXvol(vol);
  //derivXvol(vol);
  //tightenXvol(vol, 1.0, 3.0);
  VolData st;
  st.Nx = vol->nx;
  st.Ny = vol->ny;
  st.Nz = vol->nz;
  
  st.wdx = vol->wdx;
  st.wdy = vol->wdy;
  st.wdz = vol->wdz;

  st.array = vol->data;

  // default data mapping settings (data -> texture)
  st.dmin = 0.0;
  st.dmax = 1.0;
  st.data_power = -1.0;
  st.amin = 0.0;
  st.amax = 0.2;
  st.cmapidx = 1;
  st.c1 = 1000;
  st.c2 = 1255;
  st.valpha = 0.5;

  st.texid = -1;

  st.brainTex_id = 0;
  st.brainTexData = NULL;
  volToTexture(&st);

  st.refresh = 1;
  st.quality = 3;

  return st;
}

void cube2texture(VolData *vd) {
  
  static int nmaps=8;
  static char *maps[] = {"rainbow", "iron", "astro", "gray", 
			 "inverse rainbow", "inverse iron", "inverse astro", "inverse gray"};

  float dmin = vd->dmin;
  float dmax = vd->dmax;
  float data_power = vd->data_power;
  if (data_power < 1) {
    data_power = 1. / (-data_power + 2);
  }
  fprintf(stderr, "data_power = %f\n", data_power);
  // data_power is now like 1/5,1/4,1/3,1/2,1,2,3,4,5 ...

  float amin = vd->amin;
  float amax = vd->amax;

  int c1 = vd->c1;
  int c2 = vd->c2;
  s2scir(c1, c2);// Set the range of colour indices used for shading
  s2icm(maps[vd->cmapidx % nmaps], c1, c2); // Install colour maps

  int w,h,d, i, j, k;
  unsigned char *bits = (unsigned char *)ss2g3dt(vd->texid, &w, &h, &d);
  //memset(bits, (unsigned char)0, w*h*d*4);
  float r,g,b;
  long t_idx;
  unsigned int x;
  float xf, op;
  float scale  = ((c2-c1))/(dmax-dmin);
  int cd = (int)(c2-c1);
  float denom_recip = 1.0 / (dmax - dmin);

  fprintf(stderr, "texture w,h,d = %d,%d,%d\n", w, h, d);
  fprintf(stderr, "data Nx,Ny,Nz = %d,%d,%d\n", vd->Nx, vd->Ny, vd->Nz);


  for (i=0;i<vd->Nx;i++) {
    for (j=0;j<vd->Ny;j++) {
#pragma omp parallel for private(t_idx,xf,x,r,g,b,op)
      for (k=0;k<vd->Nz;k++) {
	t_idx = (long)(k * vd->Ny) * (long)(vd->Nx) + (long)(j * vd->Nx) + (long)i;

	// fraction of range
	xf = (vd->array[i][j][k] - dmin) * denom_recip;
	xf = (xf < 0.) ? 0. : ((xf > 1.) ? 1. : xf);
	
	// apply power scaling to xf and to op
	xf = powf(xf, data_power);

	x = (int)((float)xf * (float)(c2-c1));
	if (x < 0) { x = 0; }
	if (x > (c2-c1)) { x = c2-c1; }

	s2qcr(c1+x, &r, &g, &b);
	bits[t_idx*4 + 0] = (unsigned char)(r * 255);
	bits[t_idx*4 + 1] = (unsigned char)(g * 255);
	bits[t_idx*4 + 2] = (unsigned char)(b * 255);
	
	op = amin + (amax - amin) * xf;
	if (vd->array[i][j][k] < dmin) {
	  op = 0.;
	}
	bits[t_idx*4 + 3] = (unsigned char)(op * 255);

      }
    }
  }
  
  ss2ptt(vd->texid);  // Reinstall a texture
}


void volToTexture(VolData *vd) {
  fprintf(stderr, "in volToTexture\n");
  long j, k, l;
  long xx = vd->Nx;
  long yy = vd->Ny;
  long zz = vd->Nz;
  long texSize = xx * yy * zz;
  vd->brainTexData = (GLfloat*)malloc(texSize * sizeof(GLfloat));
  
#pragma omp parallel for private(k,l,voxid)
  for(j = 0; j < xx; j++) {
    for(k = 0; k < yy; k++) {
      for(l = 0; l < zz; l++) {
	long voxid = l*xx*yy + k*xx + j;
	vd->brainTexData[voxid] = vd->array[j][k][l];
      }
    }
  }
  fprintf(stderr, "ex volToTexture\n");
}


// 3d texture volume render
void s2textureVolumeRender(void) {

  // TODO should read this from *.xrw.qmat or smat
  float tr[] = {0., 1., 0., 0.,
		0., 0., 1., 0.,
		0., 0., 0., 1.}; // (default diagonal to start with)
  
  int adim = myVolData.Nx;
  int bdim = myVolData.Ny;
  int cdim = myVolData.Nz;
  int a1 = 0;
  int a2 = adim-1;
  int b1 = 0;
  int b2 = bdim-1;
  int c1 = 0;
  int c2 = cdim-1;
  
  char itrans = 's';


  // these are the vertices of a unit cube
  XYZ unitverts[] = {{0, 0, 0},
		     {1, 0, 0},
		     {0, 1, 0},
		     {1, 1, 0},
		     {0, 0, 1},
		     {1, 0, 1},
		     {0, 1, 1},
		     {1, 1, 1}};
  // the vertices of the data being displayed
  XYZ dataverts[8];
  // the world vertices of the data being displayed (= dataverts * tr)
  XYZ worldverts[8];
  // these are the edges of the cube, made up by joining these unitverts:
  int edges[12][2] = {{0,1}, {0,2}, {1,3}, {2,3},
		      {4,5}, {4,6}, {5,7}, {6,7},
		      {0,4}, {1,5}, {2,6}, {3,7}};


  // 1. get camera position and view direction in world coords
  XYZ campos, upvec, viewdir, right;
  ss2qc(&campos, &upvec, &viewdir, 1);
  Normalise(&viewdir);
  right = CrossProduct(viewdir, upvec);
  
  // override up and right vectors direct from modelview.
  XYZ thisRT, nthisRT, thisUP, nthisUP;
  XYZ nthisVD;
  
  // fetch RT and UP vectors directly from Modelview matrix
  float mvm[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, mvm);
  thisRT.x = mvm[0];
  thisRT.y = mvm[4];
  thisRT.z = mvm[8];
  nthisRT = thisRT;
  Normalise(&nthisRT);
  thisUP.x = mvm[1];
  thisUP.y = mvm[5];
  thisUP.z = mvm[9];
  nthisUP = thisUP;
  Normalise(&nthisUP);
  
  //nthisVD = CrossProduct(nthisRT, nthisUP);
  // SURELY the above is WRONG WRONG WRONG in a RHS 
  nthisVD = CrossProduct(nthisUP, nthisRT);
  
  right = nthisRT;
  upvec = nthisUP;
  viewdir = nthisVD;
  
  
  // 2. find indices of first and last vertices: first is that vertex
  //    which a plane normal to the viewdir crosses, travelling towards
  //    the centre of the cube, from the camera position.
  int i;
  int near_vtx, far_vtx;
  float near_dist, far_dist;
  XYZ tmp;
  float thisdist;
  near_vtx = far_vtx = -1;
  near_dist = 9e30;
  far_dist = -9e30;
  for (i = 0; i < 8; i++) {
    // calculate this data vertex
    dataverts[i].x = a1 + unitverts[i].x * (a2-a1);
    dataverts[i].y = b1 + unitverts[i].y * (b2-b1);
    dataverts[i].z = c1 + unitverts[i].z * (c2-c1);
    // and this world vertex position
    worldverts[i].x = tr[0] + tr[1] * dataverts[i].x
      + tr[2] * dataverts[i].y + tr[3] * dataverts[i].z;
    worldverts[i].y = tr[4] + tr[5] * dataverts[i].x
      + tr[6] * dataverts[i].y + tr[7] * dataverts[i].z;
    worldverts[i].z = tr[8] + tr[9] * dataverts[i].x
      + tr[10] * dataverts[i].y + tr[11] * dataverts[i].z;
    // and now its distance from the camera position measured along the
    // view direction
    //--tmp = VectorSub(campos, worldverts[i]);
    //--thisdist = DotProduct(tmp, viewdir);
    thisdist = DotProduct(worldverts[i], viewdir);
    if (thisdist < near_dist) {
      near_vtx = i;
      near_dist = thisdist;
    }
    if (thisdist > far_dist) {
      far_vtx = i;
      far_dist = thisdist;
    }
  }

  // 3. step from near distance to far distance, and calculate the
  //    bounds of each polygon slice (intersection of cube and plane).
  XYZ p1, p2;
  int plidx; // plane index
  float fracdist; // 0 to 1 (near to far)
  XYZ pip; // point-in-plane
  XYZ pipvd; // point-in-plane, but along viewdir: should be centred!
  PLANE theplane;
  double mu;
  XYZ pt, pt2;
  // and here we place up to 6 vertices for a sliced polygon
  int npolyverts;
  int polyverts[6]; // which edge?
  float polyfracs[6]; // how far along edge?
  // and this is the position angle of the vertex in the viewplane
  float polyangs[6];
  int j,k;
  float ang;
  float xpts[7], ypts[7], zpts[7];
  XYZ iP[6], iTC[6];
  int NPL; // the number of planes we will draw
  // now scale NPL by dot product of (nearvtx - farvtx) . viewdir
  // because this says what is the "depth" of planes...
  pt2 = VectorSub(worldverts[near_vtx], worldverts[far_vtx]);
  ang = DotProduct(pt2, viewdir);
  pt2 = VectorSub(worldverts[0], worldverts[7]); // diagonal
  ang /= Modulus(pt2);
  NPL = (int)(2.0 * ang * sqrt(myVolData.Nx*myVolData.Nx+myVolData.Ny*myVolData.Ny+myVolData.Nz*myVolData.Nz) );
  NPL /= 2;

  NPL /= myVolData.quality;

  // loop in reverse order so farthest planes added to list (and then
  // drawn) first.
  for (plidx = NPL; plidx > 0; plidx--) {
    //for (plidx = 0; plidx < NPL; plidx++) {
    fracdist = (float)plidx / (float)(NPL+1);
    // point-in-plane along near_vtx to far_vtx line
    pip = VectorSub(worldverts[near_vtx], worldverts[far_vtx]);
    pip = VectorMul(pip, fracdist);
    pip = VectorAdd(worldverts[near_vtx], pip);
    //COLOUR COLa = {0.5, 0.5, (float)plidx/(float)NPL};
    //ns2vsphere(pip, 4.1, COLa);
    
    // plane equation: for n={a,b,c}, the plane is n.p=-d, giving
    // ax+by+cz+d = 0
    // So all we do is calculate what d is:
    theplane.a = viewdir.x;
    theplane.b = viewdir.y;
    theplane.c = viewdir.z;
    theplane.d = -1. * DotProduct(viewdir, pip);
    // point-in-plane along viewdir
    p2 = VectorAdd(campos, viewdir);
    if (!LinePlane(campos, p2, theplane, &mu, &pipvd)) {
      fprintf(stderr, "Viewdir doesn't intersect plane: impossible!!!\n");
      exit(-1);
    }
    //-COLOUR COLb = {0.2, 0.7, 0.8};
    //-ns2vsphere(pipvd, 0.01, COLb);
    
    npolyverts = 0;
    for (i = 0; i < 12; i++) {
      p1 = worldverts[edges[i][0]];
      p2 = worldverts[edges[i][1]];
      if (LinePlane(p1, p2, theplane, &mu, &pt)) {
	if ((mu >= 0) && (mu <= 1.)) {
	  // get position angle of vertex
	  //--pt2 = VectorSub(pipvd, pt);
	  pt2 = VectorSub(pip, pt);
	  Normalise(&pt2);
	  ang = atan2(DotProduct(pt2, upvec), DotProduct(pt2, right));
	  // and insert in list
	  j = 0;
	  while ((j < npolyverts) && (polyangs[j] < ang)) {
	    j++;
	  }
	  k = npolyverts - 1;
	  while (k >= j) {
	    polyverts[k+1] = polyverts[k];
	    polyfracs[k+1] = polyfracs[k];
	    polyangs[k+1] = polyangs[k];
	    k--;
	  }
	  k++;
	  polyverts[k] = i;
	  polyfracs[k] = mu;
	  polyangs[k] = ang;
	  npolyverts++;
	}
      }
    }
    // ok, we have the edges, fraction along those edges, and the
    // position angle in the view plane of each vertex of this poylgon.
    // Now we need to draw the polygon in eg. clockwise order...
    for (i = 0; i < npolyverts; i++) {
      p1 = worldverts[edges[polyverts[i]][0]];
      p2 = worldverts[edges[polyverts[i]][1]];
      mu = polyfracs[i];
      pt = VectorAdd(p1, VectorMul(VectorSub(p1, p2), mu));
      xpts[i] = pt.x;
      ypts[i] = pt.y;
      zpts[i] = pt.z;
      if (i == 0) {
	xpts[npolyverts] = pt.x;
	ypts[npolyverts] = pt.y;
	zpts[npolyverts] = pt.z;
      }
      // here are the XYZ arrays for 3d texturing via ns2texpoly3d...
      iP[i] = pt;
      p1 = unitverts[edges[polyverts[i]][0]];
      p2 = unitverts[edges[polyverts[i]][1]];
      iTC[i] = VectorAdd(p1, VectorMul(VectorSub(p1, p2), mu));
    }
    if (0 || (ss2qrm() == WIREFRAME)) {
      s2sci(2 + (plidx % 12));
      s2line(npolyverts+1, xpts, ypts, zpts);
    } else {
      ns2texpoly3d(iP, iTC, npolyverts, myVolData.texid, itrans, myVolData.valpha);
    }
  }
}
