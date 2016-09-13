/*
 * moment_maps.frag
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

/*
 * DV's comment: I am not releasing the moment_maps.frag code yet! Currently, it's just a copy of raycasting.frag
 * It might lead to shader crashing.
 *
 */

/*
 * Copyright (c) 2014, Monash University. All rights reserved.
 * Author: Owen Kaluza - owen.kaluza ( at ) monash.edu
 *
 * Licensed under the GNU Lesser General Public License
 * https://www.gnu.org/licenses/lgpl.html
 * (volume shader from sharevol https://github.com/OKaluza/sharevol)
 */
#version 120
//precision highp float;

//Defined dynamically before compile...
//const int maxSamples = 256;
const int maxSamples = 2000;

uniform sampler3D uVolume;
uniform sampler2D uTransferFunction;

uniform vec3 uBBMin;
uniform vec3 uBBMax;
uniform vec3 uResolution;

uniform bool uEnableColour;

//test by DV
uniform float uMin;
uniform float uMax;

uniform float uBrightness;
uniform float uContrast;
uniform float uPower;

uniform mat4 uPMatrix;
uniform mat4 uMVMatrix;
uniform mat4 uNMatrix;
uniform float uFocalLength;
uniform vec4 uViewport;
uniform int uSamples;
uniform float uDensityFactor;
uniform float uIsoValue;
uniform vec4 uIsoColour;
uniform vec4 uVolumeColour;
uniform float uIsoSmooth;
uniform int uIsoWalls;

float tex3D(vec3 pos)
{
  return texture3D(uVolume, pos).x;
}

// It seems WebGL has no transpose
mat4 transpose(in mat4 m)
{
  return mat4(
              vec4(m[0].x, m[1].x, m[2].x, m[3].x),
              vec4(m[0].y, m[1].y, m[2].y, m[3].y),
              vec4(m[0].z, m[1].z, m[2].z, m[3].z),
              vec4(m[0].w, m[1].w, m[2].w, m[3].w)
             );
}

//Light moves with camera
const vec3 lightPos = vec3(0.5, 0.5, 5.0);
const float ambient = 0.2;
const float diffuse = 0.8;
const vec3 diffColour = vec3(1.0, 1.0, 1.0);  //Colour of diffuse light
const vec3 ambColour = vec3(0.2, 0.2, 0.2);   //Colour of ambient light

void lighting(in vec3 pos, in vec3 normal, inout vec3 colour)
{
  vec4 vertPos = uMVMatrix * vec4(pos, 1.0);
  vec3 lightDir = normalize(lightPos - vertPos.xyz);
  vec3 lightWeighting = ambColour + diffColour * diffuse * clamp(abs(dot(normal, lightDir)), 0.1, 1.0);

  colour *= lightWeighting;
}

vec3 isoNormal(in vec3 pos, in vec3 shift, in float density)
{
  vec3 shiftpos = vec3(pos.x + shift.x, pos.y + shift.y, pos.z + shift.z);
  vec3 shiftx = vec3(shiftpos.x, pos.y, pos.z);
  vec3 shifty = vec3(pos.x, shiftpos.y, pos.z);
  vec3 shiftz = vec3(pos.x, pos.y, shiftpos.z);

  //Detect bounding box hit (walls)
  if (uIsoWalls > 0)
  {
    if (pos.x <= uBBMin.x) return vec3(-1.0, 0.0, 0.0);
    if (pos.x >= uBBMax.x) return vec3(1.0, 0.0, 0.0);
    if (pos.y <= uBBMin.y) return vec3(0.0, -1.0, 0.0);
    if (pos.y >= uBBMax.y) return vec3(0.0, 1.0, 0.0);
    if (pos.z <= uBBMin.z) return vec3(0.0, 0.0, -1.0);
    if (pos.z >= uBBMax.z) return vec3(0.0, 0.0, 1.0);
  }

  //Calculate normal
  return vec3(density) - vec3(tex3D(shiftx), tex3D(shifty), tex3D(shiftz));
}

vec2 rayIntersectBox(vec3 rayDirection, vec3 rayOrigin)
{
  //Intersect ray with bounding box
  vec3 rayInvDirection = 1.0 / rayDirection;
  vec3 bbMinDiff = (uBBMin - rayOrigin) * rayInvDirection;
  vec3 bbMaxDiff = (uBBMax - rayOrigin) * rayInvDirection;
  vec3 imax = max(bbMaxDiff, bbMinDiff);
  vec3 imin = min(bbMaxDiff, bbMinDiff);
  float back = min(imax.x, min(imax.y, imax.z));
  float front = max(max(imin.x, 0.0), max(imin.y, imin.z));
  return vec2(back, front);
}

void main()
{
    //Correct gl_FragCoord for aspect ratio
    float aspect = uViewport.z / uViewport.w;
    vec2 fragCoord = gl_FragCoord.xy - uViewport.xy; //Subtract viewport offset
    float yCoord = fragCoord.y - floor(fragCoord.y / uViewport.w) * uViewport.w; //Adjust for multiple vertical viewports
    vec2 coord = vec2((fragCoord.x - (uViewport.z - uViewport.w) * 0.5) * aspect, yCoord);
    vec3 rayDirection = normalize((vec4(2.0 * coord / uViewport.zw - 1.0, -uFocalLength, 0) * uMVMatrix).xyz);

    vec4 camPos = -vec4(uMVMatrix[3]);  //4th column of modelview
    vec3 rayOrigin = (transpose(uMVMatrix) * camPos).xyz;

    //Calc step
    float stepSize = 1.732 / float(uSamples); //diagonal of [0,1] normalised coord cube = sqrt(3)

    //Intersect ray with bounding box
    vec2 intersection = rayIntersectBox(rayDirection, rayOrigin);
    //Subtract small increment to avoid errors on front boundary
    intersection.y -= 0.000001;
    //Discard points outside the box (no intersection)
    if (intersection.x <= intersection.y) discard;

    vec3 rayStart = rayOrigin + rayDirection * intersection.y;
    vec3 rayStop = rayOrigin + rayDirection * intersection.x;

    vec3 step = normalize(rayStop-rayStart) * stepSize;
    vec3 pos = rayStart;

    float T = 1.0;
    vec3 colour = vec3(0.0);
    bool inside = false;
    vec3 shift = uIsoSmooth / uResolution;
    //Number of samples to take along this ray before we pass out back of volume...
    float travel = distance(rayStop, rayStart) / stepSize;
    int samples = int(ceil(travel));

    //Raymarch, front to back
    for (int i=0; i < maxSamples; ++i)
    {
      //Render samples until we pass out back of cube or fully opaque
#ifndef IE11
      if (i == samples || T < 0.01) break;
#else
      //This is slower but allows IE 11 to render, break on non-uniform condition causes it to fail
      if (i == uSamples) break;
      if (all(greaterThanEqual(pos, uBBMin)) && all(lessThanEqual(pos, uBBMax)))
#endif
      {
        //Get density
        float density = tex3D(pos);

        // DV: Testing range in intensity
        if (uMin > -1.0 && uMax > -1.0)
        {
            // Ignore this value as it is smaller than our threshold
            if (density < uMin)
                density = 0;
            // Ignore this value as it is greater than our threshold
            if (density > uMax)
                density = 0;

            // Rescale remnant values to [0,1]
            if (density >= uMin && density <= uMax)
            {
                density = (density-uMin) / (uMax-uMin);
            }
        }

#define ISOSURFACE
#ifdef ISOSURFACE
        //Passed through isosurface?
        if (uIsoValue > 0.0 && ((!inside && density >= uIsoValue) || (inside && density < uIsoValue)))
        {
          inside = !inside;
          //Find closer to exact position by iteration
          //http://sizecoding.blogspot.com.au/2008/08/isosurfaces-in-glsl.html
          float exact;
          float a = intersection.y + (float(i)*stepSize);
          float b = a - stepSize;
          for (int j = 0; j < 5; j++)
          {
            exact = (b + a) * 0.5;
            pos = rayDirection * exact + rayOrigin;
            density = tex3D(pos);
            if (density - uIsoValue < 0.0)
              b = exact;
            else
              a = exact;
          }

          //Skip edges unless flagged to draw
          if (uIsoWalls > 0 || all(greaterThanEqual(pos, uBBMin)) && all(lessThanEqual(pos, uBBMax)))
          {
            vec4 value = vec4(uIsoColour.rgb, 1.0);

            //normal = normalize(normal);
            //if (length(normal) < 1.0) normal = vec3(0.0, 1.0, 0.0);
            vec3 normal = normalize(mat3(uNMatrix) * isoNormal(pos, shift, density));

            vec3 light = value.rgb;
            lighting(pos, normal, light);
            //Front-to-back blend equation
            colour += T * uIsoColour.a * light;
            T *= (1.0 - uIsoColour.a);
          }
        }
#endif

        if (uDensityFactor > 0.0)
        {
          //Normalise the density over provided range
          //density = (density - uRange.x) / range;

          density = pow(density, uPower); //Apply power

          vec4 value;
          if (uEnableColour)
            //value = texture2D(uTransferFunction, vec2(density, 0.5));
            value = texture2D(uTransferFunction, vec2(density, 0.5));
          else
          //  value = vec4(density);
			value = density * uVolumeColour;

          value *= uDensityFactor * stepSize;

          //Color
          colour += T * value.rgb;
          //Alpha
          T *= 1.0 - value.a;
        }
      }

      //Next sample...
      pos += step;
    }

    //Apply brightness & contrast
    colour = ((colour - 0.5) * max(uContrast, 0.0)) + 0.5;
    colour += uBrightness;
    if (T == 1.0) discard;
    gl_FragColor = vec4(colour, 1.0 - T);

//#define WRITE_DEPTH
#ifdef WRITE_DEPTH
    /* Write the depth !Not supported in WebGL without extension */
    vec4 clip_space_pos = uPMatrix * vec4(pos, 1.0);
    float ndc_depth = clip_space_pos.z / clip_space_pos.w;
    float depth = (((gl_DepthRange.far - gl_DepthRange.near) * ndc_depth) +
                   gl_DepthRange.near + gl_DepthRange.far) / 2.0;
    gl_FragDepth = depth;
#endif
}

