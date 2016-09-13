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


#include "routines.h"

float ***initVolume(int nx, int ny, int nz, float val)
/* Allocate memory and initialise a data cube */
{
   float ***volume;
   int i, j;

   float *zero;
   zero = (float *)malloc(nz *sizeof(float));
   for (i=0;i<nz;i++) {
      zero[i] = val;
   }

   volume = (float ***)malloc(nx * sizeof(float **));
   if (volume == NULL) {
      fprintf(stderr,"Failed to allocate %ld bytes\n",nx*sizeof(float **));
      exit(-1);
   }
   for (i=0;i<nx;i++) {
      volume[i] = (float **)malloc(ny * sizeof(float *));
      if (volume[i] == NULL) {
         fprintf(stderr,"Failed to allocate %ld bytes\n",nx*sizeof(float *));
         exit(-1);
      }
      for (j=0;j<ny;j++) {
         volume[i][j] = (float *)malloc(nz * sizeof(float));
         if (volume[i][j] == NULL) {
            fprintf(stderr,"Failed to allocate %ld bytes\n",nx*sizeof(float));
            exit(-1);
         }
         memcpy(volume[i][j], zero, nz*sizeof(float));
                /* Fill with zeroes! */
      }
   }

   if (zero != NULL) { free(zero); zero = NULL; }

   return volume;
}


int errorFITS(int status)
/* Report an error from dealing with FITS files */
{
   if (status) {
       fits_report_error(stderr, status);
       exit(status);
   }
   return 0;
}


FITSCube copyCubeHeader(FITSCube c)
{
   FITSCube cube;
   int i;
   for (i=0;i<3;i++) {
      cube.naxes[i] = c.naxes[i];
      cube.crv[i] = c.crv[i];
      cube.crp[i] = c.crp[i];
      cube.cde[i] = c.cde[i];
   }
   cube.dmin = c.dmin;
   cube.dmax = c.dmax;
   cube.rdmin = c.rdmin;
   cube.rdmax = c.rdmax;
   cube.low   = c.low;
   cube.high  = c.high;
   cube.obsfreq = c.obsfreq;
   cube.label = c.label;         /* Pointer to memory */
   cube.array = NULL;            /* Empty for now */
   cube.min = c.min;
   cube.max = c.max;

   cube.rflag = c.rflag;
   cube.range = c.range;
   cube.mp = c.mp;
   cube.axmax = c.axmax;
   cube.vp = c.vp;

   return cube;
}


FITSCube readFITScubeHeader(char *fname, int debug)
/* Read in a FITS file called fname */
{
   FITSCube cube;                       /* The FITS cube and metadata */
   int status, nfound;
   long i;
   long naxis;

/* NOTE: Should set defaults for all values - assumes FITS metadata exists */
   cube.rdmin   = 0;
   cube.rdmax   = 0;
   cube.low     = 0;
   cube.high    = 0;
   cube.obsfreq = 0;

   status = 0;                          /* Error condition */

   if (fits_open_file(&cube.fptr, fname, READONLY, &status))
      status = errorFITS(status);

   if (fits_read_keys_lng(cube.fptr, "NAXIS", 0, 1, &naxis, &nfound, &status))
      status = errorFITS(status);

#ifdef NEVER
/* CJF: FORCE FOR MANGA */
   if (naxis < 3) {
      fprintf(stderr,"NAXIS = %ld is not a cube!\n", naxis);
      fprintf(stderr,"Check for image extension\n");
      int hdupos,hdutype;
      fits_movabs_hdu(cube.fptr, 2, NULL, &status);
      fits_get_hdu_num(cube.fptr, &hdupos);
      fits_get_hdu_type(cube.fptr, &hdutype, &status); 
      fprintf(stderr,"hdupos = %d %d %d\n",hdupos,hdutype, IMAGE_HDU);
      fprintf(stderr,"status = %d\n",status);
      if (fits_read_keys_lng(cube.fptr, "NAXIS", 0, 1, &naxis, &nfound, &status))
         status = errorFITS(status);
      fprintf(stderr,"Naxis = %ld\n",naxis);
/*
      if (status == END_OF_FILE) status = 0;
      else {
         fits_get_hdu_num(cube.fptr, &hdupos);
         fits_get_hdu_type(cube.fptr, &hdutype, &status); 
         fprintf(stderr,"hdupos = %d %d %d\n",hdupos,hdutype, IMAGE_HDU);
      }
*/
      fits_close_file(cube.fptr, &status);
      exit(1);
   }
#endif

   if (fits_read_keys_lng(cube.fptr, "NAXIS", 1, 3, cube.naxes, &nfound, &status))
      status = errorFITS(status);

   if (fits_read_keys_flt(cube.fptr, "CRVAL1", 0, 1, &cube.crv[0], &nfound, &status))
      status = errorFITS(status);
   if (fits_read_keys_flt(cube.fptr, "CRVAL2", 0, 1, &cube.crv[1], &nfound, &status))
      status = errorFITS(status);
   if (fits_read_keys_flt(cube.fptr, "CRVAL3", 0, 1, &cube.crv[2], &nfound, &status))
      status = errorFITS(status);

/* Coordinates of reference pixel */
   if (fits_read_keys_flt(cube.fptr, "CRPIX1", 0, 1, &cube.crp[0], &nfound, &status))
      status = errorFITS(status);
   if (fits_read_keys_flt(cube.fptr, "CRPIX2", 0, 1, &cube.crp[1], &nfound, &status))
      status = errorFITS(status);
   if (fits_read_keys_flt(cube.fptr, "CRPIX3", 0, 1, &cube.crp[2], &nfound, &status))
      status = errorFITS(status);

/* Gradient at reference pixel */
   if (fits_read_keys_flt(cube.fptr, "CDELT1", 0, 1, &cube.cde[0], &nfound, &status))
      status = errorFITS(status);
   if (fits_read_keys_flt(cube.fptr, "CDELT2", 0, 1, &cube.cde[1], &nfound, &status))
      status = errorFITS(status);
   if (fits_read_keys_flt(cube.fptr, "CDELT3", 0, 1, &cube.cde[2], &nfound, &status))
      status = errorFITS(status);

   cube.label = (char **)calloc(3, sizeof(char *));
   for (i=0;i<3;i++) {
      cube.label[i] = (char *)calloc(256, sizeof(char));
   }

   if (fits_read_keys_str(cube.fptr, "CTYPE1", 0, 1, &cube.label[0], &nfound, &status))
      status = errorFITS(status);
   if (fits_read_keys_str(cube.fptr, "CTYPE2", 0, 1, &cube.label[1], &nfound, &status))
      status = errorFITS(status);
   if (fits_read_keys_str(cube.fptr, "CTYPE3", 0, 1, &cube.label[2], &nfound, &status))
      status = errorFITS(status);

   double MYEPS=1E-7;
   for (i=0;i<3;i++) {
      if (fabs(cube.cde[i]) < MYEPS) cube.cde[i] = 1;
   }


/* Data range */
/* NOTE: Check the FITS reference to see if this is the correct way to calculate these ranges */
   cube.min.x = (1-cube.crp[0])*cube.cde[0] + cube.crv[0];
   cube.max.x = (cube.naxes[0]-cube.crp[0])*cube.cde[0] + cube.crv[0];
   cube.min.y = (1-cube.crp[1])*cube.cde[1] + cube.crv[1];
   cube.max.y = (cube.naxes[1]-cube.crp[1])*cube.cde[1] + cube.crv[1];
   cube.min.z = (1-cube.crp[2])*cube.cde[2] + cube.crv[2];
   cube.max.z = (cube.naxes[2]-cube.crp[2])*cube.cde[2] + cube.crv[2];

   cube.rflag.x = (cube.max.x < cube.min.x) ? -1 : +1;
   cube.rflag.y = (cube.max.y < cube.min.y) ? -1 : +1;
   cube.rflag.z = (cube.max.z < cube.min.z) ? -1 : +1;

   double swap;
   if (cube.rflag.x < 0) {
      swap = cube.min.x; cube.min.x = cube.max.x; cube.max.x = swap;
   }
   if (cube.rflag.y < 0) {
      swap = cube.min.y; cube.min.y = cube.max.y; cube.max.y = swap;
   }
   if (cube.rflag.z < 0) {
      swap = cube.min.z; cube.min.z = cube.max.z; cube.max.z = swap;
   }

/* Data minimum and maximum */
   if (fits_read_keys_flt(cube.fptr, "DATAMIN", 0, 1, &cube.rdmin, &nfound, &status))
      status = errorFITS(status);
   if (fits_read_keys_flt(cube.fptr, "DATAMAX", 0, 1, &cube.rdmax, &nfound, &status))
      status = errorFITS(status);

/* Observing frequency */
/* NOTE: Is this sensible for optical data cubes? Not currently used */
   if (fits_read_keys_flt(cube.fptr, "OBSFREQ", 0, 1, &cube.obsfreq, &nfound, &status))
      status = errorFITS(status);


/* Calculate derived values used for s2plotting */


/* Data range */
   cube.range.x = fabs(cube.max.x - cube.min.x);
   cube.range.y = fabs(cube.max.y - cube.min.y);
   cube.range.z = fabs(cube.max.z - cube.min.z);

/* Mid-point */
   cube.mp.x = (cube.max.x + cube.min.x)*0.5;
   cube.mp.y = (cube.max.y + cube.min.y)*0.5;
   cube.mp.z = (cube.max.z + cube.min.z)*0.5;

/* Biggest axis */
   cube.axmax = cube.naxes[0];
   if (cube.naxes[1] > cube.axmax) cube.axmax = cube.naxes[1];
   if (cube.naxes[2] > cube.axmax) cube.axmax = cube.naxes[2];

/* Viewport range scaled by maximum axis */
   cube.vp.x = (float)cube.naxes[0]/(float)cube.axmax;
   cube.vp.y = (float)cube.naxes[1]/(float)cube.axmax;
   cube.vp.z = (float)cube.naxes[2]/(float)cube.axmax;


   return cube;
}



void readFITScube(FITSCube *cube, int debug)
{
   long fpixel = 1, i,j,k;
   float nullval = 0; /* Don't check for null values in the cube */
   int status = 0, anynull;

   long Mem = (cube->naxes[0]*cube->naxes[1]*cube->naxes[2]*sizeof(float))/(long)1000000;

   if (debug) fprintf(stderr,"Allocating FITScube array: %ld Mbytes\n",Mem);
   float ***array = initVolume(cube->naxes[0],cube->naxes[1],cube->naxes[2],0.0);
   if (array == NULL) {
      fprintf(stderr,"Could not allocate %ld Mbytes\n",Mem);
      exit(1);
   }

   float *dummy;
   Mem = (cube->naxes[0]*sizeof(float))/(long)1000;
   if (debug) fprintf(stderr,"Allocating dummy array: %ld Kbytes\n",Mem);
   dummy = (float *)calloc(cube->naxes[0], sizeof(float));
   if (dummy == NULL) {
      fprintf(stderr,"Could not allocate %ld Kbytes\n",Mem);
      exit(1);
   }

   for (k=0;k<cube->naxes[2];k++) {
      for (j=0;j<cube->naxes[1];j++) {
         if (fits_read_img(cube->fptr, TFLOAT, fpixel,
                cube->naxes[0], &nullval, dummy, &anynull, &status)) {
            status = errorFITS(status);
         }

	/* Copy from dummy array into the FITS cube array */
         for (i=0;i<cube->naxes[0];i++) {
            array[i][j][k] = dummy[i];
         }
         fpixel += cube->naxes[0];	/* Advance the pixel pointer */
      }
   }

/* Confirm min/max from the array */
/* NOTE: Calculated but not used beyond providing a warning */
   cube->dmin = array[0][0][0];
   cube->dmax = array[0][0][0];
   for (i=0;i<cube->naxes[0];i++) {
      for (j=0;j<cube->naxes[1];j++) {
         for (k=0;k<cube->naxes[2];k++) {
            if (array[i][j][k] < cube->dmin) cube->dmin = array[i][j][k];
            if (array[i][j][k] > cube->dmax) cube->dmax = array[i][j][k];
         }
      }
   }
 
   float TEPS = 1.0E-5;
   if (fabs(cube->rdmin-cube->dmin) > TEPS) {
      fprintf(stderr,"Data minimum discrepancy: %12.3f %12.3f\n",
		cube->rdmin,cube->dmin);
   }
   if (fabs(cube->rdmax-cube->dmax) > TEPS) {
      fprintf(stderr,"Data minimum discrepancy: %12.3f %12.3f\n",
		cube->rdmax,cube->dmax);
   } 

   cube->low  = cube->dmin;
   cube->high = cube->dmax;

/* Transfer pointer to array into the FITS cube */
   cube->array = array;
   if (fits_close_file(cube->fptr, &status))
      status = errorFITS(status);

/* Clean up allocated memory that is not longe required */
   if (dummy != NULL) { free(dummy); dummy = NULL; }

   return;
}


void averExtractCube(FITSCube *avg, FITSCube c, ScaleAxis sa, XYZ roi0, XYZ roi1)
{
   int ix0 = (int)floor(roi0.x*(float)c.naxes[0]);
   int ix1 = (int)ceil(roi1.x*(float)c.naxes[0]);
   int iy0 = (int)floor(roi0.y*(float)c.naxes[1]);
   int iy1 = (int)ceil(roi1.y*(float)c.naxes[1]);
   int iz0 = (int)floor(roi0.z*(float)c.naxes[2]);
   int iz1 = (int)ceil(roi1.z*(float)c.naxes[2]);

   int i,j,k;
   int ii = 0, jj = 0 , kk = 0;
   int il, jl, kl;
   int dx, dy, dz;
   dx = (int)((float)(ix1-ix0)/(int)((float)sa.tx-1))-1;
   dy = (int)((float)(iy1-iy0)/(int)((float)sa.ty-1))-1;
   dz = (int)((float)(iz1-iz0)/(int)((float)sa.tz-1))-1;

   if (dx < 0) { ix0 = 0; dx = sa.tx;} 
   if (dy < 0) { iy0 = 0; dy = sa.ty;} 
   if (dz < 0) { iz0 = 0; dz = sa.tz; }
   fprintf(stderr,"Dim: %ld %ld %ld\n",c.naxes[0],c.naxes[1],c.naxes[2]);
   fprintf(stderr,"Here %d %d %d\n",dx,dy,dz);
   float ***array = initVolume(sa.tx, sa.ty, sa.tz, 0.0);

   float sum;
   int mx, my, mz;
   long Lc = 0;
   long Ntot = 0;
   for (i=ix0;i<=(ix1-dx);i+=dx) {
      mx = i+dx;
      jj = 0;
      if (mx > c.naxes[0]) { mx = c.naxes[0]; fprintf(stderr,"Special i %d %d\n",i,ix1); }
      for (j=iy0;j<=(iy1-dy);j+=dy) {
         my = j+dy;
         if (my > c.naxes[1]) { my = c.naxes[1]; fprintf(stderr,"Special j %d %d\n",j,iy1); }
         kk = 0;
         for (k=iz0;k<=(iz1-dz);k+=dz) {
            mz = k+dz;
            if (mz > c.naxes[2]) { mz = c.naxes[2]; fprintf(stderr,"Special k %d %d\n",k,iz1); }
            Lc = 0;
            sum = 0.0;
            for (il=0;il<mx;il++) {
               for (jl=j;jl<my;jl++) {
                  for (kl=k;kl<mz;kl++) {
	             sum += c.array[il][jl][kl];
                     Lc++;
                     Ntot++;
                  }
               }
            }
	    fprintf(stderr,"Sum: %f\n",sum);
            fprintf(stderr,"%d %d %d %d %d %d\n",ii,jj,kk,sa.tx,sa.ty,sa.tz);
/*
            array[ii][jj][kk] = sum/(float)Lc; 
*/
            kk++;
         }
         jj++;
      }
      ii++;
   }
   fprintf(stderr,"Made it out\n");

/* NOTE: Probably need to normalise flux values by division */

   float dmin = array[0][0][0];
   if (isnan(dmin)) dmin = 0;
   float dmax = array[0][0][0];
   if (isnan(dmax)) dmax = 0;
   int iflag = 0;
   fprintf(stderr,"Here\n");
   for (i=0;i<sa.tx;i++) {
      for (j=0;j<sa.ty;j++) {
         for (k=0;k<sa.tz;k++) {
            if (!isnan(array[i][j][k])) {
               if (array[i][j][k] < dmin) dmin = array[i][j][k];
               if (array[i][j][k] > dmax) dmax = array[i][j][k];
            } else {
               iflag++;
            }
         }
      }
   }
   fprintf(stderr,"Here %f %f\n",dmin,dmax);


/* NOTE: Not sure what this does... */
   if (iflag > 0) {
      for (i=0;i<sa.tx;i++) {
         for (j=0;j<sa.ty;j++) {
            for (k=0;k<sa.tz;k++) {
               if (isnan(array[i][j][k])) {
                  array[i][j][k] = dmin-1.0;
               }
            }
         }
      }
   }

   avg->array = array;
   avg->dmin  = dmin;
   avg->dmax  = dmax;
   avg->low   = dmin;
   avg->high  = dmax;
   avg->rdmin = c.rdmin;                /* Read data minimum */
   avg->rdmax = c.rdmax;
   avg->naxes[0] = sa.tx;
   avg->naxes[1] = sa.ty;
   avg->naxes[2] = sa.tz;


   avg->axmax = avg->naxes[0];
   if (avg->naxes[1] > avg->axmax) avg->axmax = avg->naxes[1];
   if (avg->naxes[2] > avg->axmax) avg->axmax = avg->naxes[2];

   avg->vp.x = (float)avg->naxes[0]/(float)avg->axmax;
   avg->vp.y = (float)avg->naxes[1]/(float)avg->axmax;
   avg->vp.z = (float)avg->naxes[2]/(float)avg->axmax;


  
}

void extractCube(FITSCube *ext, FITSCube c, ScaleAxis sa, int mid[3], int delta[3])
/* NOTE: Need to write code here to extract a section of a cube */
{
   int dx = delta[0];
   int dy = delta[1];
   int dz = delta[2];
   if (dx > c.naxes[0]) dx = c.naxes[0];
   if (dy > c.naxes[1]) dy = c.naxes[1];
   if (dz > c.naxes[2]) dz = c.naxes[2];

   int ix1 = mid[0] - dx/2, ix2 = ix1 + (dx-1);
   int iy1 = mid[1] - dy/2, iy2 = iy1 + (dy-1);
   int iz1 = mid[2] - dz/2, iz2 = iz1 + (dz-1);

   fprintf(stderr,"%d %d %d %d %d %d\n",ix1,ix2,iy1,iy2,iz1,iz2);
   fprintf(stderr,"%d %d %d\n",dx,dy,dz);
   exit(0);
   float ***array = initVolume(dx, dy, dz, 0.0);
   int i,j,k;
   for (i=ix1;i<ix2;i++) {
      for (j=iy1;j<iy2;j++) {
         for (k=iz1;k<iz2;k++) {
            array[i-ix1][j-iy1][k-iz1] = c.array[i][j][k];
         }
      }
   }

   float dmin = array[0][0][0];
   if (isnan(dmin)) dmin = 0;
   float dmax = array[0][0][0];
   if (isnan(dmax)) dmax = 0;
   int iflag = 0;
   for (i=0;i<dx;i++) {
      for (j=0;j<dy;j++) {
         for (k=0;k<dz;k++) {
            if (!isnan(array[i][j][k])) {
               if (array[i][j][k] < dmin) dmin = array[i][j][k];
               if (array[i][j][k] > dmax) dmax = array[i][j][k];
            } else {
               iflag++;
            }
         }
      }
   }

/* NOTE: Not sure what this does... */
   if (iflag > 0) {
      for (i=0;i<dx;i++) {
         for (j=0;j<dy;j++) {
            for (k=0;k<dz;k++) {
               if (isnan(array[i][j][k])) {
                  array[i][j][k] = dmin-1.0;
               }
            }
         }
      }
   }

   ext->array = array;
   ext->dmin  = dmin;
   ext->dmax  = dmax;
   ext->low   = c.low;
   ext->high  = c.high;
   ext->rdmin = c.rdmin;                /* Read data minimum */
   ext->rdmax = c.rdmax;
   ext->naxes[0] = dx;
   ext->naxes[1] = dy;
   ext->naxes[2] = dz;

   ext->axmax = ext->naxes[0];
   if (ext->naxes[1] > ext->axmax) ext->axmax = ext->naxes[1];
   if (ext->naxes[2] > ext->axmax) ext->axmax = ext->naxes[2];

   ext->vp.x = (float)ext->naxes[0]/(float)ext->axmax;
   ext->vp.y = (float)ext->naxes[1]/(float)ext->axmax;
   ext->vp.z = (float)ext->naxes[2]/(float)ext->axmax;

  
}


void averageCube(FITSCube *avg, FITSCube c, ScaleAxis sa)
{
   int mx = c.naxes[0]/2;
   int my = c.naxes[1]/2;
   int mz = c.naxes[2]/2;
   int ix1 = mx - sa.fx/2;
   int ix2 = mx + sa.fx/2;
   int iy1 = my - sa.fy/2;
   int iy2 = my + sa.fy/2;
   int iz1 = mz - sa.fz/2;
   int iz2 = mz + sa.fz/2;


   long i, j, k;
   long ii, jj, kk;

   float ***array = initVolume(sa.tx, sa.ty, sa.tz, 0.0);

   float sum;
   int ix, iy, iz;

   long cnt = 0;
   ix = 0;
   for (i=ix1;i<ix2;i+=sa.sx) {
      iy = 0;
      for (j=iy1;j<iy2;j+=sa.sy) {
         iz = 0;
         for (k=iz1;k<iz2;k+=sa.sz) {
            sum = 0;
            for (ii=0;ii<sa.sx;ii++) {
               for (jj=0;jj<sa.sy;jj++) {
                  for (kk=0;kk<sa.sz;kk++) {
                     sum += c.array[i+ii][j+jj][k+kk];
                  }
               }
            }
            array[ix][iy][iz] = sum;
            cnt++;
            iz++;
         }
         iy++;
      }
      ix++;
   }


/* NOTE: Probably need to normalise flux values by division */


   float dmin = array[0][0][0];
   if (isnan(dmin)) dmin = 0;
   float dmax = array[0][0][0];
   if (isnan(dmax)) dmax = 0;
   int iflag = 0;
   for (i=0;i<sa.tx;i++) {
      for (j=0;j<sa.ty;j++) {
         for (k=0;k<sa.tz;k++) {
            if (!isnan(array[i][j][k])) {
               if (array[i][j][k] < dmin) dmin = array[i][j][k];
               if (array[i][j][k] > dmax) dmax = array[i][j][k];
            } else {
               iflag++;
            }
         }
      }
   }


/* NOTE: Not sure what this does... */
   if (iflag > 0) {
      for (i=0;i<sa.tx;i++) {
         for (j=0;j<sa.ty;j++) {
            for (k=0;k<sa.tz;k++) {
               if (isnan(array[i][j][k])) {
                  array[i][j][k] = dmin-1.0;
               }
            }
         }
      }
   }

   avg->array = array;
   avg->dmin  = dmin;
   avg->dmax  = dmax;
   avg->low   = dmin;
   avg->high  = dmax;
   avg->rdmin = c.rdmin;		/* Read data minimum */
   avg->rdmax = c.rdmax;
   avg->naxes[0] = sa.tx;
   avg->naxes[1] = sa.ty;
   avg->naxes[2] = sa.tz;

   avg->axmax = avg->naxes[0];
   if (avg->naxes[1] > avg->axmax) avg->axmax = avg->naxes[1];
   if (avg->naxes[2] > avg->axmax) avg->axmax = avg->naxes[2];

   avg->vp.x = (float)avg->naxes[0]/(float)avg->axmax;
   avg->vp.y = (float)avg->naxes[1]/(float)avg->axmax;
   avg->vp.z = (float)avg->naxes[2]/(float)avg->axmax;

}


ScaleAxis setTarget(int target, long naxes[3])
{
   ScaleAxis sa;
   sa.tx = target;
   sa.ty = target;
   sa.tz = target;

   if (naxes[0] < sa.tx) sa.tx = naxes[0];
   if (naxes[1] < sa.ty) sa.ty = naxes[1];
   if (naxes[2] < sa.tz) sa.tz = naxes[2];
   sa.sx = naxes[0]/sa.tx;
   sa.sy = naxes[1]/sa.ty;
   sa.sz = naxes[2]/sa.tz;
   sa.fx = sa.sx*sa.tx;
   sa.fy = sa.sy*sa.ty;
   sa.fz = sa.sz*sa.tz;

   return sa;

}



void num2tid(float number, int dec, unsigned int *tid, float asp, XYZ xyz, 
			float height, int lr, int bt)
{

   char fmt[12];
   char string[32];
   sprintf(fmt, "%%.%df",dec);
   sprintf(string,fmt,number);

   int len = strlen(string);
/*
   width = width/(float)len;
*/

   float y0 = xyz.y, yh = height;   /* xw*asp; */
   float x0 = xyz.x, xw = yh/asp;
   float z0 = xyz.z;

 
   if (lr == ARIGHT) {
      x0 = x0-len*xw;
   }

   if (bt == ATOP) {
      y0 = y0-yh;
   }

   XYZ P[4];
   COLOUR col = { 1,1,1 };
   P[0].x = P[3].x = x0;
   P[1].x = P[2].x = x0+xw;
   P[0].y = P[1].y = y0+yh;
   P[2].y = P[3].y = y0;
   P[0].z = P[1].z = P[2].z = P[3].z = z0;

   int i;
   int idx = 0;
   for (i=0;i<len;i++) {
      if (string[i] == '-') idx = 11;
      else if (string[i] == '.') idx = 10;
      else idx = string[i]-'0';
      ns2vf4x(P, col, tid[idx], 1.0, 'o');
   
      P[0].x = P[3].x = P[0].x + xw;
      P[1].x = P[2].x = P[1].x + xw;
   }

}

unsigned int *getDigitTextures(char *fname, int ndigit, float *asp)
{
   unsigned int *tid = (unsigned int *)calloc(ndigit, sizeof(unsigned int));

   int w, h;
   unsigned int digits = ss2lt(fname);
   unsigned char *tex = ss2gt(digits, &w, &h);
   *asp = (float)h/(float)w;

   int i;
   int ww = (int)((float)w/(float)NDIGIT);
   long idx=0,lidx=0;
   int j, k;
   unsigned char *buf;
   long first, offset;

   for (i=0;i<ndigit;i++) {
      tid[i] = ss2ct(ww,h);		/* Create new texture */
      buf = ss2gt(tid[i], NULL, NULL);	/* Get the texture buffer */

      first  = i*ww;			/* Offset to this digit */
      lidx   = 0;			/* Index within texture buffer */
      for (j=0;j<h;j++) {
         offset = j*w + first;		
         for (k=0;k<ww;k++) {
	    idx = (offset + k)*4;  
            buf[lidx  ] = tex[idx];
            buf[lidx+1] = tex[idx+1];
            buf[lidx+2] = tex[idx+2];
	    lidx+=4;
         }
      }
      ss2pt(tid[i]);			/* Push back the buffer ready for use */
   }

   *asp *= ndigit;
   return tid;
}

/*
int mmain(int argc, char *argv[])
{
   s2opend("/?",argc,argv);
   s2swin(0,1,0,1,0,1);
   s2box("BCDE",0,0,"BCDE",0,0,"BCDE",0,0);

   unsigned int *gtid;		
   float gasp;
   gtid = getDigitTextures("digits2.tga", NDIGIT, &gasp);
   float number = M_PI; 
   XYZ xyz = { 0.0, 0.0, 0.05 };
   float width = 0.3;
   ss2tsc("clr");
   float ar = ss2qar();
   num2tid(number,5, gtid, gasp*ar, xyz, width);

   ss2spt(ORTHOGRAPHIC);
   ss2srm(SHADE_FLAT);
   s2show(1);
   return 1;
}
*/
