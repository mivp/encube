/*
 * s2hd.c
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

/* Previously cavehd.c
 * 
 * ImageHD comparative visualisation in the CAVE2
 *
 * Original version: Yuri Benovitski, summer 2013/14
 *   with contributions from Chris Fluke, David G Barnes
 *
 * Migrated from Yuribinread11dgb.c to cavehd.c in August 2014
 *
 * And carried forward by David G Barnes
 *   with contributions from Owen Kaluza, Tszho (Leo) Wong
 *
 * Modified version by Dany Vohl, 2015.
 *      - stabilised code
    *      doesn't return(1) when something goes wrong anymore on load files,
    *      some other logical issues such as clearing different memory twice
 *      - added a new remote callback that returns data to client
 *      - new callback functions:
    *     In remote_cb:
        *     'h': highlight screen
        *     'l': load file with parameters passed through socket (gets rid of the textfiles for each node)
        *     'u': unload either specific screen or all screens (the latter is similar to 'U')
    *   In remote_cb_sock:
        *     'H': request histogram and sends it out through socket to client
        *     'S': sends state of different screens to client (subject_id)
 *
 */

/*
 This code reads a binary file which was generated in Matlab from .trk file
 To genrate the .bin file use VLSCI_01.m Matlab code.
 The .bin is of the following format:
 
 nt                      - number of tracks
 np(1)                   - number of points in track
 np(2)
 .
 .
 np(nt)
 
 X(P1) Y(P1) Z(P1)       - TRACK 1 xyz coordinates for each point
 X(P2) Y(P2) Z(P2)
 .
 .
 X(Pnt) Y(Pnt) Z(Pnt)
 
 X(P1) Y(P1) Z(P1)       - TRACK 2 xyz coordinates for each point
 X(P2) Y(P2) Z(P2)
 .
 .
 X(Pnt) Y(Pnt) Z(Pnt)
 .
 .
 .
 
 Yuribinread4a.c has a remote callback written by David. This writes camera position parameters and reads them
                 from remote terminal. Run with "perl runit.pl" to open 2 slave and 1 master columns.
 
 Yuribinread5 is a combination of Yuribinread4a.c and yuri_chris.c
                 which has 4 internal call backs for each panel and a keypress call back
                 to update .trk file names from an input.txt file.
 
 
 Further tasks:
 1. Try out the derivitive function David sent me
 2. Fix the skipping bug (CJF suggested to have a look on Monday)
 3. Fix the "U" bug (CJF said he has few ideas)
 
 Yuribinread6 is fixing the "U" bug.
                CJF suggested to use the original Key-CallBack and change Perl to wait for K
                instead of U while writing 'Ku' to terminal.
 
// Version 9dgb: adding XRW support preferred over binary matlib version.

 Function Structure:
 1. int main(int argc, char *argv[])
 2. PanelData initPanel(int N, float cen, float width, float height)
 a. int kbdcb(unsigned char *key)
 3. void cb0/1/2/3(double *t, int *kc)
 4. void doit(int id)
 5. void loadData(int id)
 a. void freeData(int id)
 6. void Directional_Color(int id, int start, int track, int len)
 7. void drawData(int id)
 
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <time.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "s2plot.h"
#include "s2globals.h"

#include "hdsupport.h"
#include "hdglobals.h"

#include "s2opengl.h"
#include "s2glut.h"



void drawData(int id, double *t)
{
  //ss2srm(SHADE_FLAT);             // Set the rendering mode.
  COLOUR amb = {0.7, 0.7, 0.7};   // This is the color after pressing 'R'
  ss2sl(amb, 0, NULL, NULL, 0);   // Set the entire lighting environment

  // decorate the panel
  if (_doFrames) {

    // show the subject id (and maybe more in future e.g. gender, age)
    unsigned int infotexid = pd[id].infotexid;
    float infoaspect = pd[id].infoaspect;
    ss2tsc("clr");
#define INFOHEIGHT 0.03
#define INFOOFFSET 0.01
#define INFOVCORNER (1.0-INFOOFFSET)
    //#define INFOHCORNER (0.5+0.5*INFOHEIGHT*infoaspect)
#define INFOHCORNER (0.0+1.0*INFOHEIGHT*infoaspect)
    XYZ infoP[] = {{INFOHCORNER-INFOHEIGHT*infoaspect, INFOVCORNER, 0.1},
                   {INFOHCORNER, INFOVCORNER, 0.1},
                   {INFOHCORNER, INFOVCORNER-INFOHEIGHT, 0.1},
                   {INFOHCORNER-INFOHEIGHT*infoaspect, INFOVCORNER-INFOHEIGHT, 0.1}};


    // HARDCODE: colour choice based on subject type
    COLOUR infoCol = {1.,1.,1.}; // white default
    switch(pd[id].type) {
    case 'C':
      infoCol = C_GREEN;
      break;
    case 'P':
      infoCol = C_BLUE;
      break;
    case 'S':
      infoCol = C_ORANGE;
      break;
    }
    //static COLOUR infoCol = {1.0, 0.5, 0.2};
    ns2vf4x(infoP, infoCol, infotexid, 1, 'o');
    ss2tsc("");

  }
}

void drawit(int id, double *t)
{
    if (pd[id].state == EMPTY) {  /* No data to draw*/
        XYZ xyz = { 0.5, 0.5, 0.5 };
        XYZ off = { 0.01, 0.01, 0. };
        //COLOUR e_COL = {0.5, 0.5, 0.1};

        //ds2vbbpr(xyz, off, e_aspect, 1.02, 0.0, e_COL, empty_texid, 1.0, 'o');

        s2slw(4.0);
        s2sch(0.4);
        s2sci(S2_PG_ORANGE);
        ds2vtb(xyz, off, APPLABEL, 1);// Draw text that always faces the camera - vector input
        s2sch(1.0);
        s2slw(1.0);
    } else if (pd[id].state == PREDRAW)
    {
        s2scir(CBASE,CBASE+CRANGE);// Set the range of colour indices used for shading
        s2icm(_cmaps[_i_cmap], CBASE, CBASE+CRANGE);

        pd[id].state = DRAW;

        if (pd[id].infotexid > 0) {
          ss2dt(pd[id].infotexid);
        }
        char tmp[10];
        sprintf(tmp, "%d", pd[id].id);
        //pd[id].infotexid = font2tex(FONT, pd[id].TRKfilename, 36, 4);
        pd[id].infotexid = font2tex(FONT, tmp, 36, 4);
        int w, h;
        ss2gt(pd[id].infotexid, &w, &h);
        pd[id].infoaspect = (float)w / (float)h;

        if (config.save_time)
        {
            // stop loading timer
            gettimeofday(&pd[id].stop_time, NULL);
            pd[id].elapsed_time = (pd[id].stop_time.tv_sec - pd[id].start_time.tv_sec) * 1000.0; // sec to ms
            pd[id].elapsed_time += (pd[id].stop_time.tv_usec - pd[id].start_time.tv_usec) / 1000.0;

            //FILE *fp = fopen(config.time_log_dir_filename.c_str(), "a"); // Open file for writing
            fprintf(stderr, "Load time for panel %d (s): %f\n", id, pd[id].elapsed_time/1000);
            //fprintf(fp, "Load time for panel %d (s): %f\n", id, pd[id].elapsed_time/1000);
            //fclose(fp);
        }

    } else if (pd[id].state == DRAW)
    {
        drawData(id, t);
    }

    // show the blinking circle / square to indicate "live"
    static double base_t =  -1.0;
    if (base_t < 0) {
        base_t = *t;
    }

#define FPERIOD 1.0
    double f_offt = ((*t - base_t) * FPERIOD);
    long i_offt = (long)truncl(f_offt);
    double phase = (f_offt - (double)i_offt) / FPERIOD;
    static int blink_texid = -1;
    if (blink_texid < 0) {
        blink_texid = ss2lt("halo32.tga");
    }

    if (phase > 0.5)
    {
        ss2tsc("clr");
#define SIDE 0.01
        XYZ P[4] = {{0.5*SIDE/ss2qar(),0.5*SIDE,0.1},
            {0.5*SIDE/ss2qar(),0.5*SIDE+SIDE,0.1},
            {0.5*SIDE/ss2qar()+SIDE/ss2qar(),0.5*SIDE+SIDE,0.1},
            {0.5*SIDE/ss2qar()+SIDE/ss2qar(),0.5*SIDE,0.1}};
        ns2vf4(P, C_GREEN);
        ss2tsc("");
    }

    if (pd[id].selected)
    {
        ss2tsc("clr");

        // Draw rectangle
        // ns2thline(x1,y1,z1, x2,y2,z2, r,g,b, size);

        //bottom horizontal line
        ns2thline(0.,0.,0.1, 1.,0.,0.1, 0.801960784,0.01372549,0.27254902, 5);
        // top horizontal line
        ns2thline(0.,1.,0.1, 1.,1.,0.1, 0.801960784,0.01372549,0.27254902, 5);
        // left vertical line
        ns2thline(0.,0.,0.1, 0.,1.,0.1, 0.801960784,0.01372549,0.27254902, 5);
        // right vertical l line
        ns2thline(1.,0.,0.1, 1.,1.,0.1, 0.801960784,0.01372549,0.27254902, 5);

        ss2tsc("");
    }
}

void cb0(double *t, int *kc)
// This is a callback for upper panel
{
    if (master)
    {
#define FEPS 0.00001
        //fprintf(stdout, "Got to CallBack0\n");
        // fetch and print camera position
        static XYZ svpos = {0,0,0}, svup = {0,0,0}, svvdir = {0,0,0};
        XYZ pos, up, vdir;

        ss2qc(&pos, &up,  &vdir, 1);  // Query the camera position, up vector and view direction
        if ((VectorLength(pos, svpos) > FEPS) || (VectorLength(up, svup) > FEPS) ||
            (VectorLength(vdir, svvdir) > FEPS))
        {
            fprintf(stdout, "C %f %f %f %f %f %f %f %f %f\n",
                    pos.x, pos.y, pos.z, up.x, up.y, up.z, vdir.x, vdir.y, vdir.z);
            //fprintf(stderr, "C %f %f %f %f %f %f %f %f %f\n",
                    //pos.x, pos.y, pos.z, up.x, up.y, up.z, vdir.x, vdir.y, vdir.z);
            svpos = pos;
            svup = up;
            svvdir = vdir;
            //float matrix[16];
            //glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
            //fprintf(stderr, "Modelview Matrix: \n");
            //for(int i = 0; i < 4; i++)
            //{
            //fprintf(stderr, "%f, %f, %f, %f\n", matrix[i], matrix[i+4], matrix[i+8], matrix[i+12]);
            //}
        }
        if (camset)
        {
            camset = 0;
        }
        //GL_Error_Check;
        prepit(0);

        if (pd[0].state == PREDRAW)
        {
            buildBufferAndTexture(0);
        }

        if(trackUpdateByRemoteFlag == 1)
        {
            updateTracksByRemoteControl();
            trackUpdateByRemoteFlag = 0;
        }

    } else
    { // slaves

        int id;

	if(config.use_processed_track)
	{
#pragma omp parallel for
            for (id = 0; id < 4; id++)
            {
                prepit(id);
            }
	}
	else
	    for (id = 0; id < 4; id++)
            {   
                prepit(id);
            }

        for(id = 0; id < 4; id++)
        {
            if (pd[id].state == PREDRAW)
            {
                buildBufferAndTexture(id);
            }
        }

        if(trackUpdateByRemoteFlag == 1)
        {
            updateTracksByRemoteControl();
            trackUpdateByRemoteFlag = 0;
        }
    }

    if(rgc.loadFlags[0])
    {
        bindRegionTex(1, rgc.act_mask1);
        rgc.loadFlags[0] = false;
    }
    if(rgc.loadFlags[1])
    {
        bindRegionTex(2, rgc.act_mask2);
        rgc.loadFlags[1] = false;
    }

    drawit(0, t);
}

void cb1(double *t, int *kc)
{
    int id = 1;
    drawit(id, t);
}

void cb2(double *t, int *kc)
{
    int id = 2;
    drawit(id, t);
}

void cb3(double *t, int *kc)
{
    int id = 3;
    drawit(id, t);
}

PanelData initPanel(int panel_number, float cen, float width, float height)
{
    //fprintf(stderr, "InitPanel");
    PanelData lpd; // lpd is type PenelData
        /* Move or create the next panel */
        if (panel_number == 0)
        {
            lpd.pid = 0;  // panel id
            xs2mp(lpd.pid, 0.5-width, cen-height, 0.5+width, cen+height); //Modify location of existing panel
        }
        else
        {
            lpd.pid = xs2ap(0.5-width, cen-height, 0.5+width, cen+height);  //Add a new panel
        }

    /* Choose panel and set coordinates */
    xs2cp(lpd.pid);  // Select a panel for subsequent geometry calls

    //fprintf(stderr,"Got to apply colour \n");
    s2scr(11111, 1, 1, 1);  // Apply white colour (reset)
    s2sci(11111);

    //int Edge = 255;
    //s2swin(0,Edge, 0,Edge,0,Edge);                 // Set the window coordinates
    s2swin(swinMin.x, swinMax.x, swinMin.y, swinMax.y, swinMin.z, swinMax.z);   // Set the window coordinates
    /* Hard-coded forcing callbacks for each panel */
    switch (panel_number) {
        case 0 : cs2scb((void*)cb0); break;  // Install callback
        case 1 : cs2scb((void*)cb1); break;
        case 2 : cs2scb((void*)cb2); break;
        case 3 : cs2scb((void*)cb3); break;
        // the callbacks doing the plotting!!
    }
    // cs2skcb(kbdcb);      /* Keyboard callback to force updates */

    cs2skcb((void*)kbdcb);  // Keyboard callback to force updates
    cs2srcb((void*)remote_cb); // Remote Callback recieves commands from Perl/Python
    cs2srcb_sock((void*)remote_cb_sock); // Remote Callback that requires to send back data to client via fprintf
    //cs2srcb_sock_write((void*)remote_cb_sock_write); // Remote callback that requires to send back data to client via write

    cs2socb((void*)oglDraw);
    //cs2socb(oglDraw);

    /* Set defaults for this panel */
    lpd.state = EMPTY;
    sprintf(lpd.TRKfilename,"%s","");
    sprintf(lpd.NIIfilename,"%s","");
    sprintf(lpd.FITS_filename,"%s","");
    lpd.id = -1;
    lpd.array = NULL;
    lpd.ntracks = 0;
    lpd.tracklens = NULL;
    lpd.npoints = 0;
    lpd.points = NULL;
    lpd.lineVerts = NULL;
    lpd.lineVertsAdj = NULL;
    lpd._geometryTex_id = 0;
    lpd._geometryTex_data = NULL;
    lpd.lineVertsVBOid = 0;
    lpd.trackFlags = NULL;
    lpd.trackFlagsAdj = NULL;
    lpd.trackFlagsVBOid = 0;
    lpd.trackColor = NULL;
    lpd.trackColorAdj = NULL;
    lpd.trackColorVBOid = 0;
    lpd.nbins = 10;
    lpd.histogram  = NULL;
    lpd.dmin=FLT_MAX;
    lpd.dmax=FLT_MIN;
    lpd.selected = 0;


    return lpd;
}

int main(int argc, char *argv[])
{
    char configfile[MAXSTRING];

    /* Filename containing input data sets */
    if (argc == 3) {    // if main has 2 inputs: 0/1 for master/slave and inputX.txt filename
        master = atoi(argv[1]); // convert string to integer
        sprintf(configfile,"%s",argv[2]);
        
    } else if (argc == 2) {
        master = atoi(argv[1]);
        sprintf(configfile,"config/default.json"); // input file works with 4 file names at the moment. Should ready multiple input files
    } else {
        master = 0;
        sprintf(configfile,"config/default.json");
    }
    
    if (master) {
        fprintf(stderr,"I am the Master\n");
        s2opend("/?", argc, argv);
    } else {
        s2opend("/?", argc, argv);
    }
    //  File names are passed to plot_tracks function in an array form:
    //    plot_tracks(sub_array);

    //load cofig
    loadConfig(configfile);
    
    // absolute initialisation of panels
    for (int i = 0; i < MAXPANEL; i++) {
        zeroPanel(i);
    }

    initGL();
    initTrackFraction();

    /* Locations of panels */
    // Used to be hard coded:
    float cen[MAXPANEL] = { 0.875, 0.625, 0.375, 0.125 };
    float height = 0.125;
    float width = 0.5;
    /*float cen[MAXPANEL];
    float increment = 1/(float)MAXPANEL;
    float height = increment/2;
    float current = 1.;
    for (int i = 0; i<MAXPANEL; i++) {
        cen[i] = current - height;
        current -= increment;
    }*/

    /* Create the panels */
    int panel_number = 0;
    if (master) // init 1 panel
    {
        s2scr(11111, 1, 1, 1);  // Apply white colour (reset)
        s2sci(11111);
        //char xopt[] = "BCDETMNOPQ";
        //char yopt[] = "BCDETMNOPQ";
        //char zopt[] = "BCDETMNOPQ";
        s2svp(-1,1,-1,1, -1,1);
        //s2svp(10,14,-5,3, 0,8);
        s2swin(swinMin.x, swinMax.x, swinMin.y, swinMax.y, swinMin.z, swinMax.z);   // Set the window coordinates
        //s2box(xopt, 0,0, yopt, 0,0, zopt, 0,0);
        //s2lab("X-axis","Y-axis","Z-axis","Plot");

        cs2scb((void*)cb0);        // Install CallBack

        fprintf(stdout,"After cs2scb\n");

        /* Set defaults for this panel */
        PanelData lpd;
        lpd.state = EMPTY;
        sprintf(lpd.TRKfilename,"%s","");
        sprintf(lpd.NIIfilename,"%s","");
        sprintf(lpd.FITS_filename,"%s","");
        lpd.ntracks = 0;
        lpd.tracklens = NULL;
        lpd.npoints = 0;
        lpd.points = NULL;
        pd[0] = lpd;
        zeroPanel(0);

        fprintf(stdout,"cs2skcb\n");
        cs2skcb((void*)kbdcb);      // Keyboard callback to force updates

        fprintf(stdout,"cs2srcb\n");
        cs2srcb((void*)remote_cb);  // Remote Callback recieves commands from Perl
        cs2srcb_sock((void*)remote_cb_sock); // Remote Callback that requires to send back data to client via fprintf
        //cs2srcb_sock_write((void*)remote_cb_sock_write); // Remote callback that requires to send back data to client via write

        fprintf(stdout,"cs2socb\n");
        cs2socb((void*)oglDraw);
    }
    else
    {
        for (panel_number=0;panel_number<MAXPANEL;panel_number++)
        {
            pd[panel_number] = initPanel(panel_number, cen[panel_number], width, height);
            if (config.app_type == APP_ASTRO_FITS)
            {
                char xopt[] = "BCDETOQ";
                char yopt[] = "BCDETOQ";//"BDETMNOPQ";
                char zopt[] = "BCDETOQ";//"BDETMNOPQ";

                s2box(xopt, 0,0, yopt, 0,0, zopt, 0,0);	/* Draw the coordinate box */
                s2lab("RA","DEC","Velocity", "");	/* Write some labels */
            }
        }
    }
    
    if (!master) {
      xs2cp(pd[0].pid);   //Select a panel for subsequent geometry calls
    }
    
    // argh! - kbd and remote callbacks are PER PANEL. - moved to initPanel code
    //cs2skcb((void*)kbdcb);		// Keyboard callback to force updates
    //cs2srcb((void*)remote_cb); // Remote Callback recieves commands from Perl
    //cs2srcb_sock((void*)remote_cb_sock); // Remote Callback that requires to send back data to client (DV)
    if (!master) {
        // link panel cameras
        xs2lpc(panel1, panel2);        // Link panels together
        xs2lpc(panel2, panel3);        // Link panels together
        xs2lpc(panel3, panel4);        // Link panels together
    }

    // unbuffer stdout
    setvbuf(stdout, NULL, _IONBF, 0); /* defines how a stream should be buffered.
                                         "_IONBF" No buffer is used.
                                         Each I/O operation is written as soon as possible.
                                         The buffer and size parameters are ignored. */

    initRegionControl();
    initMasks();
    rgc.act_mask1 = -1;
    rgc.act_mask2 = -1;
    
#ifdef GL_VERSION_3_2
    //fprintf(stderr, "OpenGL 3.2 supported!!!!! \n");
#endif
    
    if(glewIsSupported("GL_NVX_gpu_memory_info"))
    {
        int avaiMem = 0;
        glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &avaiMem);
        if(avaiMem > gMemRequired)
            bufferAllFlag = true;
    }

    //testJson();
    s2show(1);  // Open the s2plot window

    //clean up
    if ( mask_all_xrw )
        deleteXraw(mask_all_xrw);
    if ( mask_temp_xrw )
        deleteXraw(mask_temp_xrw);

    return 0;
    //return 1;   // chris has return 0. Don't think it make a difference
}
