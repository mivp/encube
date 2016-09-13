/*
 * utility.cpp
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

#include "utility.h"

inline float3 make_float3(float x, float y, float z)
{
    float3 v;
    v.x = x; v.y = y; v.z = z;
    return v;
}

inline float3 make_float3(float s)
{
    return make_float3(s, s, s);
}

// operator - & -=
inline float3 operator-(float3 a, float3 b)
{
    return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline float3 operator-(float3 a, float b)
{
    return make_float3(a.x - b, a.y - b, a.z - b);
}

inline float3 operator-(float b, float3 a)
{
    return make_float3(b - a.x, b - a.y, b - a.z);
}

inline void operator-=(float3 &a, float3 b)
{
    a.x -= b.x; a.y -= b.y; a.z -= b.z;
}

inline void operator-=(float3 &a, float b)
{
    a.x -= b; a.y -= b; a.z -= b;
}
// END: operator - & -=

// operator + & +=
inline float3 operator+(float3 a, float3 b)
{
    return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline float3 operator+(float3 a, float b)
{
    return make_float3(a.x + b, a.y + b, a.z + b);
}

inline float3 operator+(float b, float3 a)
{
    return make_float3(b + a.x, b + a.y, b + a.z);
}

inline void operator+=(float3 &a, float3 b)
{
    a.x += b.x; a.y += b.y; a.z += b.z;
}

inline void operator+=(float3 &a, float b)
{
    a.x += b; a.y += b; a.z += b;
}
// END: operator + & +=

// operator * & *=
inline float3 operator*(float3 a, float3 b)
{
    return make_float3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline float3 operator*(float3 a, float b)
{
    return make_float3(a.x * b, a.y * b, a.z * b);
}

inline float3 operator*(float b, float3 a)
{
    return make_float3(b * a.x, b * a.y, b * a.z);
}

inline void operator*=(float3 &a, float3 b)
{
    a.x *= b.x; a.y *= b.y; a.z *= b.z;
}

inline void operator*=(float3 &a, float b)
{
    a.x *= b; a.y *= b; a.z *= b;
}
// END: operator * & *=

// operator / & /=
inline float3 operator/(float3 a, float3 b)
{
    return make_float3(a.x / b.x, a.y / b.y, a.z / b.z);
}

inline float3 operator/(float3 a, float b)
{
    return make_float3(a.x / b, a.y / b, a.z / b);
}

inline float3 operator/(float b, float3 a)
{
    return make_float3(b / a.x, b / a.y, b / a.z);
}

inline void operator/=(float3 &a, float3 b)
{
    a.x /= b.x; a.y /= b.y; a.z /= b.z;
}

inline void operator/=(float3 &a, float b)
{
    a.x /= b; a.y /= b; a.z /= b;
}
// END: operator / & /=

inline bool operator==(float3 &a, float3 &b)
{
    return (a.x == b.x && a.y == b.y && a.z == b.z);
}

inline bool operator!=(float3 &a, float3 &b)
{
	return (a.x != b.x || a.y != b.y || a.z != b.z);
}

inline float dotprod(float3 &a, float3 &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float3 fminf(float3 a, float3 b)
{
	return make_float3(fmin(a.x,b.x), fmin(a.y,b.y), fmin(a.z,b.z));
}

inline float3 fmaxf(float3 a, float3 b)
{
	return make_float3(fmax(a.x,b.x), fmax(a.y,b.y), fmax(a.z,b.z));
}


inline float length(float3 v)
{
    return sqrt(dotprod(v, v));
}

inline bool normalize(float3 *v)
{
    float len = length(*v);
    if(len == 0) return false;
    v->x /= len;
    v->y /= len;
    v->z /= len;
    return true;
}

inline float3 normalize(float3 &v)
{
    float len = length(v);
    if(len == 0) return make_float3(0);
    
    float3 rst = v / len;
    return rst;
}

inline float3 reflect(float3 &i, float3 &n)
{
	return i - 2.0f * n * dotprod(n,i);
}

inline float3 cross(float3 &a, float3 &b)
{
    float3 v;
    v.x = a.y * b.z - a.z * b.y;
    v.y = a.z * b.x - a.x * b.z;
    v.z = a.x * b.y - a.y * b.x;
    return v;
}

float _to_radians(float d)
{
	return M_PI/180.0 * d;
}
/*
float degrees(float r)
{
	return 180.0/M_PI * r;
}
*/

// mat4x4 //////////////////////////////////////////////////////////////////////////////////////////////////////////
inline mat4x4 translate(mat4x4 &m, float3 v)
{
    mat4x4 Result = m;
    for(int i = 0; i < 4; i++) Result.d[3][i] = m.d[0][i] * v.a[0] + m.d[1][i] * v.a[1] + m.d[2][i] * v.a[2] + m.d[3][i];
    return Result;
}

inline mat4x4 rotate(mat4x4 &m, float angle, float3 v)
{
    float a = _to_radians(angle);
    float c = cos(a);
    float s = sin(a);
    
    float3 axis = normalize(v);
    float3 temp = (make_float3(1) - c) * axis;
    
    mat4x4 Rotate, Result;
    
    Rotate.d[0][0] = c + temp.a[0] * axis.a[0];
    Rotate.d[0][1] = 0 + temp.a[0] * axis.a[1] + s * axis.a[2];
    Rotate.d[0][2] = 0 + temp.a[0] * axis.a[2] - s * axis.a[1];

    Rotate.d[1][0] = 0 + temp.a[1] * axis.a[0] - s * axis.a[2];
    Rotate.d[1][1] = c + temp.a[1] * axis.a[1];
    Rotate.d[1][2] = 0 + temp.a[1] * axis.a[2] + s * axis.a[0];
    
    Rotate.d[2][0] = 0 + temp.a[2] * axis.a[0] + s * axis.a[1];
    Rotate.d[2][1] = 0 + temp.a[2] * axis.a[1] - s * axis.a[0];
    Rotate.d[2][2] = c + temp.a[2] * axis.a[2];
    
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 4; j++)
            Result.d[i][j] = m.d[0][j] * Rotate.d[i][0] + m.d[1][j] * Rotate.d[i][1] + m.d[2][j] * Rotate.d[i][2];
    }
    
    for(int i = 0; i < 4; i++) Result.d[3][i] = m.d[3][i];
    
    return Result;
}

inline mat4x4 scale(mat4x4 &m, float3 v)
{
    mat4x4 Result;
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 4; j++)
            Result.d[i][j] = m.d[i][j] * v.a[i];
    }
    for(int i = 0; i < 4; i++) Result.d[3][i] = m.d[3][i];
    
    return Result;
}

inline mat4x4 inverseTranspose(mat4x4 m)
{
	mat4x4 Inverse;
	float SubFactor00 = m.d[2][2] * m.d[3][3] - m.d[3][2] * m.d[2][3];
	float SubFactor01 = m.d[2][1] * m.d[3][3] - m.d[3][1] * m.d[2][3];
	float SubFactor02 = m.d[2][1] * m.d[3][2] - m.d[3][1] * m.d[2][2];
	float SubFactor03 = m.d[2][0] * m.d[3][3] - m.d[3][0] * m.d[2][3];
	float SubFactor04 = m.d[2][0] * m.d[3][2] - m.d[3][0] * m.d[2][2];
	float SubFactor05 = m.d[2][0] * m.d[3][1] - m.d[3][0] * m.d[2][1];
	float SubFactor06 = m.d[1][2] * m.d[3][3] - m.d[3][2] * m.d[1][3];
	float SubFactor07 = m.d[1][1] * m.d[3][3] - m.d[3][1] * m.d[1][3];
	float SubFactor08 = m.d[1][1] * m.d[3][2] - m.d[3][1] * m.d[1][2];
	float SubFactor09 = m.d[1][0] * m.d[3][3] - m.d[3][0] * m.d[1][3];
	float SubFactor10 = m.d[1][0] * m.d[3][2] - m.d[3][0] * m.d[1][2];
	float SubFactor11 = m.d[1][1] * m.d[3][3] - m.d[3][1] * m.d[1][3];
	float SubFactor12 = m.d[1][0] * m.d[3][1] - m.d[3][0] * m.d[1][1];
	float SubFactor13 = m.d[1][2] * m.d[2][3] - m.d[2][2] * m.d[1][3];
	float SubFactor14 = m.d[1][1] * m.d[2][3] - m.d[2][1] * m.d[1][3];
	float SubFactor15 = m.d[1][1] * m.d[2][2] - m.d[2][1] * m.d[1][2];
	float SubFactor16 = m.d[1][0] * m.d[2][3] - m.d[2][0] * m.d[1][3];
	float SubFactor17 = m.d[1][0] * m.d[2][2] - m.d[2][0] * m.d[1][2];
	float SubFactor18 = m.d[1][0] * m.d[2][1] - m.d[2][0] * m.d[1][1];

	Inverse.d[0][0] = + (m.d[1][1] * SubFactor00 - m.d[1][2] * SubFactor01 + m.d[1][3] * SubFactor02);
	Inverse.d[0][1] = - (m.d[1][0] * SubFactor00 - m.d[1][2] * SubFactor03 + m.d[1][3] * SubFactor04);
	Inverse.d[0][2] = + (m.d[1][0] * SubFactor01 - m.d[1][1] * SubFactor03 + m.d[1][3] * SubFactor05);
	Inverse.d[0][3] = - (m.d[1][0] * SubFactor02 - m.d[1][1] * SubFactor04 + m.d[1][2] * SubFactor05);

	Inverse.d[1][0] = - (m.d[0][1] * SubFactor00 - m.d[0][2] * SubFactor01 + m.d[0][3] * SubFactor02);
	Inverse.d[1][1] = + (m.d[0][0] * SubFactor00 - m.d[0][2] * SubFactor03 + m.d[0][3] * SubFactor04);
	Inverse.d[1][2] = - (m.d[0][0] * SubFactor01 - m.d[0][1] * SubFactor03 + m.d[0][3] * SubFactor05);
	Inverse.d[1][3] = + (m.d[0][0] * SubFactor02 - m.d[0][1] * SubFactor04 + m.d[0][2] * SubFactor05);

	Inverse.d[2][0] = + (m.d[0][1] * SubFactor06 - m.d[0][2] * SubFactor07 + m.d[0][3] * SubFactor08);
	Inverse.d[2][1] = - (m.d[0][0] * SubFactor06 - m.d[0][2] * SubFactor09 + m.d[0][3] * SubFactor10);
	Inverse.d[2][2] = + (m.d[0][0] * SubFactor11 - m.d[0][1] * SubFactor09 + m.d[0][3] * SubFactor12);
	Inverse.d[2][3] = - (m.d[0][0] * SubFactor08 - m.d[0][1] * SubFactor10 + m.d[0][2] * SubFactor12);

	Inverse.d[3][0] = - (m.d[0][1] * SubFactor13 - m.d[0][2] * SubFactor14 + m.d[0][3] * SubFactor15);
	Inverse.d[3][1] = + (m.d[0][0] * SubFactor13 - m.d[0][2] * SubFactor16 + m.d[0][3] * SubFactor17);
	Inverse.d[3][2] = - (m.d[0][0] * SubFactor14 - m.d[0][1] * SubFactor16 + m.d[0][3] * SubFactor18);
	Inverse.d[3][3] = + (m.d[0][0] * SubFactor15 - m.d[0][1] * SubFactor17 + m.d[0][2] * SubFactor18);
	
	float Determinant =
			+ m.d[0][0] * Inverse.d[0][0]
			+ m.d[0][1] * Inverse.d[0][1]
			+ m.d[0][2] * Inverse.d[0][2]
			+ m.d[0][3] * Inverse.d[0][3];

	for(int i = 0; i < 16; i++) Inverse.a[i] /= Determinant;
 
	return Inverse;
}

inline mat4x4 transpose(mat4x4 m)
{
	mat4x4 Result;
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
			Result.d[i][j] = m.d[j][i];
	
	return Result;
}

// AABB /////////////////////////////////////////////////////////////////////////////////////////////////////////

inline AABB make_aabb(float3 _min, float3 _max)
{
	AABB aabb;
	aabb.min = _min; aabb.max = _max;
    return aabb;
}

bool intersect(AABB *out, AABB *a, AABB *b)
{
	if(a->min.x > b->max.x || b->min.x > a->max.x) return false;
	if(a->min.y > b->max.y || b->min.y > a->max.y) return false;
	if(a->min.z > b->max.z || b->min.z > a->max.z) return false;

	out->min = fmaxf(a->min, b->min);
	out->max = fminf(a->max, b->max);
	return true;
}














