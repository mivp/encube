/*
 * float16.c
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

#include "float16.h"

//using namespace std;

short FloatToFloat16( float value )
{
    if(fabs(value) < _FLOAT16_ERROR_) return 0;
        
    short   fltInt16;
    int     fltInt32;
    memcpy( &fltInt32, &value, sizeof( float ) );
    fltInt16    =  ((fltInt32 & 0x7fffffff) >> 13) - (0x38000000 >> 13);
    fltInt16    |= ((fltInt32 & 0x80000000) >> 16);
    
    return fltInt16;
}

float Float16ToFloat( short fltInt16 )
{
    int fltInt32    =  ((fltInt16 & 0x8000) << 16);
    fltInt32        |= ((fltInt16 & 0x7fff) << 13) + 0x38000000;
    
    float fRet;
    memcpy( &fRet, &fltInt32, sizeof( float ) );
    
    if(fabs(fRet) <= _FLOAT16_ERROR_) return 0;
    return fRet;
}