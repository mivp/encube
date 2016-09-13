/*
 * hdglobals.h
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

#if !defined(_HDGLOBALS_H)
#define _HDGLOBALS_H

extern int gMemRequired;
extern float3 swinMin, swinMax;
extern HDCONFIG config;
extern bool need_reloaded_brain;
extern bool brain_loaded[MAXPANEL];
extern XRAW_STRUCT *mask_all_xrw;
extern XRAW_STRUCT *mask_temp_xrw;
extern vector<int> mask_ids;
extern int update;

extern char TRKfilename[MAXPANEL*2][MAXSTRING];
extern char NIIfilename[MAXPANEL*2][MAXSTRING];
extern char FITS_filename[MAXPANEL*2][MAXSTRING];
extern char inputextfile[MAXSTRING];
extern PanelData pd[MAXPANEL]; // array of 4 panelData variables
extern int updateflag;

extern char APPLABEL[MAXSTRING];

extern int _n_cmaps;
extern int _i_cmap;
extern char *_cmaps[];
extern float _core_alpha;
extern float _track_alpha;

extern bool bufferAllFlag;

extern float lFraction;
extern float sFraction;

extern float ps;

extern GLfloat scnTriPosArr[];

// for shader rendering
extern shaderService *vrShaders;
extern shaderService *trShaders;
extern shaderService *rgShaders;

extern int VBOdraw;
extern int trackUpdateByRemoteFlag;

extern float volumeScale;

// parameters from web
extern AABB clpb, aabb;
extern float viewport_width, viewport_height;

extern volumeProperty vp;

extern COLOUR C_GREEN, C_BLUE, C_ORANGE, C_GREY;

extern int master;

extern int nt;

extern int camset;
extern XYZ setpos, setup, setvdir;

extern RegionControl rgc;

extern float line_thick;

extern int _doFrames;
extern int DIVO;



#endif
