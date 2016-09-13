/*
 * tubes.geom
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

#version 400
layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 32) out;

uniform mat4 uPMatrix;
uniform mat4 uMVMatrix;
uniform mat4 uMVPMatrix;
uniform mat4 uNormalMatrix;
uniform float uRadius;

flat in float drawFlag[];
in vec4 vColour[];

out vec3 fVertex;
out vec4 fColour;
flat out vec3 end0;
flat out vec3 end1;
flat out vec3 planeN0;
flat out vec3 planeN1;

mat3 nMatrix = mat3(uNormalMatrix);

const int slices = 4; // must be 4
const float parallel_error = 0.01;

void findUpNSideNormals(out vec3 up, out vec3 side, in vec3 v)
{
	if(abs(v.x) <= abs(v.y) && abs(v.x) <= abs(v.z)) side = vec3(1, 0, 0);
	else if(abs(v.y) <= abs(v.x) && abs(v.y) <= abs(v.z)) side = vec3(0, 1, 0);
	else side = vec3(0, 0, 1);
	up = normalize(cross(side, v));
	side = normalize(cross(v, up));
}

void main() 
{
	if(drawFlag[1] == 0) return;

	vec3 p0, p1, p2, p3, ldir; 
	vec3 fn0, fn1, fup0, fup1, fside0, fside1;

	p0 = (uMVMatrix * gl_in[0].gl_Position).xyz; p1 = (uMVMatrix * gl_in[1].gl_Position).xyz;
	p2 = (uMVMatrix * gl_in[2].gl_Position).xyz; p3 = (uMVMatrix * gl_in[3].gl_Position).xyz;

	// tuboid direction
	fn0 = fn1 = ldir = normalize(p2-p1);

	if(drawFlag[0] == 1)
	{
		vec3 ldir_left = normalize(p1-p0);
		fn0 = normalize(ldir_left + ldir);
	} 
	if(drawFlag[2] == 1)
	{
		vec3 ldir_right = normalize(p3-p2);
		fn1 = normalize(ldir + ldir_right);
	}
	// pass geometry in eyespace to fragment shader
	end0 = p1; 
	end1 = p2; 
	planeN0 = fn0;
	planeN1 = fn1;

	// do the calculation again but in the world space
	p0 = gl_in[0].gl_Position.xyz; p1 = gl_in[1].gl_Position.xyz;
	p2 = gl_in[2].gl_Position.xyz; p3 = gl_in[3].gl_Position.xyz;

	// tuboid direction
	fn0 = fn1 = ldir = normalize(p2-p1);

	if(drawFlag[0] == 1)
	{
		vec3 ldir_left = normalize(p1-p0);
		fn0 = normalize(ldir_left + ldir);
	} 
	if(drawFlag[2] == 1)
	{
		vec3 ldir_right = normalize(p3-p2);
		fn1 = normalize(ldir + ldir_right);
	}

	findUpNSideNormals(fup0, fside0, fn0);
	findUpNSideNormals(fup1, fside1, fn1);

	vec3 cs0[slices], cs1[slices];
	cs0[0] = fup0;
	cs0[1] = fside0;
	cs0[2] = -fup0;
	cs0[3] = -fside0;
	cs1[0] = fup1;
	cs1[1] = fside1;
	cs1[2] = -fup1;
	cs1[3] = -fside1;

	// fix the flipping artifacts 
	int offset = 0;
	float dotval = 0;
	for(int i = 0; i < slices; i++)
	{
		float tmpdotval = abs(dot(normalize(cs1[i] - cs0[0]), ldir));
		if(tmpdotval > dotval)
		{
			dotval = tmpdotval;
			offset = i;
		}
	}
/*
	// generate a cuboid (8 triangles)
	for(int i = 0; i <= slices; i++)
	{
		vec3 v1 = cs0[i%slices];
		vec3 v2 = cs1[(i+offset)%slices];

		fVertex = vec3(uMVMatrix * vec4(p1 + v1 * uRadius, 1));
		fColour = vColour[1];
		gl_Position = uMVPMatrix * vec4(p1 + v1 * uRadius, 1);
		EmitVertex();

		fVertex = vec3(uMVMatrix * vec4(p2 + v2 * uRadius, 1));
		fColour = vColour[2];
        gl_Position = uMVPMatrix * vec4(p2 + v2 * uRadius, 1);
        EmitVertex();

	}
	EndPrimitive();
*/

	// generate two vertical planes (4 triangles)
	for(int i = 0; i < 2; i++)
	{
		vec3 v0 = cs0[i];
		vec3 v1 = cs1[(i+offset)%slices];
		vec3 v2 = cs0[i+2];
		vec3 v3 = cs1[(i+2+offset)%slices];
		
		fVertex = vec3(uMVMatrix * vec4(p1 + v0 * uRadius, 1));
		fColour = vColour[1];
		gl_Position = uMVPMatrix * vec4(p1 + v0 * uRadius, 1);
        EmitVertex();

		fVertex = vec3(uMVMatrix * vec4(p2 + v1 * uRadius, 1));
        fColour = vColour[2];
        gl_Position = uMVPMatrix * vec4(p2 + v1 * uRadius, 1);
        EmitVertex();

		fVertex = vec3(uMVMatrix * vec4(p1 + v2 * uRadius, 1));
        fColour = vColour[1];
        gl_Position = uMVPMatrix * vec4(p1 + v2 * uRadius, 1);
        EmitVertex();

		fVertex = vec3(uMVMatrix * vec4(p2 + v3 * uRadius, 1));
        fColour = vColour[2];
        gl_Position = uMVPMatrix * vec4(p2 + v3 * uRadius, 1);
        EmitVertex();

		EndPrimitive();
	}

}
