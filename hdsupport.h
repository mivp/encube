/*
 * hdsupport.h
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

#if !defined(_HDSUPPORT_H)
#define _HDSUPPORT_H

#include "json.h"
extern "C" {
#include "libxrw.h"
}
#include "dirent.h"
#include "float16.h"
#include "shaderService.h"
#include "routines.h"
#include "utility.h"
#include "png_util.h"
#include <GL/glew.h>
//#include <GL/glxew.h>
#include <string>
#include <vector>
#include <assert.h>
//#include <omp.h>

#include <time.h>

#include <float.h>
extern "C" {
#include "s2plot.h"
#include "s2plot_glodef.h"
#include "geomviewer.h"
}

using std::string;
using std::vector;

//****************************************************
// Global variables
//****************************************************

#define GL_Error_Check \
  { \
    GLenum error = GL_NO_ERROR; \
    while ((error = glGetError()) != GL_NO_ERROR) { \
      fprintf(stderr, "OpenGL error [ %s : %d ] \"%s\".\n",  \
            __FILE__, __LINE__, gluErrorString(error)); \
    } \
  }

#define _LAPTOP_
//#define _CAVE2_

#ifdef _LAPTOP_
#   define MEMSIZE 512
#endif
#ifdef _CAVE2_
#   define MEMSIZE 4096
#endif

#define _HALF_POS_
#ifdef _HALF_POS_
typedef Float16 HDreal;
#define _GL_FLOAT_TYPE_ GL_HALF_FLOAT
//..int gMemRequired = 3.7 * 1024 * 1024;
#else
typedef GLfloat HDreal;
#define _GL_FLOAT_TYPE_ GL_FLOAT
//..int gMemRequired = 6.2 * 1024 * 1024;
#endif

#define GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX          0x9047
#define GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    0x9048
#define GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  0x9049
#define GPU_MEMORY_INFO_EVICTION_COUNT_NVX            0x904A
#define GPU_MEMORY_INFO_EVICTED_MEMORY_NVX            0x904B

#define MAXSTRING 256
#define MAXPANEL 4

// configuration
typedef enum { APP_BRAIN_XRW = 0, APP_BRAIN_NII, APP_ASTRO_FITS } AppType;
typedef enum { DESKTOP = 0, CAVE2 } HDPLATFORM;
typedef enum { DEC_PERTRACK = 0, DEC_PERSEGMENT } DecType;
typedef enum { DEC_ABSOLUTE = 0, DEC_NO_SYM, DEC_ROTATIONAL_SYM, DEC_MIRROR_SYM } DecColor;
typedef struct {
    HDPLATFORM platform;
    AppType app_type;
    string data_dir;
    string mask_dir;
    bool show_volume;
    bool show_track;
    bool use_processed_track;
    bool save_time;
    string time_log_dir_filename;
    string framerate_log_dir_filename;
    DecType dec_type;
    DecColor dec_color;
} HDCONFIG;

typedef enum { EMPTY, LOAD, WAIT, PREDRAW, DRAW, REDRAW } action;
typedef struct {
    int pid;        /* Panel id - from xs2ap */
    action state;

    int id; // study (imagehd) subject id
    char type; // study (imagehd) subject type code e.g. 'P' presymp

    char TRKfilename[MAXSTRING];
    char NIIfilename[MAXSTRING];
    char FITS_filename[MAXSTRING];
    int ntracks;
    int ntracks_drawn;
    int *tracklens;
    int npoints;
    float *points;
    int nlines;
    HDreal *lineVerts, *lineVertsAdj;
    GLuint lineVertsVBOid;

    GLubyte *trackFlags, *trackFlagsAdj;
    GLuint trackFlagsVBOid;
    GLubyte *trackColor, *trackColorAdj;
    GLuint trackColorVBOid;
    float ***array;
    long /*float*/ Nx, Ny, Nz;

    GLuint _geometryTex_id;
    GLfloat *_geometryTex_data;
    unsigned int infotexid; // texture id for info string texture
    float infoaspect; // aspect ratio of info string texture

    float dmin, dmax; // data cube's range : dmin and dmax
    int nbins; //Histogram's number of bins
    float* histogram;
    float mean, stand_dev, median;

    int selected;
    float min_filter;
    float max_filter;
    bool mip;
    bool moment;

    float **mom0;
    float **mom1;
    float **mom2;

//    time_t start_t, stop_t;
//    double diff_t;

    timeval start_time;
    timeval stop_time;
    double elapsed_time;

} PanelData;

#define PLANEUNIT 1.0
//const float testPlaneSize = 0.5;

typedef struct
{
    float3 resolution;
    float density;
    int enable_colour;
    int colourmap;
    float brightness;
    float contrast;
    float power;
    int samples;
    float isovalue;
    float isosmooth;
    float isocolour[4];
    int drawWalls;

    // Astro related.
    int show_moment;
    int mip;
    int moment;
} volumeProperty;

#define panel1 0
#define panel2 1
#define panel3 2
#define panel4 3

//#define FONT "./Digital Readout Thick Upright.ttf"
//#define FONT "/Library/Fonts/Courier New Bold.ttf"
#define FONT "courier_new_bold.ttf"

#define DMIN 0
#define DMAX 1

//#define CBASE 100
#define CBASE 0
#define CRANGE 256

typedef struct
{
    int num_masks;
    VOL_STRUCT **mask_xvols;
    GLfloat **mask_vols;	// masks' volume data (also for 3d texture)
    int *mask_isos;
    int nx;
    int ny;
    int nz;
    int act_mask1;
    int act_mask2;
    float maskThresh;
    GLuint rg1Tex_id;
    GLuint rg2Tex_id;
    bool loadFlags[2];
    bool drawFlags[2];
    GLfloat regionColour[8];
    volumeProperty vp;
    AABB *aabb;
} RegionControl;


//----------------------------------------------------
// Methods definition list
//----------------------------------------------------
void initShaders(shaderService **ss, char *filename, int gs);
void volToTexture(int id);
XYZ normalizeS2volVert(XYZ v, XYZ size, float s);
void normalizeS2volVertf(float ov[3], float v[3], float size[3], float s);
void denormalizeS2volVertf(float ov[3], float v[3], float size[3], float s);
void volumeRendering(int texid, int rgid, shaderService *ss, float3 box, AABB clip, float* viewport, volumeProperty _vp);
void volumeRendering(int pid, int texid, int rgid, shaderService *ss, float3 box, AABB clip, float* viewport, volumeProperty _vp);
void drawTracksByShader(int pid);
void updateTrackFlags(int pid);
void updateTracksByRemoteControl();
void hsv2rgb(const float3 &hsv, float3 &rgb);
void computeDECColor(float3& dir, float3& rgb);
void computeDEC(int pid);
void oglDraw(int *eye);
void bindVertexPosBuffer(int pid, int num);
void bindTrackFlagBuffer(int pid, int num);
void bindTrackColorBuffer(int pid, int num);
void evaluate_histogram(int pid, bool normalise);
void evaluate_moment_maps(int pid);
int send_moment_maps(int id, int socket);
void calculateFPS();

// The following didn't have definition in cavehd.c
void testJson();
void loadConfig(const char* filename);
void loadTracks(int id);
void loadBrain(int id);
void loadData(int pid);
void freeData(int id);

void zeroPanel(int id);
void clearPanel(int id);
void queryGPUInfo();
Colour Colour_FromJson(json::Object& object, std::string key, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
static void multMatrix(float dst[16], const float src1[16], const float src2[16]);
void computeSphericalCoordv(float3 &sc, const float3 &vec);
void initRegionControl();
void initMasks(void);
void maskXvolToVol(int rgid, int mask_id);
void bindRegionTex(int actrgid, int mid);
int getMaskid(int medicineid);
void loadMask(int i, int roinum);
float ***initVolume_fits(int id);
float ***initVolume_xrw(int flag, float min, float max, int id);
void initVolume_nii(int flag, float min, float max, int id);
int remote_cb(char *data);
int remote_cb_sock(char *data, FILE* sockout);
//int remote_cb_sock_write(char *data, int socket);
int kbdcb(unsigned char *key);
void prepit(int id);
void buildVolumeTex(int pid);
void buildBufferAndTexture(int pid);
void initGL();
void initTrackFraction();
//----------------------------------------------------
// End of methods definition list
//----------------------------------------------------



#endif
