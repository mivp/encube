/*
 * float.h
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

#ifndef THE__FLOAT_16_H_
#define THE__FLOAT_16_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#define _FLOAT16_ERROR_ 0.000031

extern short FloatToFloat16( float value );
extern float Float16ToFloat( short value );

class Float16
{
public:
    short mValue;
    
public:
    Float16();
    Float16( float value );
    Float16( const Float16& value );
    
    operator float();
    operator float() const;
    
    friend Float16 operator + ( const Float16& val1, const Float16& val2 );
    friend Float16 operator - ( const Float16& val1, const Float16& val2 );
    friend Float16 operator * ( const Float16& val1, const Float16& val2 );
    friend Float16 operator / ( const Float16& val1, const Float16& val2 );
    
    Float16& operator =( const Float16& val );
    Float16& operator +=( const Float16& val );
    Float16& operator -=( const Float16& val );
    Float16& operator *=( const Float16& val );
    Float16& operator /=( const Float16& val );
    Float16& operator -();
};

inline Float16::Float16() { }

inline Float16::Float16( float value )
{
    mValue = FloatToFloat16( value );
}

inline Float16::Float16( const Float16 &value )
{
    mValue  = value.mValue;
}

inline Float16::operator float()
{
    return Float16ToFloat( mValue );
}

inline Float16::operator float() const
{
    return Float16ToFloat( mValue );
}

inline Float16& Float16::operator =( const Float16& val )
{
    mValue  = val.mValue;
    return *this;
}

inline Float16& Float16::operator +=( const Float16& val )
{
    *this   = *this + val;
    return *this;
}

inline Float16& Float16::operator -=( const Float16& val )
{
    *this   = *this - val;
    return *this;
    
}

inline Float16& Float16::operator *=( const Float16& val )
{
    *this   = *this * val;
    return *this;
}

inline Float16& Float16::operator /=( const Float16& val )
{
    *this   = *this / val;
    return *this;
}

inline Float16& Float16::operator -()
{
    *this   = Float16( -(float)*this );
    return *this;
}

////// Friends ////////////////////////////////////////////////////////////////////////////////////

inline Float16 operator + ( const Float16& val1, const Float16& val2 )
{
    return Float16( (float)val1 + (float)val2 );
}

inline Float16 operator - ( const Float16& val1, const Float16& val2 )
{
    return Float16( (float)val1 - (float)val2 );
}

inline Float16 operator * ( const Float16& val1, const Float16& val2 )
{
    return Float16( (float)val1 * (float)val2 );
}

inline Float16 operator / ( const Float16& val1, const Float16& val2 )
{
    return Float16( (float)val1 / (float)val2 );
}

#endif
