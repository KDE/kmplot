/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2006  David Saxton <david@bluehaze.org>
*               
* This file is part of the KDE Project.
* KmPlot is part of the KDE-EDU Project.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "function.h"
#include "vector.h"

#include <assert.h>

//BEGIN class Vector
Vector Vector::operator + ( const Vector & other ) const
{
	Vector v(*this);
	v += other;
	return v;
}


Vector & Vector::operator += ( const Vector & other )
{
	assert( size() == other.size() );
	for ( int i=0; i<size(); ++i )
		(*this)[i] += other[i];
	
	return *this;
}


Vector Vector::operator - ( const Vector & other ) const
{
	Vector v(*this);
	v -= other;
	return v;
}


Vector & Vector::operator -= ( const Vector & other )
{
	assert( size() == other.size() );
	for ( int i=0; i<size(); ++i )
		(*this)[i] -= other[i];
	
	return *this;
}


Vector Vector::operator * ( double x ) const
{
	Vector v(*this);
	v *= x;
	return v;
}


Vector & Vector::operator *= ( double x )
{
	for ( int i=0; i<size(); ++i )
		(*this)[i] *= x;
	return *this;
}


Vector & Vector::operator = ( const QVector<Value> & other )
{
	int size = other.size();
	resize( size );
	for ( int i = 0; i < size; ++i )
		(*this)[i] = other[i].value();
	return *this;
}
//END class Vector
