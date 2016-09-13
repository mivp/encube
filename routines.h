/* s2involve: S2PLOT INteractive VOLumetric Visualisation Environment    */
/*************************************************************************
 *   Copyright (C) 2015  Christopher Fluke (cfluke@swin.edu.au)
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/

#ifndef __ROUTINES_H__
#define __ROUTINES_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fitsio.h"
#include "s2plot.h"


typedef struct {
   fitsfile *fptr;                      /* Pointer to FITS file */
   long naxes[3];
   float crv[3], crp[3], cde[3];
   float dmin, dmax;
   float low, high;
   float rdmin, rdmax;
   float obsfreq;
   char **label;

   float ***array;

   XYZ min, max;
   XYZ rflag;
   XYZ range, mp;
   long axmax;
   XYZ vp;
} FITSCube;


typedef struct {
   int tx, ty, tz;
   int sx, sy, sz;
   int fx, fy, fz;
} ScaleAxis;

#define NDIGIT 12


float ***initVolume(int nx, int ny, int nz, float val);

int errorFITS(int status);
FITSCube readFITScubeHeader(char *fname, int debug);
void readFITScube(FITSCube *cube, int debug);
FITSCube copyCubeHeader(FITSCube c);
void averageCube(FITSCube *avg, FITSCube c, ScaleAxis sa);
void extractCube(FITSCube *avg, FITSCube c, ScaleAxis sa, int mid[3], int delta[3]);
void averExtractCube(FITSCube *avg, FITSCube c, ScaleAxis sa, XYZ roi0, XYZ roi1);
ScaleAxis setTarget(int target, long naxes[3]);

unsigned int *getDigitTextures(char *fname, int ndigit, float *asp);
void num2tid(float number, int dec, unsigned int *tid, float asp, XYZ xyz,
                        float width, int lr, int bt);




#define ALEFT 1
#define ARIGHT 2
#define ATOP 3
#define ABOTTOM 4

#endif
