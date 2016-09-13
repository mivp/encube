/*
 * shaderService.c
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

#include "shaderService.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <GL/glew.h>

// Error Checking functions -----------------------------------------------------------
int printOglError(char* file, int line)
{
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    GLenum glErr;
    int    retCode = 0;
    
    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        //fprintf(stderr, "glError 0x%x file %s @ %d: %s\n", glErr, file, line, gluErrorString(glErr));
        fprintf(stderr, "glError 0x%x file %s @ %d\n", glErr, file, line);
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}

void printShaderInfoLog(GLuint shader)
{
    int infologLength = 0;
    int charsWritten  = 0;
    GLchar *infoLog;
    
    printOpenGLError();  // Check for OpenGL errors
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);
    printOpenGLError();  // Check for OpenGL errors
    
    if (infologLength > 0)
    {
        infoLog = (GLchar *)malloc(infologLength);
        if (infoLog == NULL)
        {
            fprintf(stderr, "ERROR: Could not allocate InfoLog buffer\n");
            exit(1);
        }
        glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
        fprintf(stderr, "Shader InfoLog:\n%s\n\n", infoLog);
        free(infoLog);
    }
    printOpenGLError();  // Check for OpenGL errors
}

void printProgramInfoLog(GLuint program)
{
    int infologLength = 0;
    int charsWritten  = 0;
    GLchar *infoLog;
    
    printOpenGLError();  // Check for OpenGL errors
    
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);
    
    printOpenGLError();  // Check for OpenGL errors
    
    if (infologLength > 0)
    {
        infoLog = (GLchar *)malloc(infologLength);
        if (infoLog == NULL)
        {
            fprintf(stderr, "ERROR: Could not allocate InfoLog buffer\n");
            exit(1);
        }
        glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
        fprintf(stderr, "Program InfoLog:\n%s\n\n", infoLog);
        free(infoLog);
    }
    printOpenGLError();  // Check for OpenGL errors
}
// END: Error Checking functions -----------------------------------------------------------

int initShaderService(shaderService *ss, char *fileName, int _igs, int _ifs)
{
//    ss = (shaderService*)malloc(sizeof(shaderService));
    ss->iGS = _igs;
    ss->iFS = _ifs;
    ss->vertexShader = ss->geometryShader = ss->fragmentShader = NULL;
    
    int success = install(ss, fileName);
    if(!success) exit(0);
    
    return 1;
}

int destroyShaderService(shaderService *ss)
{
    if(ss->vertexShader != NULL) free(ss->vertexShader);
    if(ss->geometryShader != NULL) free(ss->geometryShader);
    if(ss->fragmentShader != NULL) free(ss->fragmentShader);
    if(ss != NULL) free(ss);
    return 1;
}

int install(shaderService *ss, char *fileName)
{
    readShaderSource(ss, fileName);
    installShaders(ss);
    return 1;
}

int shaderSize(char *fileName, EShaderType shaderType)
{
    // Returns the size in bytes of the shader fileName.
    // If an error occurred, it returns -1.
    // File name convention: <fileName>.vert <fileName>.geom <fileName>.frag
    int fd, count = -1;
    char name[100];
    
    strcpy(name, fileName);
    
    switch (shaderType)
    {
        case EVertexShader:
            strcat(name, ".vert");
            break;
		case EGeometryShader:
            strcat(name, ".geom");
            break;
        case EFragmentShader:
            strcat(name, ".frag");
            break;
        default:
            fprintf(stderr, "ERROR: unknown shader file type\n");
            exit(1);
            break;
    }
    
    // Open the file, seek to the end to find its length
#ifdef WIN32 /*[*/
    fd = _open(name, _O_RDONLY);
    if (fd != -1)
    {
        count = _lseek(fd, 0, SEEK_END) + 1;
        _close(fd);
    }
#else /*][*/
    fd = open(name, O_RDONLY);
    if (fd != -1)
    {
        count = lseek(fd, 0, SEEK_END) + 1;
        close(fd);
    }
#endif /*]*/
    
    return count;
}

int readShader(char *fileName, EShaderType shaderType, char *shaderText, int size)
{
    // Reads a shader from the supplied file and returns the shader in the
    // arrays passed in. Returns 1 if successful, 0 if an error occurred.
    // The parameter size is an upper limit of the amount of bytes to read.
    // It is ok for it to be too big.
    FILE *fh;
    char name[100];
    int count;
    
    strcpy(name, fileName);
    
    switch (shaderType)
    {
        case EVertexShader:
            strcat(name, ".vert");
            break;
		case EGeometryShader:
            strcat(name, ".geom");
            break;
        case EFragmentShader:
            strcat(name, ".frag");
            break;
        default:
            fprintf(stderr, "ERROR: unknown shader file type\n");
            exit(1);
            break;
    }
    
    // Open the file
    fh = fopen(name, "r");
    if (!fh) return -1;
    
    // Get the shader from a file.
    fseek(fh, 0, SEEK_SET);
    count = (int) fread(shaderText, 1, size, fh);
    shaderText[count] = '\0';
    
    if (ferror(fh)) count = 0;
    
    fclose(fh);
    return count;
}

int readShaderSource(shaderService *ss, char *fileName)
{
    int vSize = 0, gSize = 0, fSize = 0;
    GLchar **vertexShader = &(ss->vertexShader);
    GLchar **geometryShader = &(ss->geometryShader);
    GLchar **fragmentShader = &(ss->fragmentShader);
    
    // Allocate memory to hold the source of our shaders.
    vSize = shaderSize(fileName, EVertexShader);
	if(ss->iGS) gSize = shaderSize(fileName, EGeometryShader);
    if(ss->iFS) fSize = shaderSize(fileName, EFragmentShader);
    fprintf(stderr, "vsize: %d, fsize: %d\n", vSize, fSize);
    
    if ((vSize == -1) || (fSize == -1))
    {
        fprintf(stderr, "Cannot determine size of the shader %s\n", fileName);
        return 0;
    }
	if(ss->iGS)
	{
		if (gSize == -1)
		{
			fprintf(stderr, "Cannot determine size of the geometry shader %s\n", fileName);
			return 0;
		}
	}
    
    *vertexShader = (GLchar *) malloc(vSize);
	if(ss->iGS) *geometryShader = (GLchar *) malloc(gSize);
    if(ss->iFS) *fragmentShader = (GLchar *) malloc(fSize);
    
    // Read the source code
    if (!readShader(fileName, EVertexShader, *vertexShader, vSize))
    {
        fprintf(stderr, "Cannot read the file %s.vert\n", fileName);
        return 0;
    }
    
	if(ss->iGS)
	{
		if (!readShader(fileName, EGeometryShader, *geometryShader, gSize))
		{
			fprintf(stderr, "Cannot read the file %s.vert\n", fileName);
			return 0;
		}
	}
    
    if(ss->iFS)
    {
        if (!readShader(fileName, EFragmentShader, *fragmentShader, fSize))
        {
            fprintf(stderr, "Cannot read the file %s.frag\n", fileName);
            return 0;
        }
    }
    return 1;
}

int installShaders(shaderService *ss)
{
 
    GLint vertCompiled = 0, geomCompiled = 0, fragCompiled = 0;    // status values
    GLint linked = 0;
    
    // Create a vertex shader object and a fragment shader object
    ss->vsObject = glCreateShader(GL_VERTEX_SHADER);
	if(ss->iGS) ss->gsObject = glCreateShader(GL_GEOMETRY_SHADER);
    if(ss->iFS) ss->fsObject = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Load source code strings into shaders
    glShaderSource(ss->vsObject, 1, (const GLchar * *)&(ss->vertexShader), NULL);
	if(ss->iGS) glShaderSource(ss->gsObject, 1, (const GLchar * *)&(ss->geometryShader), NULL);
    if(ss->iFS) glShaderSource(ss->fsObject, 1, (const GLchar * *)&(ss->fragmentShader), NULL);
  
    // Compile the vertex shader, and print out the compiler log file.
    glCompileShader(ss->vsObject);
    printOpenGLError();
    glGetShaderiv(ss->vsObject, GL_COMPILE_STATUS, &vertCompiled);
    printShaderInfoLog(ss->vsObject);
 
	if(ss->iGS)
	{
		glCompileShader(ss->gsObject);
		printOpenGLError();  // Check for OpenGL errors
		glGetShaderiv(ss->gsObject, GL_COMPILE_STATUS, &geomCompiled);
		printShaderInfoLog(ss->gsObject);
	}
    
    // Compile the fragmen shader, and print out the compiler log file.
    if(ss->iFS)
    {
        glCompileShader(ss->fsObject);
        printOpenGLError();  // Check for OpenGL errors
        glGetShaderiv(ss->fsObject, GL_COMPILE_STATUS, &fragCompiled);
        printShaderInfoLog(ss->fsObject);
    }
    
    if (!vertCompiled || !fragCompiled) return 0;
	if(ss->iGS)
	{
		if(!geomCompiled) return 0;
	}
 
    // Create a program object and attach compiled shaders
    ss->prgObject= glCreateProgram();
    glAttachShader(ss->prgObject, ss->vsObject);
	if(ss->iGS) glAttachShader(ss->prgObject, ss->gsObject);
    if(ss->iFS) glAttachShader(ss->prgObject, ss->fsObject);
    
    // Link the program object and print out the info log
    glLinkProgram(ss->prgObject);
    printOpenGLError();  // Check for OpenGL errors
    glGetProgramiv(ss->prgObject, GL_LINK_STATUS, &linked);
    printProgramInfoLog(ss->prgObject);
    
    if (!linked) return 0;

    return 1;
}

// public functions
GLint getAttriLoc(shaderService *ss, const GLchar* name)
{
    GLint loc = glGetAttribLocation(ss->prgObject, name);
    
    if (loc == -1) fprintf(stderr, "No such uniform named \"%s\"\n", name);
    
    printOpenGLError();  // Check for OpenGL errors
    return loc;
}

GLint getUniLoc(shaderService *ss, const GLchar* name)
{
    GLint loc = glGetUniformLocation(ss->prgObject, name);
    
    if (loc == -1) fprintf(stderr, "No such uniform named \"%s\"\n", name);
    
    printOpenGLError();  // Check for OpenGL errors
    return loc;
}

int testit(shaderService *ss)
{
    return ss->vsObject;
}
