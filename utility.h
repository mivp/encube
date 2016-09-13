/*
 * utility.h
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

#if !defined(_UTILITY_H)
#define _UTILITY_H

#define inline

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <GL/glew.h>

//Defines a 32bit colour accessible as rgba byte array, integer and r,g,b,a component struct
typedef union {
    GLubyte rgba[4];
    int value;
    struct {
        GLubyte r;
        GLubyte g;
        GLubyte b;
        GLubyte a;
    };
} Colour;

typedef union
{
    float a[3];
    struct
    {
        float x, y, z;
    };
} float3;

// coloum-row fashion
typedef union
{
    float a[16];
    float d[4][4];
} mat4x4;

typedef struct
{
	float3 min;
	float3 max;
} AABB;

inline float3 make_float3(float x, float y, float z);
inline float3 make_float3(float s);
inline float3 operator+(float3 a, float3 b);
inline float3 operator+(float3 a, float b);
inline float3 operator+(float b, float3 a);
inline void operator+=(float3 &a, float3 b);
inline void operator+=(float3 &a, float b);

inline float3 operator-(float3 a, float3 b);
inline float3 operator-(float3 a, float b);
inline float3 operator-(float b, float3 a);
inline void operator-=(float3 &a, float3 b);
inline void operator-=(float3 &a, float b);

inline float3 operator*(float3 a, float3 b);
inline float3 operator*(float3 a, float b);
inline float3 operator*(float b, float3 a);
inline void operator*=(float3 &a, float3 b);
inline void operator*=(float3 &a, float b);

inline float3 operator/(float3 a, float3 b);
inline float3 operator/(float3 a, float b);
inline float3 operator/(float b, float3 a);
inline void operator/=(float3 &a, float3 b);
inline void operator/=(float3 &a, float b);

inline bool operator==(float3 &a, float3 &b);
inline bool operator!=(float3 &a, float3 &b);

inline float dotprod(float3 &a, float3 &b);
inline float length(float3 v);
inline float3 normalize(float3 &v);
inline float3 reflect(float3 &i, float3 &n);
inline float3 cross(float3 &a, float3 &b);

inline mat4x4 inverseTranspose(mat4x4 m);


inline AABB make_aabb(float3 _min, float3 _max);
bool intersect(AABB *out, AABB *a, AABB *b);

#endif


