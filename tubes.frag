/*
 * tubes.frag
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
uniform mat4 uPMatrix;
uniform mat4 uMVMatrix;
uniform mat4 uMVPMatrix;
uniform mat4 uNormalMatrix;
uniform float uRadius;

in vec3 fVertex;
in vec4 fColour;
flat in vec3 end0;
flat in vec3 end1;
flat in vec3 planeN0;
flat in vec3 planeN1;

out vec4 fragColor;

vec4 ambient = vec4(.2, .2, .2, .2);
vec4 diffuse = vec4(.71, .71, .71, .3);
vec4 specular = vec4(.999, .999, .999, .999);
float shininess = 8.0;

void lighting(vec3 v, vec3 n)
{
	vec3 L = normalize(-v);   // head light
    vec3 E = normalize(-v);
    vec3 R = normalize(-reflect(L, n));

    //calculate Ambient Term:
    vec4 Iamb = ambient;

    //calculate Diffuse Term:
    vec4 Idiff = diffuse * max(dot(n, L), 0.0);
    Idiff = clamp(Idiff, 0.0, 1.0);

    // calculate Specular Term:
    vec4 Ispec = specular * pow(max(dot(R, E), 0.0), 0.3 * shininess);
    Ispec = clamp(Ispec, 0.0, 1.0);

    fragColor = mix(fColour*1.5, Idiff + Ispec, 0.5);
//  fragColor = mix(fColour*1.5, Ispec, 0.5);

}

bool rayTubeIntersect_eyespace(out float t, vec3 rayView, vec3 tubeDir)
{
	vec3 a = cross(tubeDir, rayView);
	vec3 b = cross(tubeDir, end0);
	float c = dot(a, a);
	float d = dot(b, b);

	float dotab = dot(a, b);
	float e = sqrt(dotab*dotab - c * (d - uRadius*uRadius));
	
	float t1 = (dotab + e) / c;
	float t2 = (dotab - e) / c;

	t = min(t1, t2);
	if( t > 0) return true;
	else if(t1 > 0) { t = t1; return true; }
	else if(t2 > 0) { t = t2; return true; }
	else return false;
}

void computeInEyeSpace()
{
    float t = 0;
    vec3 rayView = normalize(fVertex);
    vec3 tubeDir = normalize(end1 - end0);

    if(!rayTubeIntersect_eyespace(t, rayView, tubeDir))
    {
        discard;
        return;
    }

    vec3 intersectedPoint = rayView * t;
    vec3 n = -((intersectedPoint - end0) - tubeDir * dot(intersectedPoint - end0, tubeDir)) / uRadius;

    vec4 ndc = uPMatrix * vec4(intersectedPoint, 1);
    gl_FragDepth = ndc.z / ndc.w;
    lighting(intersectedPoint, n);
}


vec3 findNormal(vec3 v)
{
    vec3 b = cross(v, vec3(0, 0, 1));
    if (dot(b, b) < 0.01)
        b = cross(v, vec3(0, 1, 0));
    return b;
}

bool rayTubeIntersect_fragcoord(vec3 origin, vec3 dir, out float t)
{
    vec3 A = end0; vec3 B = end1;
    float Epsilon = 0.0000001;
    float extent = distance(A, B);
    vec3 W = (B - A) / extent;
    vec3 U = findNormal(W);
    vec3 V = cross(U, W);
    U = normalize(cross(V, W));
    V = normalize(V);
    float rSqr = uRadius*uRadius;
    vec3 diff = origin - 0.5 * (A + B);
    mat3 basis = mat3(U, V, W);
    vec3 P = diff * basis;
    float dz = dot(W, dir);
    if (abs(dz) >= 1.0 - Epsilon) {
        float radialSqrDist = rSqr - P.x*P.x - P.y*P.y;
        if (radialSqrDist < 0.0)
            return false;
        t = (dz > 0.0 ? -P.z : P.z) + extent * 0.5;
        return true;
    }

    vec3 D = vec3(dot(U, dir), dot(V, dir), dz);
    float a0 = P.x*P.x + P.y*P.y - rSqr;
    float a1 = P.x*D.x + P.y*D.y;
    float a2 = D.x*D.x + D.y*D.y;
    float discr = a1*a1 - a0*a2;
    if (discr < 0.0)
        return false;

    if (discr > Epsilon) {
        float root = sqrt(discr);
        float inv = 1.0/a2;
        t = (-a1 + root)*inv;
        return true;
    }

    t = -a1/a2;
    return true;
}

void computeInLocalFragmentCoord()
{
	vec3 rayView = normalize(-fVertex);

    if (distance(fVertex, vec3(0)) < 0.1) {
        discard;
        return;
    }

    float t = 0;
    if (!rayTubeIntersect_fragcoord(fVertex, rayView, t)) {
        discard;
        return;
    }
    vec3 intersectedPoint = fVertex + rayView * t;

    vec3 tubeDir = normalize(end1 - end0);
    vec3 n = -((intersectedPoint - end0) - tubeDir * dot(intersectedPoint - end0, tubeDir)) / uRadius;

    vec4 ndc = uPMatrix * vec4(intersectedPoint, 1);
    gl_FragDepth = ndc.z / ndc.w;
    lighting(intersectedPoint, n);

}

void main(void)
{
	// there are two ways to compute raycast tuboid. Not much difference in performance and quality
	computeInEyeSpace();
//	computeInLocalFragmentCoord();
}
