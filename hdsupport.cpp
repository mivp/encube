/*
 * hdsupport.cpp
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

#include "hdsupport.h"

/* define globals */
#ifdef _HALF_POS_
int gMemRequired = 3.7 * 1024 * 1024;
#else
int gMemRequired = 6.2 * 1024 * 1024;
#endif

// swin coordinates range
float3 swinMin = make_float3(-1);
float3 swinMax = make_float3(1);

HDCONFIG config; //shared between panels
// for brain apps speed up, no need to reload mean brain
bool need_reload_brain = true;
bool brain_loaded[MAXPANEL] = {false, false, false, false};

// combined masks
XRAW_STRUCT *mask_all_xrw = NULL;
XRAW_STRUCT *mask_temp_xrw = NULL;
vector<int> mask_ids;

int update = FALSE;

char TRKfilename[MAXPANEL*2][MAXSTRING];
char NIIfilename[MAXPANEL*2][MAXSTRING];
char FITS_filename[MAXPANEL*2][MAXSTRING];
char inputextfile[MAXSTRING];
PanelData pd[MAXPANEL]; // array of 4 panelData variables
int updateflag = 0;

char APPLABEL[MAXSTRING] = "...";

int _n_cmaps = 5;
int _i_cmap = 0;
char *_cmaps[] = {"grey", "hot", "astro", "mgreen", "alt"};
float _core_alpha = 0.1;
float _track_alpha = 0.95;

bool bufferAllFlag = true; //false;

float lFraction = 0.04;   // _LAPTOP_ load fraction for tracks
float sFraction = 0.04;    // _CAVE2_ sample fraction for tracks

float ps = 0.5f;

GLfloat scnTriPosArr[] =
        {
                -PLANEUNIT, -PLANEUNIT, .0f,
                -PLANEUNIT, PLANEUNIT, .0f,
                PLANEUNIT, -PLANEUNIT, .0f,
                PLANEUNIT, -PLANEUNIT, .0f,
                -PLANEUNIT, PLANEUNIT, .0f,
                PLANEUNIT, PLANEUNIT, .0f
        };

// for shader rendering
shaderService *vrShaders = NULL;
shaderService *trShaders = NULL;
shaderService *rgShaders = NULL;
shaderService *mmShaders = NULL;

int VBOdraw = 1;
int trackUpdateByRemoteFlag = 0;

float volumeScale = 2.0f;

// parameters from web
AABB clpb = make_aabb(make_float3(0), make_float3(1));
AABB aabb = clpb;
float viewport_width, viewport_height;

volumeProperty vp;
// END: parameters from web

COLOUR C_GREEN = {0.3, 1.0, 0.3};
COLOUR C_BLUE  = {0.3, 0.3, 1.0};
COLOUR C_ORANGE = {1.0, 0.6, 0.2};
COLOUR C_GREY = {0.2, 0.2, 0.5};

int master = 0;

int nt;

int camset = 0;
XYZ setpos, setup, setvdir;

RegionControl rgc;

float line_thick = 2.0;

int _doFrames = 0;
int DIVO = 4096;

int CURRENT_TIME = 0, PREVIOUS_TIME = 0;

bool autospin = false;

//
int flag = -1;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Methods
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//void calculateFPS()
//{
//    //  Increase frame count
//    frameCount++;
//
//    //  Get the number of milliseconds since glutInit called
//    //  (or first call to glutGet(GLUT ELAPSED TIME)).
//    CURRENT_TIME = glutGet(GLUT_ELAPSED_TIME);
//
//    //  Calculate time passed
//    int timeInterval = CURRENT_TIME - PREVIOUS_TIME;
//
//    if(timeInterval > 1000)
//    {
//        //  calculate the number of frames per second
//        fps = frameCount / (timeInterval / 1000.0f);
//
//        //  Set time
//        PREVIOUS_TIME = CURRENT_TIME;
//
//        //  Reset frame count
//        frameCount = 0;
//    }
//}

void initShaders(shaderService **ss, char *filename, int gs)
{
    if(*ss != NULL) destroyShaderService(*ss);
    *ss = (shaderService*)malloc(sizeof(shaderService));
    initShaderService(*ss, filename, gs, 1);
}

void volToTexture(int id)
{
    int xx = pd[id].Nx, yy = pd[id].Ny, zz = pd[id].Nz;
    int texSize = xx * yy * zz;
    pd[id]._geometryTex_data = (GLfloat*)malloc(texSize * sizeof(GLfloat));

    int mini, minj, mink, maxi, maxj, maxk;
    mini = minj = mink = 255;
    maxi = maxj = maxk = 0;
    int voxid = 0;
    for(int k = 0; k < zz; k++)
    {
        for(int j = 0; j < yy; j++)
        {
            for(int i = 0; i < xx; i++)
            {
                pd[id]._geometryTex_data[voxid] = pd[id].array[i][j][k];

                if(pd[id]._geometryTex_data[voxid] > 0 )
                {
                    if(i < mini) mini = i;
                    if(j < minj) minj = j;
                    if(k < mink) mink = k;

                    if(i > maxi) maxi = i;
                    if(j > maxj) maxj = j;
                    if(k > maxk) maxk = k;
                }
                voxid++;
            }
        }
    }

    aabb = make_aabb(make_float3(mini, minj, mink) / (vp.resolution - 1), (make_float3(maxi, maxj, maxk)+1) / (vp.resolution - 1));

    /*/Dump raw volume data
    FILE* fp = fopen("MeanImage.raw", "wb");
    if (fp)
    {
      fwrite(pd[id]._geometryTex_data, texSize, sizeof(GLfloat), fp);
      fclose(fp);
    }*/
}

XYZ normalizeS2volVert(XYZ v, XYZ size, float s)
{
    XYZ v0;
    v0.x = (v.x / size.x - 0.5) * s;
    v0.y = (v.y / size.y - 0.5) * s;
    v0.z = (v.z / size.z - 0.5) * s;
    return v0;
}

void normalizeS2volVertf(float ov[3], float v[3], float size[3], float s)
{
    for(int i = 0; i < 3; i++)
        ov[i] = (v[i] / size[i] - 0.5) * s;
}

void denormalizeS2volVertf(float ov[3], float v[3], float size[3], float s)
{
    for(int i = 0; i < 3; i++)
        ov[i] = (v[i] / s + 0.5) * size[i];
}

#if !defined(ENCUBE_OMEGA)
void volumeRendering(int texid, int rgid, shaderService *ss, float3 box, AABB clip, float* viewport, volumeProperty _vp)
{
    //Blending required for correct intensity output
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(ss->prgObject);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texid);
    glUniform1i(glGetUniformLocation(ss->prgObject, "uVolume"), 0);

    //Bounds for clipping the normalised coord volume bounding box [0,1]
    glUniform3fv(glGetUniformLocation(ss->prgObject, "uBBMin"), 1, clip.min.a);
    glUniform3fv(glGetUniformLocation(ss->prgObject, "uBBMax"), 1, clip.max.a);

    float focalLength = 1.0 / tan(0.5 * ss2qca() * PI/180.0);;
    glUniform1f(glGetUniformLocation(ss->prgObject, "uFocalLength"), focalLength);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uDensityFactor"), _vp.density);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uBrightness"), _vp.brightness);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uContrast"), _vp.contrast);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uPower"), _vp.power);
    glUniform1i(glGetUniformLocation(ss->prgObject, "uSamples"), _vp.samples);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uIsoValue"), _vp.isovalue);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uIsoSmooth"), _vp.isosmooth);
    glUniform4fv(glGetUniformLocation(ss->prgObject, "uIsoColour"), 1, _vp.isocolour);

    // Test by DV for range threshold in pixel intensity
    glUniform1f(glGetUniformLocation(ss->prgObject, "uMin"), -1);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uMax"), -1);

    //For colour maps, not used currently
    glUniform1f(glGetUniformLocation(ss->prgObject, "uEnableColour"), _vp.enable_colour);
    glUniform1i(glGetUniformLocation(ss->prgObject, "uTransferFunction"), _vp.colourmap);

    glUniform4fv(glGetUniformLocation(ss->prgObject, "uViewport"), 1, viewport);
    glUniform1i(glGetUniformLocation(ss->prgObject, "uIsoWalls"), _vp.drawWalls);
    glUniform3fv(glGetUniformLocation(ss->prgObject, "uResolution"), 1, _vp.resolution.a);

    glPushMatrix();  //Following changes to modelview for volume ray cast only
    //Apply translation and scaling for actual bounding box to normalised coords [0,1] used in volume renderer
    glTranslatef(-box.x*0.5, -box.y*0.5, -box.z*0.5);  //Translate to origin
    glScalef(1.0/box.x, 1.0/box.y, 1.0/box.z);         //Scale to normalised cube
    float matrix[16], projection[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uMVMatrix"), 1, GL_FALSE, matrix);
    matrix[12] = matrix[13] = matrix[14] = 0;
    glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uNMatrix"), 1, GL_FALSE, matrix);
    glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uPMatrix"), 1, GL_FALSE, projection);

    if(rgid > -1) glUniform4fv(glGetUniformLocation(ss->prgObject, "uVolumeColour"), 1, &(rgc.regionColour[(rgid)*4]));
    else glUniform4f(glGetUniformLocation(ss->prgObject, "uVolumeColour"), 1.0, 1.0, 1.0, 1.0 );

    //Draw two triangles to fill screen
    glBegin(GL_TRIANGLES);
    glVertex2f(-1, -1); glVertex2f(-1, 1); glVertex2f(1, -1);
    glVertex2f(-1,  1); glVertex2f( 1, 1); glVertex2f(1, -1);
    glEnd();

    glUseProgram(0);

    glPopMatrix();  //Restore original modelview for drawing rest of scene
    glEnable(GL_DEPTH_TEST);
}

void volumeRendering(int pid, int texid, int rgid, shaderService *ss, float3 box, AABB clip, float* viewport, volumeProperty _vp)
{
    //Blending required for correct intensity output
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(ss->prgObject);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texid);
    glUniform1i(glGetUniformLocation(ss->prgObject, "uVolume"), 0);

    //Bounds for clipping the normalised coord volume bounding box [0,1]
    glUniform3fv(glGetUniformLocation(ss->prgObject, "uBBMin"), 1, clip.min.a);
    glUniform3fv(glGetUniformLocation(ss->prgObject, "uBBMax"), 1, clip.max.a);

    float focalLength = 1.0 / tan(0.5 * ss2qca() * PI/180.0);;
    glUniform1f(glGetUniformLocation(ss->prgObject, "uFocalLength"), focalLength);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uDensityFactor"), _vp.density);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uBrightness"), _vp.brightness);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uContrast"), _vp.contrast);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uPower"), _vp.power);
    glUniform1i(glGetUniformLocation(ss->prgObject, "uSamples"), _vp.samples);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uIsoValue"), _vp.isovalue);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uIsoSmooth"), _vp.isosmooth);
    glUniform4fv(glGetUniformLocation(ss->prgObject, "uIsoColour"), 1, _vp.isocolour);

    // Test by DV for range threshold in pixel intensity
    if (pd[pid].min_filter == -1)
        glUniform1f(glGetUniformLocation(ss->prgObject, "uMin"), pd[pid].mean-(3*pd[pid].stand_dev));
    else
        glUniform1f(glGetUniformLocation(ss->prgObject, "uMin"), pd[pid].min_filter);

    if (pd[pid].max_filter == -1)
        glUniform1f(glGetUniformLocation(ss->prgObject, "uMax"), pd[pid].mean+(3*pd[pid].stand_dev));
    else
        glUniform1f(glGetUniformLocation(ss->prgObject, "uMax"), pd[pid].max_filter);

    //For colour maps, not used currently
    //glUniform1f(glGetUniformLocation(ss->prgObject, "uEnableColour"), 0);
    //glUniform1i(glGetUniformLocation(ss->prgObject, "uTransferFunction"), 1);

    //fprintf(stderr, "Enable_colour: %d\n", _vp.enable_colour);

    if (!_vp.enable_colour)
    {
        glUniform1f(glGetUniformLocation(ss->prgObject, "uEnableColour"), 0);
        glUniform1i(glGetUniformLocation(ss->prgObject, "uTransferFunction"), 1);
    }
    else
    {
        glUniform1f(glGetUniformLocation(ss->prgObject, "uEnableColour"), _vp.enable_colour);

        // The following information should come from server (currently for testing)
        //GLuint tex = loadColourmapTexture("colourmap.png");
        glUniform1i(glGetUniformLocation(ss->prgObject, "uTransferFunction"), 1);
    }

    glUniform4fv(glGetUniformLocation(ss->prgObject, "uViewport"), 1, viewport);
    glUniform1i(glGetUniformLocation(ss->prgObject, "uIsoWalls"), _vp.drawWalls);
    glUniform3fv(glGetUniformLocation(ss->prgObject, "uResolution"), 1, _vp.resolution.a);

    glPushMatrix();  //Following changes to modelview for volume ray cast only
    //Apply translation and scaling for actual bounding box to normalised coords [0,1] used in volume renderer
    glTranslatef(-box.x*0.5, -box.y*0.5, -box.z*0.5);  //Translate to origin
    glScalef(1.0/box.x, 1.0/box.y, 1.0/box.z);         //Scale to normalised cube
    float matrix[16], projection[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uMVMatrix"), 1, GL_FALSE, matrix);
    matrix[12] = matrix[13] = matrix[14] = 0;
    glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uNMatrix"), 1, GL_FALSE, matrix);
    glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uPMatrix"), 1, GL_FALSE, projection);

    if(rgid > -1) glUniform4fv(glGetUniformLocation(ss->prgObject, "uVolumeColour"), 1, &(rgc.regionColour[(rgid)*4]));
    else glUniform4f(glGetUniformLocation(ss->prgObject, "uVolumeColour"), 1.0, 1.0, 1.0, 1.0 );

    //Draw two triangles to fill screen
    glBegin(GL_TRIANGLES);
    glVertex2f(-1, -1); glVertex2f(-1, 1); glVertex2f(1, -1);
    glVertex2f(-1,  1); glVertex2f( 1, 1); glVertex2f(1, -1);
    glEnd();

    glUseProgram(0);

    glPopMatrix();  //Restore original modelview for drawing rest of scene
    glEnable(GL_DEPTH_TEST);
}

void moment_maps(int pid, int texid, int rgid, shaderService *ss, float3 box, AABB clip, float* viewport, volumeProperty _vp)
{
    //Blending required for correct intensity output
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(ss->prgObject);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texid);
    glUniform1i(glGetUniformLocation(ss->prgObject, "uVolume"), 0);

    //Bounds for clipping the normalised coord volume bounding box [0,1]
    glUniform3fv(glGetUniformLocation(ss->prgObject, "uBBMin"), 1, clip.min.a);
    glUniform3fv(glGetUniformLocation(ss->prgObject, "uBBMax"), 1, clip.max.a);

    float focalLength = 1.0 / tan(0.5 * ss2qca() * PI/180.0);;
    glUniform1f(glGetUniformLocation(ss->prgObject, "uFocalLength"), focalLength);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uDensityFactor"), _vp.density);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uBrightness"), _vp.brightness);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uContrast"), _vp.contrast);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uPower"), _vp.power);
    glUniform1i(glGetUniformLocation(ss->prgObject, "uSamples"), _vp.samples);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uIsoValue"), _vp.isovalue);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uIsoSmooth"), _vp.isosmooth);
    glUniform4fv(glGetUniformLocation(ss->prgObject, "uIsoColour"), 1, _vp.isocolour);

    // Test by DV for range threshold in pixel intensity
    if (pd[pid].min_filter == -1)
        glUniform1f(glGetUniformLocation(ss->prgObject, "uMin"), pd[pid].mean-(3*pd[pid].stand_dev));
    else
        glUniform1f(glGetUniformLocation(ss->prgObject, "uMin"), pd[pid].min_filter);

    if (pd[pid].max_filter == -1)
        glUniform1f(glGetUniformLocation(ss->prgObject, "uMax"), pd[pid].mean+(3*pd[pid].stand_dev));
    else
        glUniform1f(glGetUniformLocation(ss->prgObject, "uMax"), pd[pid].max_filter);

    //For moment and MIP
    glUniform1f(glGetUniformLocation(ss->prgObject, "uMIP"), (bool)vp.mip);
    glUniform1f(glGetUniformLocation(ss->prgObject, "uMoment"), (bool)vp.moment);

    if (!_vp.enable_colour)
    {
        glUniform1f(glGetUniformLocation(ss->prgObject, "uEnableColour"), 0);
        glUniform1i(glGetUniformLocation(ss->prgObject, "uTransferFunction"), 1);
    }
    else
    {
        glUniform1f(glGetUniformLocation(ss->prgObject, "uEnableColour"), _vp.enable_colour);

        // The following information should come from server (currently for testing)
        //GLuint tex = loadColourmapTexture("colourmap.png");
        //glUniform1i(glGetUniformLocation(ss->prgObject, "uTransferFunction"), 1);
    }

    glUniform4fv(glGetUniformLocation(ss->prgObject, "uViewport"), 1, viewport);
    glUniform1i(glGetUniformLocation(ss->prgObject, "uIsoWalls"), _vp.drawWalls);
    glUniform3fv(glGetUniformLocation(ss->prgObject, "uResolution"), 1, _vp.resolution.a);

    glPushMatrix();  //Following changes to modelview for volume ray cast only
    //Apply translation and scaling for actual bounding box to normalised coords [0,1] used in volume renderer
    glTranslatef(-box.x*0.5, -box.y*0.5, -box.z*0.5);  //Translate to origin
    glScalef(1.0/box.x, 1.0/box.y, 1.0/box.z);         //Scale to normalised cube
    float matrix[16], projection[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uMVMatrix"), 1, GL_FALSE, matrix);
    matrix[12] = matrix[13] = matrix[14] = 0;
    glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uNMatrix"), 1, GL_FALSE, matrix);
    glUniformMatrix4fv(glGetUniformLocation(ss->prgObject, "uPMatrix"), 1, GL_FALSE, projection);

    if(rgid > -1) glUniform4fv(glGetUniformLocation(ss->prgObject, "uVolumeColour"), 1, &(rgc.regionColour[(rgid)*4]));
    else glUniform4f(glGetUniformLocation(ss->prgObject, "uVolumeColour"), 1.0, 1.0, 1.0, 1.0 );

    //Draw two triangles to fill screen
    glBegin(GL_TRIANGLES);
    glVertex2f(-1, -1); glVertex2f(-1, 1); glVertex2f(1, -1);
    glVertex2f(-1,  1); glVertex2f( 1, 1); glVertex2f(1, -1);
    glEnd();

    glUseProgram(0);

    glPopMatrix();  //Restore original modelview for drawing rest of scene
    glEnable(GL_DEPTH_TEST);
}
#endif

/*void screendump(int pid)
{
    char str[10];
    sprintf(str, "%d", pid);
    ss2wtga(str);
}*/

void drawTracksByShader(int pid)
{
    if(!config.show_track)
        return;
    glColor3f(0.8f, 0.25f, 0.0f);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_POLYGON_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );

    //glEnable(GL_CULL_FACE);

    glUseProgram(trShaders->prgObject);

    float matrix[16], projection[16], mvp[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    multMatrix(mvp, projection, matrix);

    glUniformMatrix4fv(glGetUniformLocation(trShaders->prgObject, "uMVMatrix"), 1, GL_FALSE, matrix);
    glUniformMatrix4fv(glGetUniformLocation(trShaders->prgObject, "uPMatrix"), 1, GL_FALSE, projection);
    glUniformMatrix4fv(glGetUniformLocation(trShaders->prgObject, "uMVPMatrix"), 1, GL_FALSE, mvp);

    mat4x4 tmpmat;
    for(int i = 0; i < 16; i++)
        tmpmat.a[i] = matrix[i];
    mat4x4 normalMatrix = inverseTranspose(tmpmat);
    glUniformMatrix4fv(glGetUniformLocation(trShaders->prgObject, "uNormalMatrix"), 1, GL_FALSE, normalMatrix.a);

    glUniform1i(glGetUniformLocation(trShaders->prgObject, "uDrawMode"), VBOdraw);
    glUniform1f(glGetUniformLocation(trShaders->prgObject, "uTrackAlpha"), _track_alpha);
    glUniform1f(glGetUniformLocation(trShaders->prgObject, "uRadius"), line_thick/1000.0);

    glLineWidth(line_thick);
    if(VBOdraw)
    {
        glEnableVertexAttribArray(glGetAttribLocation(trShaders->prgObject, "aTrackFlag"));
        glEnableVertexAttribArray(glGetAttribLocation(trShaders->prgObject, "aPos"));
        glEnableVertexAttribArray(glGetAttribLocation(trShaders->prgObject, "aColor"));

        if(config.platform == DESKTOP || bufferAllFlag)
        {
            //      glBindBuffer(GL_ARRAY_BUFFER, pd[pid].lineVertsVBOid);
            //      glEnableClientState(GL_VERTEX_ARRAY);
            //      glVertexPointer(3, GL_FLOAT, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, pd[pid].lineVertsVBOid);
            glVertexAttribPointer(glGetAttribLocation(trShaders->prgObject, "aPos"), 3, _GL_FLOAT_TYPE_, GL_FALSE, 0, 0);

            glBindBuffer (GL_ARRAY_BUFFER, pd[pid].trackFlagsVBOid);
            glVertexAttribPointer(glGetAttribLocation(trShaders->prgObject, "aTrackFlag"), 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);

            glBindBuffer (GL_ARRAY_BUFFER, pd[pid].trackColorVBOid);
            glVertexAttribPointer(glGetAttribLocation(trShaders->prgObject, "aColor"), 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);

            glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, pd[pid].npoints);
        }
        else if(config.platform == CAVE2)
        {
            glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, pd[pid].npoints);
        }
        glDisableVertexAttribArray(glGetAttribLocation(trShaders->prgObject, "aPos"));
        glDisableVertexAttribArray(glGetAttribLocation(trShaders->prgObject, "aTrackFlag"));
        glDisableVertexAttribArray(glGetAttribLocation(trShaders->prgObject, "aColor"));
    }
    else
    {
        /*    glBegin(GL_LINES);
            for(int i = 0; i < pd[pid].nlines; i++)
            {
                if(pd[pid].trackFlags[i*2])
                {
                    float r = pd[pid].trackColor[i*6+0] / 255.0;
                    float g = pd[pid].trackColor[i*6+1] / 255.0;
                    float b = pd[pid].trackColor[i*6+2] / 255.0;
                    glColor3f(r, g, b);
                    glVertex3f(pd[pid].lineVerts[i*6+0], pd[pid].lineVerts[i*6+1], pd[pid].lineVerts[i*6+2]);
                    r = pd[pid].trackColor[i*6+3] / 255.0;
                    g = pd[pid].trackColor[i*6+4] / 255.0;
                    b = pd[pid].trackColor[i*6+5] / 255.0;
                    glColor3f(r, g, b);
                    glVertex3f(pd[pid].lineVerts[i*6+3], pd[pid].lineVerts[i*6+4], pd[pid].lineVerts[i*6+5]);
                }
            }
            glEnd();
        */
    }

    glUseProgram(0);

    //glDisable(GL_CULL_FACE);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_POLYGON_SMOOTH );
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void updateTrackFlags(int pid)
{
    if (pd[pid].id != -1)
    {
        static int offset = -1;
        if (offset < 0) {
            struct timeval tv;
            struct timezone tz;
            //struct tm *tm;
            gettimeofday(&tv, &tz);
            //tm=localtime(&tv.tv_sec);
            srand48(tv.tv_usec);
            offset = (int)((float)DIVO * drand48());
        }

        float volRes[3];
        volRes[0] = volRes[1] = volRes[2] = 256;

        int i, j, k, np = 0;
        GLubyte flag;
        int ii, jj, kk;

        pd[pid].ntracks_drawn = 0;

        for(i = 0; i < pd[pid].ntracks; i++)
        {   // generate line vert indices
            int tlen = pd[pid].tracklens[i];
            if (!((i + offset) % (rgc.act_mask1 > -1 ? DIVO : (DIVO*256))))
            {
                flag = 1;
                int start_in = 0;
                float iv[3], ov0[3], ov1[3];
                for(k = 0; k < 3; k++) iv[k] = pd[pid].lineVerts[3*np + k];
                denormalizeS2volVertf(ov0, iv, volRes, volumeScale);
                for(k = 0; k < 3; k++) iv[k] = pd[pid].lineVerts[3*(np+tlen-1) + k];
                denormalizeS2volVertf(ov1, iv, volRes, volumeScale);

                if (rgc.act_mask1 > -1)
                {   // require start or end of track to be in mask1
                    // check start:
                    // int cast assumes diagonal tr matrix from image to track coords
                    ii = (int)(ov0[0]); jj = (int)(ov0[1]); kk = (int)(ov0[2]);
                    flag = ( (rgc.mask_vols[rgc.act_mask1])[kk*256*256 + jj*256 + ii] > rgc.maskThresh );
                }
                if (flag) {
                    start_in = 1;
                } else
                {   // check end
                    ii = (int)(ov1[0]); jj = (int)(ov1[1]); kk = (int)(ov1[2]);
                    flag = ( (rgc.mask_vols[rgc.act_mask1])[kk*256*256 + jj*256 + ii] > rgc.maskThresh );
                    if (flag) {
                        start_in = 0;
                    }
                }

                if (flag && (rgc.act_mask2 > -1))
                {   // require OTHER end of track to be in mask2 // TBI
                    if (start_in)
                    {   // check end
                        ii = (int)(ov1[0]); jj = (int)(ov1[1]); kk = (int)(ov1[2]);
                    } else
                    {   // check start
                        ii = (int)(ov0[0]); jj = (int)(ov0[1]); kk = (int)(ov0[2]);
                    }
                    flag = flag && ( (rgc.mask_vols[rgc.act_mask2])[kk*256*256 + jj*256 + ii] > rgc.maskThresh );
                }
            }
            else flag = 0;

            if (flag > 0) pd[pid].ntracks_drawn += 1;

            for(j = 0; j < tlen - 1; j++)
            {
                pd[pid].trackFlags[np + j] = flag;
            }
            pd[pid].trackFlags[np + tlen - 1] = 0;
            np += tlen;
        }

        pd[pid].trackFlagsAdj[0] = pd[pid].trackFlagsAdj[1];
        pd[pid].trackFlagsAdj[np+1] = pd[pid].trackFlagsAdj[np];
    }
}

void updateTracksByRemoteControl()
{
    if(!config.show_track)
        return;

#pragma omp parallel for
    for(int i = 0; i < (master ? 1 : MAXPANEL); i++)
        updateTrackFlags(i);

    for(int i = 0; i < (master ? 1 : MAXPANEL); i++)
        bindTrackFlagBuffer(i, pd[i].npoints+2);
}

void hsv2rgb(const float3 &hsv, float3 &rgb)
{
    float r, g, b, f, p, q, t;
    float h = hsv.a[0], s = hsv.a[1], v = hsv.a[2];
    int i = floor(h * 6);
    f = h * 6 - i;
    p = v * (1 - s);
    q = v * (1 - f * s);
    t = v * (1 - (1 - f) * s);

    r = g = b = 0;
    switch (i % 6) {
      case 0: r = v, g = t, b = p; break;
      case 1: r = q, g = v, b = p; break;
      case 2: r = p, g = v, b = t; break;
      case 3: r = p, g = q, b = v; break;
      case 4: r = t, g = p, b = v; break;
      case 5: r = v, g = p, b = q; break;
    }
    rgb.a[0] = r; rgb.a[1] = g; rgb.a[2] = b;
}

void computeDECColor(float3& dir, float3& rgb)
{
    float3 ndir = normalize(dir);
    if(ndir.a[2] < 0)
        ndir = -1*ndir;
    if(length(ndir) == 0)
        return;

    if(config.dec_color == DEC_ABSOLUTE)
    {
        for(int i = 0; i < 3; i++)
            rgb.a[i] = fabs(ndir.a[i]);
    }
    else
    {
        float azimuth;
        float azimuth_R = 0;
        float inclination = acos(ndir.a[2]);
        float3 hsv = make_float3(0);
        float ps = 0.5;
        hsv.a[1] = sin(ps*inclination)/sin(ps*PI/2);
        hsv.a[2] = 1;
        if(config.dec_color == DEC_NO_SYM) {
            azimuth = atan2(ndir.a[1], ndir.a[0]);
            hsv.a[0] = fmod(azimuth - azimuth_R + 2*PI, (float)2*PI);
        }
        else if (config.dec_color == DEC_ROTATIONAL_SYM)
        {
            azimuth = atan2(ndir.a[1], ndir.a[0]);
            hsv.a[0] = fmod (2*(azimuth - azimuth_R + 2*PI), (float)2*PI);
        }
        else if (config.dec_color == DEC_MIRROR_SYM)
        {
            azimuth = atan2(ndir.a[1], fabs(ndir.a[0]));
            hsv.a[0] = 2*fmod((azimuth - azimuth_R + PI), (float)PI);
        }
        hsv.a[0] = hsv.a[0] / (2*PI);

        hsv2rgb(hsv, rgb);
    }

    for(int i=0; i < 3; i++)
        rgb.a[i] = floor(rgb.a[i] * 255);
}

void computeDEC(int pid)
{
    int *np_array = (int*) malloc(pd[pid].ntracks*sizeof(int));
    int np_ac = 0;
    for(int i=0; i < pd[pid].ntracks; i++)
    {
        np_array[i] = np_ac;
        np_ac += pd[pid].tracklens[i];
    }

    #pragma omp parallel for
    for(int i = 0; i < pd[pid].ntracks; i++)
    {
        int tlen = pd[pid].tracklens[i];
        int np = np_array[i];
        if(tlen < 2)
            continue;

        float3 rgb = make_float3(0);

        if(config.dec_type == DEC_PERTRACK)
        {
            float3 dir = make_float3(0);
            for (int k = 0; k < 3; k++) {
                dir.a[k] = pd[pid].lineVerts[3*(np+0) + k] - pd[pid].lineVerts[3*(np+tlen-1) + k];
            }

            computeDECColor(dir, rgb);

            for(int j = 0; j < tlen; j++)
            {
                for(int k = 0; k < 3; k++)
                    pd[pid].trackColor[3*(np+j) + k] = (GLubyte)rgb.a[k];
            }
        }
        else // per segment
        {
            for(int j = 0; j < tlen; j++)
            {
                float3 dir = make_float3(0);
                int numdir = 0;
                if(j > 0)
                {
                    for(int k = 0; k < 3; k++)
                        dir.a[k] += pd[pid].lineVerts[3*(np+j) + k] - pd[pid].lineVerts[3*(np+j-1) + k];
                    numdir++;
                }
                if(j < tlen - 1)
                {
                    for(int k = 0; k < 3; k++)
                        dir.a[k] += pd[pid].lineVerts[3*(np+j+1) + k] - pd[pid].lineVerts[3*(np+j) + k];
                    numdir++;
                }

                dir /= (float)numdir;

                computeDECColor(dir, rgb);

                for(int k = 0; k < 3; k++)
                    pd[pid].trackColor[3*(np+j) + k] = (GLubyte)rgb.a[k];
            }
        }
    }

    for(int i = 0; i < 3; i++)
    {
        pd[pid].trackColorAdj[i] = pd[pid].trackColorAdj[3 + i];
        pd[pid].trackColorAdj[3*(np_ac+1) + i] = pd[pid].trackColorAdj[3*np_ac + i];
    }

    free(np_array);
}

#if !defined(ENCUBE_OMEGA)
void oglDraw(int *eye)
{
    int pid = xs2qsp();
    if((pd[pid].state != DRAW)) return;

    //Get which eye being rendered left/right
    char eyec = (char)*eye;
    int eyei = 0;
    if (eyec == 'l') eyei = -1;
    if (eyec == 'r') eyei = 1;

    //Calculate viewport eye shift in pixels
    GLfloat viewport[4];
    glGetFloatv(GL_VIEWPORT, viewport);
    float eye_sep_ratio = ss2qces() / ss2qcfl();
    float eye_shift = eyei * eye_sep_ratio * viewport[3] * 0.6 / tan(ss2qca() * M_PI/180.0);
    //fprintf(stderr, "EYE %d CAMERA EYESEP %f FOV %f OPTIONS SCREENHEIGHT %d EYE SHIFT %f\n",
    //  eye, camera.eyesep, camera.aperture, options.screenheight, eye_shift);
    viewport[0] += eye_shift;   //Horizontal shift for parallel camera stereo

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    float3 dmin, dmax;
    s2qvp(&(dmin.x),&(dmax.x), &(dmin.y),&(dmax.y), &(dmin.z),&(dmax.z));
    float3 box = (dmax-dmin);
    float3 scl = box / (swinMax-swinMin);
    float3 trns = (dmax+dmin)/2 - (swinMax+swinMin)/2;

    glTranslatef(trns.x, trns.y, trns.z);

    if (config.show_volume)
    {
        AABB clip = clpb;
        intersect(&clip, &aabb, &clpb);
        if (config.app_type != APP_ASTRO_FITS)
        {
            volumeRendering(pd[pid]._geometryTex_id, -1, vrShaders, box, clip, viewport, vp);
        }
        else
        {
            if (!vp.show_moment)
            {
                volumeRendering(pid, pd[pid]._geometryTex_id, -1, vrShaders, box, clip, viewport, vp);
            }
            else
            {
                moment_maps(pid, pd[pid]._geometryTex_id, -1, mmShaders, box, clip, viewport, vp);
            }
        }
    }

    if (rgc.act_mask1 > -1 && rgc.drawFlags[0])
    {
        AABB clip = clpb;
        intersect(&clip, &(rgc.aabb[rgc.act_mask1]), &clpb);
        volumeRendering(rgc.rg1Tex_id, 0, rgShaders, box, clip, viewport, rgc.vp);
        if(rgc.act_mask2 > -1 && rgc.drawFlags[1])
        {
            clip = clpb;
            intersect(&clip, &(rgc.aabb[rgc.act_mask2]), &clpb);
            volumeRendering(rgc.rg2Tex_id, 1, rgShaders, box, clip, viewport, rgc.vp);
        }
    }

    glScalef( scl.x, scl.y, scl.z );
    drawTracksByShader(pid);

    glPopMatrix();
}
#endif

void bindVertexPosBuffer(int pid, int num)
{
    if(config.platform == DESKTOP || bufferAllFlag)
    {
        if(!pd[pid].lineVertsVBOid) glGenBuffers (1, &(pd[pid].lineVertsVBOid));
        glBindBuffer (GL_ARRAY_BUFFER, pd[pid].lineVertsVBOid);
        glBufferData (GL_ARRAY_BUFFER, num * 3 * sizeof(HDreal), pd[pid].lineVertsAdj, GL_STATIC_DRAW);
    }
    else if(config.platform == CAVE2)
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(glGetAttribLocation(trShaders->prgObject, "aPos"));
        glVertexAttribPointer(glGetAttribLocation(trShaders->prgObject, "aPos"), 3, _GL_FLOAT_TYPE_, GL_FALSE, 0, pd[pid].lineVertsAdj);
        glDisableVertexAttribArray(glGetAttribLocation(trShaders->prgObject, "aPos"));
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void bindTrackFlagBuffer(int pid, int num)
{
    if(config.platform == DESKTOP || bufferAllFlag)
    {
        if(!pd[pid].trackFlagsVBOid) glGenBuffers (1, &(pd[pid].trackFlagsVBOid));
        glBindBuffer (GL_ARRAY_BUFFER, pd[pid].trackFlagsVBOid);
        glBufferData (GL_ARRAY_BUFFER, num * sizeof(GLubyte), pd[pid].trackFlagsAdj, GL_STATIC_DRAW);
    }
    else if(config.platform == CAVE2)
    {
        glEnableVertexAttribArray(glGetAttribLocation(trShaders->prgObject, "aTrackFlag"));
        glVertexAttribPointer(glGetAttribLocation(trShaders->prgObject, "aTrackFlag"), 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, pd[pid].trackFlagsAdj);
        glDisableVertexAttribArray(glGetAttribLocation(trShaders->prgObject, "aTrackFlag"));
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void bindTrackColorBuffer(int pid, int num)
{
    if(config.platform == DESKTOP || bufferAllFlag)
    {
        if(!pd[pid].trackColorVBOid) glGenBuffers (1, &(pd[pid].trackColorVBOid));
        glBindBuffer (GL_ARRAY_BUFFER, pd[pid].trackColorVBOid);
        glBufferData (GL_ARRAY_BUFFER, num * 3 * sizeof(GLubyte), pd[pid].trackColorAdj, GL_STATIC_DRAW);
    }
    else if(config.platform == CAVE2)
    {
        glEnableVertexAttribArray(glGetAttribLocation(trShaders->prgObject, "aColor"));
        glVertexAttribPointer(glGetAttribLocation(trShaders->prgObject, "aColor"), 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, pd[pid].trackColorAdj);
        glDisableVertexAttribArray(glGetAttribLocation(trShaders->prgObject, "aColor"));
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void evaluate_histogram(int id, bool normalise)
{
    pd[id].histogram  = (float*)malloc(pd[id].nbins * sizeof(float));

    //float bin_width = (pd[id].dmax-pd[id].dmin+1)/pd[id].nbins;

    for (int i=0; i<pd[id].nbins; i++)
        pd[id].histogram[i]=0;

    int idx=0;

    if (pd[id].dmin == FLT_MAX || pd[id].dmax==FLT_MIN )
    {
        // find min and max
        for (int i=0; i<pd[id].Nx; i++)
        {
            for (int j=0; j<pd[id].Ny; j++)
            {
                for (int k=0; k<pd[id].Nz; k++)
                {
                    if (pd[id].dmin > pd[id].array[i][j][k])
                        pd[id].dmin = pd[id].array[i][j][k];

                    if (pd[id].dmax < pd[id].array[i][j][k])
                        pd[id].dmax = pd[id].array[i][j][k];
                }
            }
        }
    }

    for (int i=0; i<pd[id].Nx; i++)
    {
        for (int j=0; j<pd[id].Ny; j++)
        {
            for (int k=0; k<pd[id].Nz; k++)
            {
                idx = (int)ceil(pd[id].array[i][j][k]*(pd[id].nbins-1));
                pd[id].histogram[idx]++;
            }
        }
    }

    if (normalise)
    {
        for (int i=0; i<pd[id].nbins; i++)
            pd[id].histogram[i] /= (pd[id].Nx * pd[id].Ny * pd[id].Nz);
    }

    fprintf(stderr, "Evaluate_hist... %d %f %f\n", pd[id].nbins, pd[id].dmin, pd[id].dmax);
}

void evaluate_moment_maps(int id)
{
    // Load file
    FILE *fp = fopen(pd[id].FITS_filename, "r");
    if (fp == NULL) {
        fprintf(stderr,"EXIT: Could not open input FITS file: %s\n",pd[id].FITS_filename);
    }
    else
    {
        fclose(fp);
    }

    FITSCube c = readFITScubeHeader(pd[id].FITS_filename, 1);
    readFITScube(&c, 1);

    int size_x = pd[id].Nx;
    int size_y = pd[id].Ny;
    int size_z = pd[id].Nz;

    float step_z = c.crp[2]; // CRPIX3
    float zero_z = c.cde[2]; // CDELT3

    // Init moment maps
    pd[id].mom0 = (float **) malloc(sizeof(float *) * size_x);
    pd[id].mom1 = (float **) malloc(sizeof(float *) * size_x);
    pd[id].mom2 = (float **) malloc(sizeof(float *) * size_x);
    for (int i = 0; i < size_x; ++i) {
        pd[id].mom0[i] = (float *) malloc(sizeof(float) * size_y);
        pd[id].mom1[i] = (float *) malloc(sizeof(float) * size_y);
        pd[id].mom2[i] = (float *) malloc(sizeof(float) * size_y);
    }
    for (int i = 0; i < size_x; ++i) {
        for (int j = 0; j < size_y; ++j) {
            pd[id].mom0[i][j] = 0;
            pd[id].mom1[i][j] = 0;
            pd[id].mom2[i][j] = 0;
        }
    }

    #pragma omp parallel for
    for (int x = 0; x < size_x; ++x)
    {
        for (int y = 0; y < size_y; ++y)
        {
            float m0, m1, m2;
            float flx_sum = 0;
            float flxvel_sum = 0;
            float flxdvel2_sum = 0;

            //
            // Calculate moment 0 for the current spatial position.
            //

            #pragma omp simd
            for (int z = 0; z < size_z; ++z)
            {
                float flx = c.array[x][y][z] > 0 ? c.array[x][y][z] : 0;
                flx_sum += flx;
            }
            m0 = flx_sum;
            pd[id].mom0[x][y] = m0;

            //
            // Calculate moment 1 for the current spatial position.
            //

            #pragma omp simd
            for (int z = 0; z < size_z; ++z)
            {
                float flx = c.array[x][y][z] > 0 ? c.array[x][y][z] : 0;
                float vel = zero_z + z*step_z;
                flxvel_sum += flx*vel;
            }
            m1 = flx_sum > 0 ? flxvel_sum/flx_sum : 0;
            m1 = m1 - zero_z - size_z/2*step_z; // ...
            pd[id].mom1[x][y] = m1;

            //
            // Calculate moment 2 for the current spatial position.
            //

            #pragma omp simd
            for (int z = 0; z < size_z; ++z)
            {
                float flx = c.array[x][y][z] > 0 ? c.array[x][y][z] : 0;
                float vel = zero_z + z*step_z;
                flxdvel2_sum += flx*(vel-m1)*(vel-m1);
            }
            m2 = flx_sum > 0 ? sqrt(flxdvel2_sum/flx_sum) : 0;
            pd[id].mom2[x][y] = m2;
        }
    }
}

// =======================================================================================
void testJson()
{
    FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen ( "jsontest.txt" , "rb" );
    if( !fp ) perror("jsontest.txt"),exit(1);

    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    /* allocate memory for entire content */
    buffer = (char*)calloc( 1, lSize+1 );
    if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

    /* copy the file into the buffer */
    if( 1!=fread( buffer , lSize, 1 , fp) )
        fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

    //fprintf(stderr, "%ld, jsonstring:\n %s \n\n", lSize, buffer);
    json::Object jsonObj = json::Deserialize(buffer);

    //Getting the volume properties
    json::Object props = jsonObj["volume"]["properties"];

    //Setting uniforms from json object with defaults
    //(prog->uniforms[] is a map where I've stored the results of getUniformLocation calls)
    fprintf(stderr, "colourmap: %i\n", props["colourmap"].ToInt(1));
    fprintf(stderr, "brightness: %f\n", props["brightness"].ToFloat(0.0));

    fclose(fp);
    free(buffer);
}

void loadConfig(const char* filename)
{
    FILE *fp;
    long lSize;
    char *buffer;

    fprintf(stderr, "Open config file: %s\n", filename);
    fp = fopen ( filename , "rt" );
    if( !fp ) perror(filename),exit(1);

    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    // allocate memory for entire content
    buffer = (char*)calloc( 1, lSize+1 );
    if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

    // copy the file into the buffer
    if( 1!=fread( buffer , lSize, 1 , fp) )
        fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

    //fprintf(stderr, "%ld, jsonstring:\n %s \n\n", lSize, buffer);
    json::Object jsonObj = json::Deserialize(buffer);
    string tmp;

    tmp = jsonObj["platform"].ToString("desktop");
    if(tmp.compare("desktop") == 0)
        config.platform = DESKTOP;
    else if (tmp.compare("cave2") == 0)
        config.platform = CAVE2;
    else
        fputs("invalid platform",stderr),exit(1);

    tmp = jsonObj["app_type"].ToString("brain");
    if(tmp.compare("brain_xrw") == 0)
        config.app_type = APP_BRAIN_XRW;
    else if (tmp.compare("brain_nii") == 0)
        config.app_type = APP_BRAIN_NII;
    else if(tmp.compare("astro_fits") == 0)
        config.app_type = APP_ASTRO_FITS;
    else
        fputs("invalid app type",stderr),exit(1);

    config.data_dir = jsonObj["data_dir"].ToString("");
    config.mask_dir = jsonObj["mask_dir"].ToString("");
    config.show_volume = jsonObj["show_volume"].ToBool(true);
    config.show_track = jsonObj["show_track"].ToBool(false);
    config.use_processed_track = jsonObj["use_processed_track"].ToBool(false);
    config.save_time = jsonObj["save_time"].ToBool(false);
    config.time_log_dir_filename = jsonObj["time_log_dir_filename"].ToString("");
    config.framerate_log_dir_filename = jsonObj["framerate_log_dir_filename"].ToString("");

    tmp = jsonObj["dec_type"].ToString("pertrack");
    if(tmp.compare("pertrack") == 0)
        config.dec_type = DEC_PERTRACK;
    else if (tmp.compare("persegment") == 0)
        config.dec_type = DEC_PERSEGMENT;
    else
        fputs("invalid dec type",stderr),exit(1);

    tmp = jsonObj["dec_color"].ToString("absolute");
    if(tmp.compare("absolute") == 0)
        config.dec_color = DEC_ABSOLUTE;
    else if (tmp.compare("no symmetry") == 0)
        config.dec_color = DEC_NO_SYM;
    else if (tmp.compare("rotational symmetry") == 0)
        config.dec_color = DEC_ROTATIONAL_SYM;
    else if (tmp.compare("mirror symmetry") == 0)
        config.dec_color = DEC_MIRROR_SYM;
    else
        fputs("invalid dec color scheme",stderr),exit(1);

    if(config.app_type != APP_ASTRO_FITS)
        need_reload_brain = false;

    if(config.platform == DESKTOP && config.use_processed_track)
    {
        fprintf(stderr, "use_processed_track is not supported on desktop platform now\n");
        fprintf(stderr, "use_processed_track is disabled\n");
        config.use_processed_track = false;
    }

    fclose(fp);
    free(buffer);
}

void loadTracks(int id)
{
    /* Open file */
    fprintf(stderr, "load data id = %d\n", id);
    FILE *TRKfile = fopen(pd[id].TRKfilename,"r");
    if (TRKfile == NULL)
    {
        fprintf(stderr,"TRK Filename: %s does not exist\n",pd[id].TRKfilename);
        //exit(1);
    }
    else
    {
        /* Was data loaded previously? If so, free memory */
        //    if (pd[id].xyz != NULL) freeData(id);
        if (pd[id].ntracks != 0)
            freeData(id);

        // Load tracks file here:
        // pd[id].ntracks   = countLines(TRKfile);
        fread(&pd[id].ntracks, sizeof(int), 1, TRKfile); // reading the first line
        //fprintf(stderr, "pd[id].ntracks = %d\n", pd[id].ntracks);

        pd[id].tracklens = (int *)malloc(pd[id].ntracks * sizeof(int));     // allocating memory
        fread(pd[id].tracklens, sizeof(int), pd[id].ntracks, TRKfile);     // reading all tracks' length

        //-fprintf(stderr, "*** NB ntracks /= 5 *** \n");
        //-pd[id].ntracks /= 5;
        // cannot do because the track file contains *all the x points*, then *all the y points*, ...
        // NOT (x1,y1,z1), (x2,y2,z2), ...

        int i = 0;
        pd[id].npoints = 0;
        for (i = 0; i < pd[id].ntracks; i++)  // Go over all tracks
        {
            pd[id].npoints += pd[id].tracklens[i];  // a += b means a = a + b
            //-fprintf(stderr, "pd[id].npoints = %d\n", pd[id].npoints);
        }

        int nt, np, nl;
        nt = np = nl = 0;

        if(config.platform == DESKTOP)
        {
            // read a subset of tracks noting *all* vertices are stored as *all x's* then
            // *all y's* etc.
            //#define SUBSAM 50
            int subsam = ceil(1.0 / lFraction);
            fprintf(stderr, "*** NB ntracks /= %d***\n", subsam);
            nt = pd[id].ntracks / subsam;
            np = 0;
            nl = 0; // for gl, 2 verts for each line

#if (1) // read the first 1/SUBSAM tracks of the whole file - fast, but possibly only tracks from a subregion
            for (i = 0; i < nt; i++) {
                np += pd[id].tracklens[i];
                nl += pd[id].tracklens[i] - 1;
            }

            pd[id].points = (float *)malloc(np * 3 * sizeof(float));
            fread(pd[id].points, sizeof(float), np, TRKfile);
            fseek(TRKfile, (pd[id].npoints - np) * sizeof(float), SEEK_CUR);
            fread(pd[id].points + np, sizeof(float), np, TRKfile);
            fseek(TRKfile, (pd[id].npoints - np) * sizeof(float), SEEK_CUR);
            fread(pd[id].points + (2 * np), sizeof(float), np, TRKfile);
            pd[id].ntracks = nt;
            pd[id].npoints = np;
            pd[id].nlines = nl;

# else // read every SUBSAM'th track - slower, but statistically more meaningful

            int *ntrlens = (int *)malloc(nt * sizeof(int));
            for (i = 0; i < nt; i++) {
                ntrlens[i] = pd[id].tracklens[i*subsam];
                np += ntrlens[i];
            }
            pd[id].points = (float *)malloc(np * 3 * sizeof(float));
            long offx = ftell(TRKfile);
            long offy = offx + pd[id].npoints * sizeof(float);
            long offz = offx + 2 * pd[id].npoints * sizeof(float);
            float *storex = pd[id].points;
            float *storey = pd[id].points + np;
            float *storez = pd[id].points + 2 * np;
            for (i = 0; i < nt; i++)
            {
                fseek(TRKfile, offx, SEEK_SET);
                fread(storex, sizeof(float), ntrlens[i], TRKfile);
                fseek(TRKfile, offy, SEEK_SET);
                fread(storey, sizeof(float), ntrlens[i], TRKfile);
                fseek(TRKfile, offz, SEEK_SET);
                fread(storez, sizeof(float), ntrlens[i], TRKfile);

                // move forward file "pointers"
                int j;
                for (j = 0; j < subsam; j++)
                {
                    offx += pd[id].tracklens[i*subsam+j] * sizeof(float);
                    offy += pd[id].tracklens[i*subsam+j] * sizeof(float);
                    offz += pd[id].tracklens[i*subsam+j] * sizeof(float);
                }
                // move forward storage pointers
                storex += ntrlens[i];
                storey += ntrlens[i];
                storez += ntrlens[i];
            }
            pd[id].ntracks = nt;
            pd[id].npoints = np;
            pd[id].tracklens = ntrlens;

#endif
        }
        else if(config.platform == CAVE2)
        {
            // normally load all tracks
            if(!bufferAllFlag) VBOdraw = 0;
            pd[id].points = (float *)malloc((pd[id].npoints) * 3 * sizeof(float));
            fread(pd[id].points, sizeof(float), (pd[id].npoints)*3, TRKfile);
            //printf("pd[id].points: %f\n", *pd[id].points);
        }

        float volRes[3];
        volRes[0] = volRes[1] = volRes[2] = 256;

        nt = pd[id].ntracks;
        np = pd[id].npoints;
        nl = np - nt;
        pd[id].nlines = nl;
        //fprintf(stderr, "nt: %d, np: %d, nl: %d\n", nt, np, nl);

        pd[id].lineVertsAdj = (HDreal *)malloc((np+2) * 3 * sizeof(HDreal));
        pd[id].trackFlagsAdj = (GLubyte *)malloc((np+2) * sizeof(GLubyte));
        pd[id].trackColorAdj = (GLubyte *)malloc((np+2) * 3 * sizeof(GLubyte));
        pd[id].lineVerts = pd[id].lineVertsAdj + 3;
        pd[id].trackFlags = pd[id].trackFlagsAdj + 1;
        pd[id].trackColor = pd[id].trackColorAdj + 3;

        for(int i = 0; i < np; i++)
        {
            float ov[3], iv[3];
            for(int j = 0; j < 3; j++) iv[j] = pd[id].points[j*np + i];
            normalizeS2volVertf(ov, iv, volRes, volumeScale);
            for(int j = 0; j < 3; j++) pd[id].lineVerts[i*3 + j] = ov[j];
        }

        for(int i = 0; i < 3; i++)
        {
            pd[id].lineVertsAdj[i] = pd[id].lineVertsAdj[3 + i];
            pd[id].lineVertsAdj[3*(np+1) + i] = pd[id].lineVertsAdj[3*np + i];
        }
        free(pd[id].points);
        pd[id].points = NULL;
        fclose(TRKfile);
    }
}

void loadBrain(int id)
{
    switch(config.app_type) {
        case APP_BRAIN_XRW:
            pd[id].array = initVolume_xrw(1, DMIN, DMAX, id);
            break;
        case APP_BRAIN_NII:
            initVolume_nii(1, DMIN, DMAX, id);
            break;
        case APP_ASTRO_FITS:
            pd[id].array = initVolume_fits(id);
            break;
        default:
            break;
    };

    if (pd[id].array != NULL)
    {
        //if (config.app_type == APP_ASTRO_FITS)
        //    evaluate_histogram(id,1);

        volToTexture(id); // create texture3D for brain volume
        for (int i = 0; i < pd[id].Nx; i++)
        {
            for (int j = 0; j < pd[id].Ny; j++)
            {
                free(pd[id].array[i][j]);
            }
            free(pd[id].array[i]);
        }
        free(pd[id].array);
        pd[id].array = NULL;
    }
    else
    {
        pd[id].state = EMPTY;
    }
}

void loadData(int pid)
{
    if(config.show_track)
    {
        if(!config.use_processed_track)
        {
            loadTracks(pid);
            // if TRK file does not exist, don't do more tracks-related operations...
            if (pd[pid].ntracks > 0 && pd[pid].tracklens)
            {
                updateTrackFlags(pid);
                computeDEC(pid);
            }
        }
        else
        {
            fprintf(stderr, "Open file %s\n", pd[pid].TRKfilename);
            FILE *file = fopen(pd[pid].TRKfilename, "rb");
            if(file == NULL)
            {
                fprintf(stderr, "Cannot open file to read %s\n", pd[pid].TRKfilename);
                exit(1);
            }

            assert(fread(&pd[pid].ntracks, sizeof(int), 1, file) == 1);
            assert(fread(&pd[pid].npoints, sizeof(int), 1, file) == 1);
            assert(fread(&pd[pid].nlines, sizeof(int), 1, file) == 1);
            //fprintf(stderr, "ntracks: %d, npoints: %d, nlines: %d\n", pd[pid].ntracks, pd[pid].npoints, pd[pid].nlines);

            if(pd[pid].tracklens) free(pd[pid].tracklens);
            pd[pid].tracklens = (int*) malloc(pd[pid].ntracks*sizeof(int));
            assert(fread(pd[pid].tracklens, sizeof(int), pd[pid].ntracks, file) == pd[pid].ntracks);

            if(pd[pid].lineVertsAdj) free(pd[pid].lineVertsAdj);
            pd[pid].lineVertsAdj = (HDreal *) malloc((pd[pid].npoints+2)*3*sizeof(HDreal));
            assert(fread(pd[pid].lineVertsAdj, sizeof(HDreal), (pd[pid].npoints+2)*3, file) == (pd[pid].npoints+2)*3);

            if(pd[pid].trackFlagsAdj) free(pd[pid].trackFlagsAdj);
            pd[pid].trackFlagsAdj = (unsigned char*) malloc((pd[pid].npoints+2)*sizeof(char));
            assert(fread(pd[pid].trackFlagsAdj, sizeof(char), (pd[pid].npoints+2), file) == (pd[pid].npoints+2));

            if(pd[pid].trackColorAdj) free(pd[pid].trackColorAdj);
            pd[pid].trackColorAdj = (unsigned char*) malloc((pd[pid].npoints+2)*3*sizeof(char));
            assert(fread(pd[pid].trackColorAdj, sizeof(char), (pd[pid].npoints+2)*3, file) == (pd[pid].npoints+2)*3);

            pd[pid].lineVerts = pd[pid].lineVertsAdj + 3;
            pd[pid].trackFlags = pd[pid].trackFlagsAdj + 1;
            pd[pid].trackColor = pd[pid].trackColorAdj + 3;

            fclose(file);

            updateTrackFlags(pid);
        }
    }

    if( !brain_loaded[pid] || need_reload_brain )
    {
        loadBrain(pid);
        brain_loaded[pid] = true;
    }
}

void freeData(int id)
{
    pd[id].ntracks = 0;
    if (pd[id].tracklens) {
        free(pd[id].tracklens);
        pd[id].tracklens = NULL;
    }
    pd[id].npoints = 0;
    if (pd[id].points) {
        free(pd[id].points);
        pd[id].points = NULL;
    }
    if (pd[id].array) {
        int i, j;
        for (i = 0; i < pd[id].Nx; i++) {
            for (j = 0; j < pd[id].Ny; j++) {
                free(pd[id].array[i][j]);
            }
            free(pd[id].array[i]);
        }
        free(pd[id].array);
        pd[id].array = NULL;
    }
    if (pd[id]._geometryTex_data) {
        free(pd[id]._geometryTex_data);
        pd[id]._geometryTex_data = NULL;
    }
    if (pd[id].lineVertsAdj) {
        free(pd[id].lineVertsAdj);
        pd[id].lineVertsAdj = NULL;
        pd[id].lineVerts = NULL;
    }
    if (pd[id].trackFlagsAdj) {
        free(pd[id].trackFlagsAdj);
        pd[id].trackFlagsAdj = NULL;
        pd[id].trackFlags = NULL;
    }
    if (pd[id].trackColorAdj) {
        free(pd[id].trackColorAdj);
        pd[id].trackColorAdj = NULL;
        pd[id].trackColor = NULL;
    }
}

void zeroPanel(int id) {
    pd[id].pid = -1;
    pd[id].state = EMPTY;
    pd[id].id = -1;
    pd[id].type = ' ';
    pd[id].ntracks = 0;
    pd[id].ntracks_drawn = 0;
    pd[id].tracklens = NULL;
    pd[id].npoints = 0;
    pd[id].points = NULL;
    pd[id].array = NULL;
    if (config.app_type == APP_ASTRO_FITS)
    {
        pd[id].mom0 = NULL;
        pd[id].mom1 = NULL;
        pd[id].mom2 = NULL;
    }

    if(need_reload_brain || !brain_loaded[id])
    {
        pd[id]._geometryTex_id = 0;
        pd[id]._geometryTex_data = NULL;
        pd[id].Nx = pd[id].Ny = pd[id].Nz = 0;
    }

    pd[id].infotexid = 0;
    pd[id].infoaspect = 1.0;
    pd[id].lineVerts = NULL;
    pd[id].lineVertsAdj = NULL;
    pd[id].lineVertsVBOid = 0;
    pd[id].trackFlags = NULL;
    pd[id].trackFlagsAdj = NULL;
    pd[id].trackFlagsVBOid = 0;
    pd[id].trackColor = NULL;
    pd[id].trackColorAdj = NULL;
    pd[id].trackColorVBOid = 0;
    pd[id].trackColorVBOid = 0;

    pd[id].nbins = 101;
    pd[id].histogram  = NULL;
    pd[id].dmin=FLT_MAX;
    pd[id].dmax=FLT_MIN;
    pd[id].mean = 0.;
    pd[id].stand_dev = 0.;
    pd[id].median = 0.;

    pd[id].selected = 0;
    pd[id].min_filter = -1.;
    pd[id].max_filter = -1.;
}

void clearPanel(int id) {
    if (pd[id].ntracks > 0 && pd[id].tracklens) {
        free(pd[id].tracklens);
    }
    if (pd[id].npoints > 0 && pd[id].points) {
        free(pd[id].points);
    }
    if (pd[id].lineVertsAdj) {
        free(pd[id].lineVertsAdj);
        //    pd[id].lineVerts = NULL;
    }
    if (pd[id].trackFlagsAdj) {
        free(pd[id].trackFlagsAdj);
        //    pd[id].trackFlags = NULL;
    }
    if (pd[id].trackColorAdj) {
        free(pd[id].trackColorAdj);
        //    pd[id].trackColor = NULL;
    }

    if (pd[id].Nx > 0 && pd[id].Ny > 0 && pd[id].Nz > 0 && pd[id].array) {
        int i, j;
        for (i = 0; i < pd[id].Nx; i++) {
            for (j = 0; j < pd[id].Ny; j++) {
                free(pd[id].array[i][j]);
            }
            free(pd[id].array[i]);
        }
        free(pd[id].array);
    }

//    if (config.app_type == APP_ASTRO_FITS)
//    {
//        if (pd[id].Nx > 0 && pd[id].Ny > 0 && pd[id].mom0 != NULL) {
//            for (int i = 0; i < pd[id].Nx; i++) {
//                free(pd[id].mom0[i]);
//            }
//            free(pd[id].mom0);
//        }
//
//        if (pd[id].Nx > 0 && pd[id].Ny > 0 && pd[id].mom1 != NULL) {
//            for (int i = 0; i < pd[id].Nx; i++) {
//                free(pd[id].mom1[i]);
//            }
//            free(pd[id].mom1);
//        }
//
//        if (pd[id].Nx > 0 && pd[id].Ny > 0 && pd[id].mom2 != NULL) {
//            for (int i = 0; i < pd[id].Nx; i++) {
//                free(pd[id].mom2[i]);
//            }
//            free(pd[id].mom2);
//        }
//    }

    /*
    if (pd[id].infotexid > 0) {
        ss2dt(pd[id].infotexid);
    }
    */

    if (pd[id].histogram){
        free(pd[id].histogram);
    }

    zeroPanel(id);
}

void queryGPUInfo()
{
    if(!glewIsSupported("GL_NVX_gpu_memory_info"))
    {
        fprintf(stderr, "GL_NVX_gpu_memory_info NOT supported!\n");
        return;
    }
    int avaiMem = 0, totalMem = 0;
    glGetIntegerv(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMem);
    glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &avaiMem);
    fprintf(stderr, "GPU TOTAL MEM: %7.2fMB, GPU AVAILABLE MEM: %7.2fMB\n", totalMem/1024.0, avaiMem/1024.0);
}

/*
Colour Colour_FromJson(json::Object& object, std::string key, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    Colour colour = {red, green, blue, alpha};
    if (!object.HasKey(key)) return colour;

    //Will accept integer colour or [r,g,b,a] array
    if (object[key].GetType() == json::IntVal)
    {
        colour.value = object[key];
    }
    else if (object[key].GetType() == json::ArrayVal)
    {
        json::Array array = object[key].ToArray();
        colour.r = (GLubyte)(array[0].ToInt(0)*1);
        colour.g = (GLubyte)(array[1].ToInt(0)*1);
        colour.b = (GLubyte)(array[2].ToInt(0)*1);

        if (array.size() > 3)
            colour.a = (GLubyte)(array[3].ToInt(0)/1);
    }
    return colour;
}
*/

static void multMatrix(float dst[16], const float src1[16], const float src2[16])
{
    float tmp[16];

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            tmp[j*4+i] = src1[0*4+i] * src2[j*4+0] +
                         src1[1*4+i] * src2[j*4+1] +
                         src1[2*4+i] * src2[j*4+2] +
                         src1[3*4+i] * src2[j*4+3];
        }
    }
    for (int i = 0; i < 16; i++) dst[i] = tmp[i];
}

void computeSphericalCoordv(float3 &sc, const float3 &vec)
{
    sc.x = sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
    sc.y = acosf(vec.z / sc.x);
    sc.z = atan2f(vec.y, vec.x);
}

void initRegionControl()
{
    rgc.num_masks = 3;
    rgc.mask_xvols = NULL;
    rgc.mask_vols = NULL;
    rgc.aabb = NULL;
    rgc.act_mask1 = rgc.act_mask2 = -1;
    rgc.maskThresh = 0.5;
    rgc.rg1Tex_id = rgc.rg2Tex_id = 0;
    rgc.regionColour[0] = 0.2; rgc.regionColour[1] = 0.2; rgc.regionColour[2] = 0.6; rgc.regionColour[3] = 0.75;
    rgc.regionColour[4] = 0.2; rgc.regionColour[5] = 0.2; rgc.regionColour[6] = 0.6; rgc.regionColour[7] = 0.75;
    rgc.vp = vp;
    rgc.vp.density = 60.0;
    rgc.vp.isovalue = 0.0;
    rgc.vp.isosmooth = 3.0;
    rgc.vp.isocolour[0] = 0.2; rgc.vp.isocolour[1] = 0.2; rgc.vp.isocolour[2] = 0.6; rgc.vp.isocolour[3] = 0.2;
}

void initMasks(void)
{
    rgc.nx = rgc.ny = rgc.nz = 256;
    rgc.loadFlags[0] = rgc.loadFlags[1] = false;
    rgc.drawFlags[0] = rgc.drawFlags[1] = false;

    // load ids
    string filename = config.mask_dir;
    filename.append("/vol_all.txt");
    FILE* f = fopen(filename.c_str(), "rt");
    if(!f)
    {
        fprintf(stderr, "Fail to open file: %s", filename.c_str());
        exit(1);
    }
    int n, id;
    fscanf(f, "%d", &n);
    fprintf(stderr, "%d Masks\n", n);
    mask_ids.clear();
    for(int i=0; i < n; i++)
    {
        fscanf(f, "%d", &id);
        mask_ids.push_back(id);
    }
    fclose(f);

    //load all mask
    filename.clear();
    filename = config.mask_dir;
    filename.append("/vol_all.xrw");
    mask_all_xrw = loadXraw((char*)filename.c_str());
    mask_temp_xrw = createXraw("temp.xrw", rgc.nx, rgc.ny, rgc.nz, 0);

    // init
    rgc.num_masks = n;
    if(rgc.mask_xvols != NULL)
        free(rgc.mask_xvols);
    rgc.mask_xvols = (VOL_STRUCT**)malloc(n * sizeof(VOL_STRUCT*));
    rgc.mask_vols = (GLfloat**)malloc(n * sizeof(GLfloat*));
    rgc.mask_isos = (int*)malloc(n * sizeof(int));
    rgc.aabb = (AABB*)malloc(n * sizeof(AABB));

    for(int i=0; i < n; i++)
    {
        rgc.mask_xvols[i] = NULL;
        rgc.mask_vols[i] = NULL;
        rgc.aabb[i] = make_aabb(make_float3(0), make_float3(1));
    }
}

void maskXvolToVol(int rgid, int mask_id)
{
    int xx = rgc.nx, yy = rgc.ny, zz = rgc.nz;
    int texSize = xx * yy * zz;
//    rgc.mask_vols[mask_id] = (GLubyte*)malloc(texSize * sizeof(GLubyte));
    rgc.mask_vols[mask_id] = (GLfloat*)malloc(texSize * sizeof(GLfloat));

    int mini, minj, mink, maxi, maxj, maxk;
    mini = minj = mink = 255;
    maxi = maxj = maxk = 0;
    for(int i = 0; i < xx; i++)
    {
        for(int j = 0; j < yy; j++)
        {
            for(int k = 0; k < zz; k++)
            {
                int voxid = k*xx*yy + j*xx + i;
                (rgc.mask_vols[mask_id])[voxid] = rgc.mask_xvols[mask_id]->data[i][j][k];// / (1.0/255);

                if((rgc.mask_vols[mask_id])[voxid] > 0 )
                {
                    if(i < mini)
                        mini = i;
                    if(j < minj)
                        minj = j;
                    if(k < mink)
                        mink = k;

                    if(i > maxi)
                        maxi = i;
                    if(j > maxj)
                        maxj = j;
                    if(k > maxk)
                        maxk = k;
                }
            }
        }
    }

    rgc.aabb[mask_id] = make_aabb(make_float3(mini, minj, mink) / (rgc.vp.resolution - 1), (make_float3(maxi, maxj, maxk)+1) / (rgc.vp.resolution - 1));
}

void bindRegionTex(int actrgid, int mid)    // rgid has to be 1 or 2
{
    if(actrgid < 1 || actrgid > 2 || mid < 0 || mid >= rgc.num_masks) return;

    GLuint *idPtr = (actrgid == 1 ? &(rgc.rg1Tex_id) : &(rgc.rg2Tex_id));
    if(*idPtr == 0) glGenTextures(1, idPtr);
    glBindTexture(GL_TEXTURE_3D, *idPtr);
    fprintf(stderr, "TEXTURE id: %d\n", *idPtr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
//    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8I, rgc.nx, rgc.ny, rgc.nz, 0, GL_RED, GL_UNSIGNED_BYTE, rgc.mask_vols[mid]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, rgc.nx, rgc.ny, rgc.nz, 0, GL_RED, GL_FLOAT, rgc.mask_vols[mid]);
    rgc.drawFlags[actrgid - 1] = true;
}

int getMaskid(int medicineid)
{
    for (int i=0; i < mask_ids.size(); i++)
        if(mask_ids[i] == medicineid)
            return i;
    return -1;
}

void loadMask(int i, int roinum)
{
    if(rgc.mask_vols[i] == NULL)
    {
        int stride[] = {1, 1, 1};

        int num = rgc.nx*rgc.ny*rgc.nz;
        for(int ind=0; ind < num; ind++)
        {
            if(mask_all_xrw->data[ind] == i+1) // in combined mask file, index from 1
                mask_temp_xrw->data[ind] = 255;
            else
                mask_temp_xrw->data[ind] = 0;
        }
        rgc.mask_xvols[i] = Xraw2Xvol(mask_temp_xrw, stride);
        //    fprintf(stderr, "region res: %d, %d, %d\n", rgc.mask_xvols[i]->nx, rgc.mask_xvols[i]->ny, rgc.mask_xvols[i]->nz);
        derivXvol(rgc.mask_xvols[i]);
        //  normaliseXvol(rgc.mask_xvols[i]);
        maskXvolToVol(roinum-1, i);
        free(rgc.mask_xvols[i]);
    }
//    rgc.loadFlags[roinum - 1] = true;
}

float ***initVolume_fits(int id)
{

    FILE *fp = fopen(pd[id].FITS_filename, "r");
    if (fp == NULL) {
        fprintf(stderr,"EXIT: Could not open input FITS file: %s\n",pd[id].FITS_filename);
        //exit(1);
        return NULL;
    }
    else
    {
        fclose(fp);


        FITSCube c = readFITScubeHeader(pd[id].FITS_filename, 1);

        readFITScube(&c, 1);

        pd[id].Nx = c.naxes[0];
        pd[id].Ny = c.naxes[1];
        pd[id].Nz = c.naxes[2];

        // normalise cube to be [0,1]
        //float *** ndat = (float ***)malloc(pd[id].Nx * sizeof(float **));
        int i, j, k;
        float dmin=9e99, dmax=-9e99;

        //float dx, dy, dz;
        for (i = 0; i < pd[id].Nx; i++) {
            for (j = 0; j < pd[id].Ny; j++) {
                for (k = 0; k < pd[id].Nz; k++) {
                    if (c.array[i][j][k] < dmin) {
                        dmin = c.array[i][j][k];
                    }
                    if (c.array[i][j][k] > dmax) {
                        dmax = c.array[i][j][k];
                    }
                }
            }
        }

        fprintf(stderr, "Normalising data to [0,1] range.\n");

        float scale = 1.0 / fabs(dmax-dmin);

        int n=0;
        float delta=0.;
        float mean=0.;
        float M2=0.;

        for (i=0; i<pd[id].Nx; i++) {
            for (j=0; j<pd[id].Ny; j++) {
                for (k=0; k<pd[id].Nz; k++){
                    // in place modification... may be an issue?
                    c.array[i][j][k] = (c.array[i][j][k] - dmin) * scale;

                    // update panel's min and max
                    if (c.array[i][j][k] < pd[id].dmin) {
                        pd[id].dmin = c.array[i][j][k];
                    }
                    if (c.array[i][j][k] > pd[id].dmax) {
                        pd[id].dmax = c.array[i][j][k];
                    }

                    // online variance and mean calculation
                    // thanks to David Knuth.
                    n += 1;
                    delta = c.array[i][j][k] - mean;
                    mean += delta/n;
                    M2 += delta*(c.array[i][j][k] - mean);
                }
            }
        }

        pd[id].mean = mean;
        pd[id].stand_dev = sqrt(M2/(n-1));

        // **********
        // Eval histogram and median.
        // Inspired by the Bin Approx algorithm (http://www.stat.cmu.edu/~ryantibs/median/) O(n) in worst case.
        // Bin x across the interval [mu-sigma, mu+sigma]
        /*
        int bottomcount = 0;
        int bincounts[1001];
        */

        pd[id].histogram  = (float*)malloc(pd[id].nbins * sizeof(float));
        for (i=0; i<pd[id].nbins; i++) {
            pd[id].histogram[i]=0;
            if (i < 1001)
                bincounts[i] = 0;
        }

        /*
        float scalefactor = 1000/(2*pd[id].stand_dev);
        float leftend =  pd[id].mean-pd[id].stand_dev;
        float rightend = pd[id].mean+pd[id].stand_dev;
        int bin;
        */
        int idx=0;

        fprintf(stderr, "Histogram binning.\n");
        for (i=0; i<pd[id].Nx; i++) {
            for (j=0; j<pd[id].Ny; j++) {
                for (k=0; k<pd[id].Nz; k++){
                    /*
                    if (c.array[i][j][k] < leftend) {
                        bottomcount++;
                    }
                    else if (c.array[i][j][k] < rightend) {
                        bin = (int)((c.array[i][j][k]-leftend) * scalefactor);
                        bincounts[bin]++;
                    }
                    */

                    idx = (int)ceil(c.array[i][j][k]*(pd[id].nbins-1));
                    pd[id].histogram[idx]++;
                }
            }
        }

        //normalize histogram
        int n_voxels = pd[id].Nx * pd[id].Ny * pd[id].Nz;
        for (int i=0; i<pd[id].nbins; i++)
            pd[id].histogram[i] /= n_voxels;

        /*
        // Eval median
        // If n is odd
        fprintf(stderr, "Looking for median.\n");
        if (n & 1) {
            // Find the bin that contains the median
            int k = (n+1)/2;
            int count = bottomcount;

            for (i = 0; i < 1001; i++) {
              count += bincounts[i];

              if (count >= k) {
                pd[id].median = (i+0.5)/scalefactor + leftend;
              }
            }
        }

        // If n is even
        else {
            // Find the bins that contains the medians
            int k = n/2;
            int count = bottomcount;

            for (i = 0; i < 1001; i++) {
                count += bincounts[i];

                if (count >= k) {
                    int j = i;
                    while (count == k) {
                        j++;
                        count += bincounts[j];
                    }
                    pd[id].median = (i+j+1)/(2*scalefactor) + leftend;
                }
            }
        }
        */
        // **********

        //fprintf(stderr, "panel id:%d\tMean:%f\tStd:%f\tMedian:%f\n", id, pd[id].mean, pd[id].stand_dev, pd[id].median);
        fprintf(stderr, "panel id:%d\tMean:%f\tStd:%f\n", id, pd[id].mean, pd[id].stand_dev);
        return c.array;
        }
}

float ***initVolume_xrw(int flag, float min, float max, int id)
{
    XRAW_STRUCT *xrw = loadXraw(pd[id].NIIfilename);
    int stride[] = {1, 1, 1};
    VOL_STRUCT *vol = Xraw2Xvol(xrw, stride);
    derivXvol(vol);
    //normaliseXvol(vol);
    //tightenXvol(vol, 2.0, 3.0);
    pd[id].Nx = vol->nx;
    pd[id].Ny = vol->ny;
    pd[id].Nz = vol->nz;

    deleteXraw(xrw);
    return vol->data;
}

//float ***initVolume_nii(int flag, float min, float max, int id)
void initVolume_nii(int flag, float min, float max, int id)
/* Allocate memory and initialise a data cube */
{
    int Nx = pd[id].Nx, Ny = pd[id].Ny, Nz = pd[id].Nz;

    fprintf(stderr, "initVolmyid = %d\n", id);
    fprintf(stderr,"initVolume_NII Filename: %s \n",pd[id].NIIfilename);
    FILE *NIIfile = fopen(pd[id].NIIfilename,"r");
    if (NIIfile == NULL)
    {
        fprintf(stderr,"NII Filename: %s does not exist\n",pd[id].NIIfilename);
        //exit(1);
    }
    else
    {
        unsigned short *voxels;          // declaration of pointer to all tracks' points NOTE: it has to be read as unsigned short because that is how it is stored in bin file. Later it is stored in a float array..
        voxels = (unsigned short *)malloc(Nx*Ny*Nz * sizeof(unsigned short));
        fread(voxels, sizeof(unsigned short), Nx*Ny*Nz, NIIfile);
        fclose(NIIfile);
        //float ***volume;    // reference to 3D array

        int i, j, k;
        long testpoint = -1;
        float *zero;
        zero = (float *)malloc(Nz *sizeof(float));
        for (i=0;i<Nz;i++)
        {
            zero[i] = 0.0;
        }
        pd[id].array = (float ***)malloc(Nx * sizeof(float **));  // allocating memory
        if (pd[id].array == NULL)
        {
            fprintf(stderr,"Failed to allocate %ld bytes\n",Nx*sizeof(float **));
            exit(-1);
        }
        for (i=0;i<Nx;i++)  // Along the X axis
        {
            pd[id].array[i] = (float **)malloc(Ny * sizeof(float *));
            if (pd[id].array[i] == NULL)
            {
                fprintf(stderr,"Failed to allocate %ld bytes\n",Nx*sizeof(float *));
                exit(-1);
            }
            for (j=0;j<Ny;j++)  // Along the Y axis
            {
                pd[id].array[i][j] = (float *)malloc(Nz * sizeof(float));
                if (pd[id].array[i][j] == NULL)
                {
                    fprintf(stderr,"Failed to allocate %ld bytes\n",Nx*sizeof(float));
                    exit(-1);
                }
                memcpy(pd[id].array[i][j], zero, Nz*sizeof(float)); /* Fill with zeroes!  memcpy copy memory area */
                if (flag == 1)  // What's this flag for? It's always 1
                {
                    //                int k;
                    for (k=0;k<Nz;k++)  // Along Z axis
                    {
                        //testpoint++;
                        testpoint = i + ((j) * Nx) + (k * Nx * Ny);  // this is reading the data in a different order
                        // {
                        pd[id].array[i][j][k] = voxels[testpoint]; //
                        // }

                        //                    printf("%d \n", voxels[testpoint]);  // Print out raw data
                        //                    printf("pd[id].array[%d][%d][%d] = %f \n", i, j, k, pd[id].array[i][j][k]);
                    }
                }
            }
        }
        if (zero != NULL)
        {
            free(zero); zero = NULL;
        }

        // THIS IS THE DERIVATIVE CODE
        float ***ndat = (float ***)malloc(Nx * sizeof(float **));
        //    int i, j, k;
        float lmin=9e99, lmax=-9e99;
        float lmean=0.0, lvar=0.0;
        float dx, dy, dz;
        for (i = 0; i < Nx; i++)
        {
            ndat[i] = (float **)malloc(Ny * sizeof(float *));
            for (j = 0; j < Ny; j++)
            {
                ndat[i][j] = (float *)malloc(Nz * sizeof(float));
                for (k = 0; k < Nz; k++) {
                    if ((i == 0) || (i == Nx-1) ||
                        (j == 0) || (j == Ny-1) ||
                        (k == 0) || (k == Nz-1))
                    {
                        ndat[i][j][k] = 0.;
                        continue;
                    }
                    dx = fabs(pd[id].array[i+1][j][k] - pd[id].array[i-1][j][k]);
                    dy = fabs(pd[id].array[i][j+1][k] - pd[id].array[i][j-1][k]);
                    dz = fabs(pd[id].array[i][j][k+1] - pd[id].array[i][j][k-1]);
                    // normalised, so max(dx*dx+dy*dy+dz*dz) = 3

                    ndat[i][j][k] = sqrt(0.333333*(dx*dx+dy*dy+dz*dz));

                    // dgb this is dangerous code: min and max are args to the function. Let's instead
                    // make locals that are initialised appropriately.

                    if (ndat[i][j][k] < lmin)
                    {
                        lmin = ndat[i][j][k];
                    }
                    if (ndat[i][j][k] > lmax)
                    {
                        lmax = ndat[i][j][k];
                    }
                    lmean += ndat[i][j][k];
                    lvar += ndat[i][j][k] * ndat[i][j][k];
                }
            }
        }

        lmean /= (float)(Nx*Ny*Nz);
        lvar = lvar / (float)(Nx*Ny*Nz) - lmean*lmean;
        lvar = sqrt(lvar);
        pd[id].dmin = lmin = lmean + 1.0 * lvar;
        pd[id].dmax = lmax = lmean + 2.0 * lvar;

        for (i = 0; i < Nx; i++)
        {
            for (j = 0; j < Ny; j++)
            {
                for (k = 0; k < Nz; k++)
                {
                    pd[id].array[i][j][k] = (ndat[i][j][k] - lmin) / (lmax - lmin);
                    //printf("pd[id].array[%d][%d][%d] = %f \n", i, j, k, pd[id].array[i][j][k]);
                    //printf("ndat[%d][%d][%d] = %f \n", i, j, k, ndat[i][j][k]);
                }
                free(ndat[i][j]);
            }
            free(ndat[i]);
        }
    }
}

int remote_cb(char *data)   // Remote Callback
{
    XYZ pos, up, vdir;
    char cmd[256];
    //fprintf(stderr,"REMOTE RECEIVED: %s\n",data);

    if (data[0] == 'C')
    {
        if (sscanf(data, "%s %lf %lf %lf %lf %lf %lf %lf %lf %lf", cmd, &pos.x, &pos.y, &pos.z, &up.x, &up.y, &up.z, &vdir.x, &vdir.y, &vdir.z) == 10)
        {
            ss2sc(pos, up, vdir, 1);  //Set the camera position, up vector and view direction.
            setpos = pos;
            setup = up;
            setvdir = vdir;
            camset = 1;
        } else
        {
            camset = 0;
        }
        return 1;

    } else if (data[0] == 'T') { // track density
        int exponent;
        if (sscanf(data, "%s %d", cmd, &exponent) == 2) {
            DIVO = 1L << exponent;
            if (DIVO < 1) {
                DIVO = 1;
            } else if (DIVO > (1L<<20)) {
                DIVO = 1L<<20;
            }
        }
        trackUpdateByRemoteFlag = 1;
    } else if (data[0] == '#') { // region colours (8 float)
        GLfloat c[8];
        if (sscanf(data, "%s %f %f %f %f %f %f %f %f", cmd, c, c+1, c+2, c+3, c+4, c+5, c+6, c+7) == 9) {
            memcpy(rgc.regionColour, c, 8*sizeof(GLfloat));
        }
    } else if (data[0] == 'O') { // roi
        int roinum, roiidx;
        if (sscanf(data, "%s %d %d", cmd, &roinum, &roiidx) == 3) {
            int idx = -1;
            if((roinum == 1 || roinum == 2))// && roiidx < rgc.num_masks)
            {
                idx = getMaskid(roiidx);
                if(idx >= 0 && idx < rgc.num_masks)
                {
                    loadMask(idx, roinum);
                    rgc.loadFlags[roinum - 1] = true;
                }
            }
            if (roinum == 1) {
                rgc.act_mask1 = idx;
                if (rgc.act_mask1 >= rgc.num_masks) {
                    rgc.act_mask1 = -1;
                    rgc.act_mask2 = -1; // also turn off act_mask2
                }
            } else if (roinum == 2) {
                rgc.act_mask2 = idx;
                if (rgc.act_mask2 >= rgc.num_masks || rgc.act_mask2 == rgc.act_mask1) {
                    rgc.act_mask2 = -1;
                }
            }
        }
        trackUpdateByRemoteFlag = 1;
    } else if (data[0] == 'L') { // label
        sscanf(data+2, "%s", APPLABEL);
    }
    else if (data[0] == 'J')
    {
        char jsonstr[4096];
        sscanf(data, "%s %s", cmd, jsonstr);

        fprintf(stderr, "\n\n%s\n\n", jsonstr);

        json::Object jsonObj = json::Deserialize(jsonstr);

        if (jsonObj.HasKey("volume"))
        {
            //Getting the volume properties
            json::Object props = jsonObj["volume"]["properties"];

            clpb.min.x = props["Xmin"].ToFloat(0); clpb.min.y = props["Ymin"].ToFloat(0); clpb.min.z = props["Zmin"].ToFloat(0);
            clpb.max.x = props["Xmax"].ToFloat(1); clpb.max.y = props["Ymax"].ToFloat(1); clpb.max.z = props["Zmax"].ToFloat(1);

            vp.enable_colour = props["usecolourmap"].ToBool(0);
            vp.colourmap = props["colourmap"].ToInt(1);
            vp.brightness = props["brightness"].ToFloat(0.0);
            vp.contrast = props["contrast"].ToFloat(1.0);
            vp.power = props["power"].ToFloat(1.0);
            vp.isovalue = props["isovalue"].ToFloat(0);
            vp.isosmooth = props["isosmooth"].ToFloat(0.1);
            vp.samples = props["samples"].ToInt(256);
            vp.density = props["density"].ToFloat(20.0);

            json::Array arr = jsonObj["res"].ToArray();
            for(int i = 0; i < 3; i++) vp.resolution.a[i] = arr[i].ToInt(256);

            //    arr = jsonObj["scale"].ToArray();
            //    for(int i = 0; i < 3; i++) scale[i] = arr[i].ToFloat(1.0);

            arr = props["isocolour"].ToArray();
            for(int i = 0; i < 3; i++) vp.isocolour[i] = arr[i].ToFloat(128.0) / 255.0;

            vp.isocolour[3] = props["isoalpha"].ToFloat(1);

            vp.drawWalls = props["drawWalls"].ToInt(1);

            float matrix[9];
            arr = jsonObj["volume"]["rotate"].ToArray();
            for(int i = 0; i < 9; i++) matrix[i] = arr[i].ToFloat(0.0);

            float translate[3];
            arr = jsonObj["volume"]["translate"].ToArray();
            for(int i = 0; i < 3; i++) translate[i] = arr[i].ToFloat(0.0);

            //Calculate eye position, lookAt and up vector from rotation/translation:
            float3 eye  = make_float3(0, 0, -translate[2]);
            float3 at = make_float3(-translate[0], -translate[1], 0);
            //Rotated camera pos
            eye = make_float3(eye.z * matrix[2], eye.z * matrix[5], eye.z * matrix[8]);
            //Rotate up vector (0,1,0)
            float3 up = make_float3(matrix[1], matrix[4], matrix[7]);
            //Rotate lookAt position
            at = make_float3(at.x * matrix[0] + at.y * matrix[1], at.x * matrix[3] + at.y * matrix[4], at.x * matrix[6] + at.y * matrix[7]);

            XYZ _campos, _vup, _vdir;
            _campos.x = eye.x; _campos.y = eye.y; _campos.z = eye.z;
            _vup.x = up.x; _vup.y = up.y; _vup.z = up.z;
            _vdir.x = at.x - eye.x; _vdir.y = at.y - eye.y; _vdir.z = at.z - eye.z;
            ss2sc(_campos, _vup, _vdir, 1);
            camset = 1;


            //        fprintf(stderr, "J %f %f %f %f %f %f %f %f %f\n",
            //                campos.x, campos.y, campos.z, vup.x, vup.y, vup.z, vdir.x, vdir.y, vdir.z);
            //    fprintf(stderr, "colourmap = %d, brightness = %f, contrast = %f, power = %f, isovalue = %f, isosmooth= %f, samples = %d, density = %f\n",
            //            colourmap, brightness, contrast, power, isovalue, isosmooth, samples, density);
            //    fprintf(stderr, "isocolor = %f, %f, %f\n", isocolour[0], isocolour[1], isocolour[2]);
        }
        else if (jsonObj.HasKey("moment"))
        {
            // For Astro
            vp.show_moment = jsonObj["show"].ToInt(0);
            vp.mip = jsonObj["mip"].ToInt(0);
            vp.moment = jsonObj["moment"].ToInt(0);

            fprintf(stderr, "show:%d mip:%d moment:%d\n", vp.show_moment, vp.mip, vp.moment);

            // Test screenshot... This shouldn't be here.
            //screendump(pd[0].Nx, pd[0].Ny);
        }
        else if (jsonObj.HasKey("min_filter"))
        {
            // For Astro
            int id = jsonObj["panel"].ToInt(0);
            pd[id].min_filter = jsonObj["min_filter"].ToFloat(-1.);
            pd[id].max_filter = jsonObj["max_filter"].ToFloat(-1.);
        }
        else
        {
            DIVO = (int)(40 / jsonObj["trackSamples"].ToFloat(0.001) - 40);
            if (DIVO == 0) DIVO = 1;
            line_thick = jsonObj["trackThickness"].ToFloat(2.0);
            _track_alpha = jsonObj["trackOpacity"].ToFloat(0.5);
            _doFrames = (int)jsonObj["labels"].ToBool(false);
            fprintf(stderr, "DIVO %d THICK %f ALPHA %f FRAMES %d\n", DIVO, line_thick, _track_alpha, _doFrames);
            trackUpdateByRemoteFlag = 1;
            return 1;
        }

    } else if (data[0] == 'u') { // (u)nload specific or all (will replace KU from kbdcb)
        int id=-1;

        sscanf(data, "%s %d", cmd, &id);

        fprintf(stderr, "scanned. %s %d\n", cmd, id);

        if (id==-1)
        {
            for (int i = 0; i < MAXPANEL; i++)
            {
                clearPanel(i);
            }
        }
        else
        {
            //zeroPanel(id);
            clearPanel(id);
        }

        fprintf(stderr, "unloaded.\n");

        return 1;

    } else if (data[0] == 'l') { // (l)oad specific or all (will replace KL from kbdcb) (DV)
        int panel_id = -1;
        int subject_id=-1;
        char code='\0';
        char NII_filename[MAXSTRING];

        sscanf(data, "%s %d %d %c %s", cmd, &panel_id, &subject_id, &code, NII_filename);

        //zeroPanel(panel_id);

        // if something is already loading on this panel (someone was impatient on the GUI)
        if (pd[panel_id].state == LOAD || pd[panel_id].state == PREDRAW)
            while(pd[panel_id].state !=DRAW){sleep(1);}

        clearPanel(panel_id);

        if (config.save_time)
        {
            // start loading timer
            gettimeofday(&pd[panel_id].start_time, NULL);
        }

        if (config.app_type == APP_ASTRO_FITS)
        {
            if (subject_id > -1)
            {
                pd[panel_id].state = LOAD;
                pd[panel_id].id = subject_id;
                sprintf(pd[panel_id].FITS_filename, "%s/fits/%d.fits", config.data_dir.c_str(), subject_id);
                fprintf(stderr, "%s/fits/%d.fits", config.data_dir.c_str(), subject_id);
            }
        }
        else
        {
            if (subject_id > -1)
            {
                pd[panel_id].state = LOAD;
                pd[panel_id].id = subject_id;
                pd[panel_id].type = code;
                char swcode[4];
                switch(pd[panel_id].type) {
                    case 'C':
                        sprintf(swcode, "con");
                        break;
                    case 'P':
                        sprintf(swcode, "pre");
                        break;
                    case 'S':
                        sprintf(swcode, "sym");
                        break;
                    default:
                        fprintf(stderr, "unknown subject code/type %c.\n", pd[panel_id].type);
                        exit(-1);
                }

                if(config.use_processed_track)
                    sprintf(pd[panel_id].TRKfilename, "%s/tracks/processed_data/pertrack_absolute/cavehd_%s_%02d.bin.cache", config.data_dir.c_str(), swcode, subject_id);

                else
                    sprintf(pd[panel_id].TRKfilename, "%s/tracks/cavehd_%s_%02d.bin", config.data_dir.c_str(), swcode, subject_id);

            //sprintf(pd[panel_id].FITS_filename, "fits/%d.fits", subject_id);
            }
            fprintf(stderr,"TRK Filename: %s \n",pd[panel_id].TRKfilename);

            sprintf(pd[panel_id].NIIfilename, "%s/%s", config.data_dir.c_str(), NII_filename);

            fprintf(stderr,"NII Filename: %s \n",pd[panel_id].NIIfilename);
        }

        return 1;

    } else if (data[0] == 'h') // higlight panel
    {
        int id=-1;
        int selected=0;

        sscanf(data, "%s %d %d", cmd, &id, &selected);

        //fprintf(stderr, "scanned for highlighting. %s %d %d\n", cmd, id, selected);

        if (id!=-1)
        {
            pd[id].selected = selected;
        }
        return 1;

    } else if (data[0] == 'a' || data[0] == '+' || data[0] == '-' || data[0] == 'd')
    {
        HandleKeyboard(data[0], 0, 0);

        if (data[0] == 'a')
            if (config.save_time)
            {
                //sleep(5);
                //FILE *fp = fopen(filename, "a"); // Open file for writing
                fprintf(stderr, "Frame rate = %.1f frames/second\n", getFramerate());
                //fprintf(fp, "Frame rate = %.1f frames/second\n", getFramerate());
                //fclose(fp);
                // stop spinning.
                //HandleKeyboard(data[0], 0, 0);
            }
    }
    return 0;
}

int remote_cb_sock(char *data, FILE* sockout)
{
    char cmd[256];
    if (data[0] == 'H')
    {
        int id=-1;
        sscanf(data, "%s %d", cmd, &id);

        if (pd[id].min_filter == -1.)
        {
            fprintf(sockout,
                "%d#%f#%f#%f#%f#%f#%f#",
                pd[id].nbins,
                pd[id].dmin,
                pd[id].dmax,
                pd[id].mean,
                pd[id].stand_dev,
                pd[id].mean-(3*pd[id].stand_dev),
                pd[id].mean+(3*pd[id].stand_dev));
        }
        else
        {
            fprintf(sockout,
                "%d#%f#%f#%f#%f#%f#%f#",
                pd[id].nbins,
                pd[id].dmin,
                pd[id].dmax,
                pd[id].mean,
                pd[id].stand_dev,
                pd[id].min_filter,
                pd[id].max_filter);
        }


        for (int i=0;i<pd[id].nbins;++i) {
            fprintf(sockout, "%f#", pd[id].histogram[i]);
            //fprintf(stderr, "%d\t%f\n", i, pd[id].histogram[i]);
        }
    } else if (data[0] == 'S')
    {
        // Returns subject_ids to client (-1 if no data is loaded to this panel)
        for (int i = 0; i < MAXPANEL; ++i)
        {
            fprintf(sockout, "%d#%d#", pd[i].id, pd[i].selected);
        }
    } else if (data[0] == 'T')
    {
        // Returns ntracks to client (-1 if no data is loaded to this panel)
        fprintf(stderr, "in 'T'\n");
        for (int i = 0; i < MAXPANEL; ++i)
        {
            fprintf(sockout, "%d#%d#", pd[i].id, pd[i].ntracks_drawn);
        }
    }
    return 0;
}

/*int remote_cb_sock_write(char *data, int socket)
{
    char cmd[256];
    if (data[0] == 'M')
    {
        int id=-1;
        sscanf(data, "%s %d", cmd, &id);
        evaluate_moment_maps(id);

        fprintf(stderr, "send x\n");
        write(socket, &pd[id].Nx, sizeof(int));
        fprintf(stderr, "send y\n");
        write(socket, &pd[id].Ny, sizeof(int));
        //write(socket, (int)sizeof(pd[id].mom0), sizeof(int));
        fprintf(stderr, "send mom0\n");
        write(socket, pd[id].mom0, sizeof(pd[id].mom0));

        fprintf(sockout,
                msg_string,
                pd[id].mom1);

        fprintf(sockout,
                msg_string,
                pd[id].mom2);

    }
    return 0;
}*/

/*
int send_moment_maps(int id, int socket)
{
    int size, read_size, stat, packet_index;
    char send_buffer[10240], read_buffer[256];
    packet_index = 1;

    //Send Picture Size
    printf("Sending Picture Size\n");
    write(socket, sizeof(pd[id].mom1), sizeof(int));

    //Send Picture as Byte Array
    printf("Sending Picture as Byte Array\n");

    do { //Read while we get errors that are due to signals.
        stat=read(socket, &read_buffer , 255);
        printf("Bytes read: %i\n",stat);
    } while (stat < 0);

    printf("Received data in socket\n");
    printf("Socket data: %c\n", read_buffer);

    while(!feof(picture)) {
        //while(packet_index = 1){
        //Read from the file into our send buffer
        read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

        //Send data through our socket
        do{
            stat = write(socket, send_buffer, read_size);
        }while (stat < 0);

        printf("Packet Number: %i\n",packet_index);
        printf("Packet Size Sent: %i\n",read_size);
        printf(" \n");
        printf(" \n");


        packet_index++;

        //Zero out our send buffer
        bzero(send_buffer, sizeof(send_buffer));
    }
}
*/

int kbdcb(unsigned char *key)  // Keyboard callback
{
    fprintf(stdout, "K%c\n", key[0]);
    if (key[0] == 'U') { // unload (note that this is also done in 'L'...)
        int i;

        for (i = 0; i < MAXPANEL; i++) {
            clearPanel(i);
        }
        return 1;

    } else if (key[0] == 'L') { // load  ( I'll leave it here for the moment so that the perl script still works.)
        //     See 'l' in remote_cb to avoid using a file and having to deal
        //     with all panels every time (DV).
        FILE *inputextfilepointer = fopen(inputextfile,"r");
        for (int i=0; i < MAXPANEL; i++) { // read 0,1,2,3 for TRK files
            clearPanel(i);

            fgets(TRKfilename[i], MAXSTRING, inputextfilepointer);
            int id;
            char code;
            sscanf(TRKfilename[i], "%d,%c", &id, &code);
            if (id > -1)
            {
                pd[i].state = LOAD;
                pd[i].id = id;
                pd[i].type = code;
                char swcode[4];
                switch(pd[i].type) {
                    case 'C':
                        sprintf(swcode, "con");
                        break;
                    case 'P':
                        sprintf(swcode, "pre");
                        break;
                    case 'S':
                        sprintf(swcode, "sym");
                        break;
                    default:
                        fprintf(stderr, "unknown subject code/type %c.\n", pd[i].type);
                        exit(-1);
                }
                if(config.use_processed_track)
                    sprintf(pd[i].TRKfilename, "%s/tracks/processed_data/pertrack_absolute/cavehd_%s_%02d.bin.cache", config.data_dir.c_str(), swcode, id);
                else
                    sprintf(pd[i].TRKfilename, "%s/tracks/cavehd_%s_%02d.bin", config.data_dir.c_str(), swcode, id);
            }
            fprintf(stderr,"TRK Filename: %s \n",pd[i].TRKfilename);
        }

        for (int i= 4; i < 2*MAXPANEL; i++) { // read 4,5,6,7 for NII files
            fprintf(stderr, "i = %d\n", i);
            fgets(NIIfilename[i], MAXSTRING, inputextfilepointer);
            int len = strlen(NIIfilename[i]);
            if (NIIfilename[i][len-1] == '\n')
                NIIfilename[i][len-1] = '\0';
            if (strcmp(NIIfilename[i], pd[i-MAXPANEL].NIIfilename) != 0) {
                //-pd[i].state = LOAD;
                sprintf(pd[i-MAXPANEL].NIIfilename, "%s", NIIfilename[i]);
            }
            fprintf(stderr,"NII Filename: %s \n",pd[i-MAXPANEL].NIIfilename);
        }
        fclose(inputextfilepointer);

        return 1;

    } else if (key[0] == 'c') {
        _i_cmap = (_i_cmap + 1) % _n_cmaps;
        int i;
        for (i = 0; i < MAXPANEL; i++) {
            pd[i].state = PREDRAW;
        }
        return 1;
    } else if (key[0] == 'n') { // volume less opaque
        if(vp.density >= 2) vp.density -= 2.0;
        return 1;
    } else if (key[0] == 'm') { // volume more opaque
        if(vp.density <= 100)
            vp.density += 2.0;
        return 1;
    } else if (key[0] == 'N') { // less tracks
        DIVO = DIVO * 2;
        updateTracksByRemoteControl();
        return 1;
    } else if (key[0] == 'M') { // more tracks
        if (DIVO > 1) {
            DIVO = DIVO / 2;
        }
        updateTracksByRemoteControl();
        return 1;
    } else if (key[0] == '(') { // tracks less opaque
        _track_alpha = powf(_track_alpha, 10./9.);
        fprintf(stderr, "_track_alpha = %f\n", _track_alpha);
        return 1;
    } else if (key[0] == ')') { // tracks more opaque
        _track_alpha = powf(_track_alpha, 9./10.);
        fprintf(stderr, "_track_alpha = %f\n", _track_alpha);
        return 1;
    } else if (key[0] == '{') { // region less opaque
        rgc.regionColour[3] = powf(rgc.regionColour[3], 10./9.);
        rgc.regionColour[7] = powf(rgc.regionColour[7], 10./9.);
        return 1;
    } else if (key[0] == '}') { // region more opaque
        rgc.regionColour[3] = powf(rgc.regionColour[3], 9./10.);
        rgc.regionColour[7] = powf(rgc.regionColour[7], 9./10.);
        return 1;
    } else if (key[0] == 's') { // cycle mask region 1 [-1, num_masks-1]
        rgc.act_mask1 = (rgc.act_mask1 + 1 + 1) % (rgc.num_masks+1) - 1;
        fprintf(stderr, "act_mask1 = %d\n", rgc.act_mask1);
        loadMask(rgc.act_mask1, 1);
        rgc.loadFlags[0] = true;
        updateTracksByRemoteControl();
        return 1;
    } else if(key[0] == 'S') {    // deactive regions
        rgc.act_mask1 = -1;
        rgc.act_mask2 = -1;
        updateTracksByRemoteControl();
    } else if (key[0] == 't') { // line thickness thinner
        line_thick = line_thick - .5;
        if (line_thick < 0.5) {
            line_thick = 0.5;
        }
        return 1;
    } else if (key[0] == 'T') { // line thicker
        line_thick = line_thick + .5;
        if (line_thick > 10.0) {
            line_thick = 10.0;
        }
        return 1;
    } else if (key[0] == 'i') { // smaller isovalue
        //   isovalue = powf(isovalue, 9./10.);
        vp.isovalue -= 0.01;
        if(vp.isovalue < 0) vp.isovalue = 0;
        fprintf(stderr, "isovalue = %f\n", vp.isovalue);
        return 1;
    } else if (key[0] == 'I') { // larger isovalue
        //   isovalue = powf(isovalue, 10./9.);
        vp.isovalue += 0.01;
        if(vp.isovalue > 1.0)
            vp.isovalue = 1.0;
        fprintf(stderr, "isovalue = %f\n", vp.isovalue);
        return 1;
    } else if (key[0] == 'V') { // toggle volume rener
        config.show_volume = !config.show_volume;
        return 1;
    } else if (key[0] == 'B') { // toggle track rener
        config.show_track = !config.show_track;
        return 1;
    } else if (key[0] == 'F') { // toggle drawing of panel frames
        _doFrames = (_doFrames + 1) % 2;
        return 1;
    } else if(key[0] == '1') {
        initShaders(&vrShaders, "raycasting", 0);
        fprintf(stderr, "RELOAD VOLUME SHADER\n");
        return 1;
    } else if(key[0] == '2') {
        initShaders(&rgShaders, "raycasting", 0);
        fprintf(stderr, "RELOAD REGION SHADER\n");
        return 1;
    } else if(key[0] == '3') {
        initShaders(&trShaders, "tubes", 1);
        fprintf(stderr, "RELOAD TRACKS SHADER: tube mode\n");
        return 1;
    } else if(key[0] == '4') {
        initShaders(&trShaders, "tracks_400", 1);
        fprintf(stderr, "RELOAD TRACKS SHADER: line mode\n");
        return 1;
    } else if(key[0] == '5') {
        VBOdraw = !VBOdraw;
        if(VBOdraw)
            fprintf(stderr, "DRAW MODE: GL DRAW ARRAY\n");
        else
            fprintf(stderr, "DRAW MODE: IMMEDIATE\n");
        return 1;
    } else if(key[0] == 'g') {
        queryGPUInfo();
        return 1;
    } else if(key[0] == 'Q'){
        //fprintf (stderr,"Q! __FUNCTION__ = %s\n", __FUNCTION__);
        for (int i=0; i < MAXPANEL; i++)
            clearPanel(i);
        exit(0);
    }

    return 0;
}

void prepit(int id)
{
    if (pd[id].state == LOAD) {
        //fprintf(stderr, "C ");
        loadData(id);             /* Read and load data */
        if (pd[id].state != EMPTY)
            pd[id].state = PREDRAW;   /* Change state */
    }
    //fprintf(stderr, "\n");
}

void buildVolumeTex(int pid)
{
    int xx = pd[pid].Nx, yy = pd[pid].Ny, zz = pd[pid].Nz;
    if(pd[pid]._geometryTex_id == 0) glGenTextures(1, &(pd[pid]._geometryTex_id));
    glBindTexture(GL_TEXTURE_3D, pd[pid]._geometryTex_id);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, xx, yy, zz, 0, GL_RED, GL_FLOAT, pd[pid]._geometryTex_data);
}

void buildBufferAndTexture(int pid)
{
    buildVolumeTex(pid);
    bindVertexPosBuffer(pid, pd[pid].npoints+2);
    bindTrackFlagBuffer(pid, pd[pid].npoints+2);
    bindTrackColorBuffer(pid, pd[pid].npoints+2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#if (11)
void initGL()
{
    glewInit();

    glDisable(GL_CULL_FACE);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

    initShaders(&vrShaders, "raycasting", 0);

    vp.show_moment = 0;
    vp.mip = 0;
    vp.moment = 0;

    // No need for tubes and region yet in astro mode
    if (config.app_type != APP_ASTRO_FITS)
    {
        initShaders(&trShaders, "tubes", 1);
        //initShaders(&trShaders, "tracks_400", 1);
        initShaders(&rgShaders, "raycasting", 0);
    }
    else
    {
        // moment maps shader
        initShaders(&mmShaders, "moment_maps", 0);
    }

    vp.resolution = make_float3(256);
    vp.density = 20.0;
    vp.enable_colour = 0;
    vp.colourmap = 1;
    vp.brightness = 0.0;
    vp.contrast = 1.0;
    vp.power = 1.0;
    vp.samples = 256;
    vp.isovalue = 0.0;
    vp.isosmooth = 3.0;
    vp.isocolour[0] = .7; vp.isocolour[1] = .4; vp.isocolour[2] = .4; vp.isocolour[3] = .3;
    vp.drawWalls = 1;
}
#endif

void initTrackFraction()
{
    // assume 1 master and 1 slaves with 4 panels each
    if(config.platform == DESKTOP) lFraction = MEMSIZE / 10.0 / 2048.0;
    else
    {
        float scale = 0.1; // for testing
        sFraction = scale * (MEMSIZE / 5.0 / 2048.0);
    }
    //fprintf(stderr, "load fraction: %f, sample fraction: %f\n", lFraction, sFraction);
}
