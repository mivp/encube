/*
 * shaderService.h
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

#ifndef __ShaderService_H__
#define __ShaderService_H__

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
//#include "s2opengl.h"
#include <GL/glew.h>
//#include <GLUT/glut.h>

#define printOpenGLError() printOglError(__FILE__, __LINE__)

typedef enum {
    EVertexShader,
	EGeometryShader,
    EFragmentShader,
} EShaderType;

typedef struct {
	GLuint vsObject;
	GLuint gsObject;
	GLuint fsObject;
	GLuint prgObject;
	GLchar* vertexShader;
	GLchar* geometryShader;
	GLchar* fragmentShader;
	int iGS;
	int iFS;

} shaderService;

// public call: initialize shaders
int initShaderService(shaderService *ss, char *fileName, int _igs, int _ifs);
// destroy shaders
int destroyShaderService(shaderService *ss);

int install(shaderService *ss, char *fileName);

int readShaderSource(shaderService *ss, char *fileName);

int installShaders(shaderService *ss);

// public functions
GLint getAttriLoc(shaderService *ss, const GLchar* name);
GLint getUniLoc(shaderService *ss, const GLchar* name);

// END: public functions

int shaderSize(char *fileName, int shaderType);
int readShader(char *fileName, int shaderType, char *shaderText, int size);

int printOglError(char* file, int line);
void printShaderInfoLog(GLuint shader);
void printProgramInfoLog(GLuint program);

int testit(shaderService *ss);

#endif
