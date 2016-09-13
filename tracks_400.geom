/*
 * tracks_400.geom
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
layout(line_strip, max_vertices = 2) out;

uniform mat4 uPMatrix;
uniform mat4 uMVMatrix;

flat in float drawFlag[];
in vec4 vColour[];

out vec4 fColour;

mat4 mvp = uPMatrix * uMVMatrix;

float tube_radius = 0.001;

void main() 
{
	if(drawFlag[1] == 0) return;

	fColour = vColour[1];
    gl_Position = mvp * gl_in[1].gl_Position;
    EmitVertex();

	fColour = vColour[2];
    gl_Position = mvp * gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}
