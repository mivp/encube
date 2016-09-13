#include <GL/gl.h>
#include <GL/glu.h>
#include <png.h>
#include <cstdio>
#include <string>

#define TEXTURE_LOAD_ERROR 0

using namespace std;

GLuint loadColourmapTexture(const string filename);