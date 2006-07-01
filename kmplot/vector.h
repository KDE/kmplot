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

#ifndef VECTOR_H
#define VECTOR_H

#include <QVector>

class Value;

/**
 * Mathematical vector.
 */
class Vector
{
	public:
		Vector() {};
		Vector( int size ) : m_data(size) {};
		Vector( const Vector & other ) : m_data( other.m_data ) {};
		
		int size() const { return m_data.size(); }
		void resize( int size ) { m_data.resize( size ); }
		Vector operator * ( double x ) const;
		Vector & operator *= ( double x );
		Vector operator + ( const Vector & other ) const;
		Vector & operator += ( const Vector & other );
		Vector operator - ( const Vector & other ) const;
		Vector & operator -= ( const Vector & other );
		Vector & operator = ( const Vector & other ) { m_data = other.m_data; return *this; }
		Vector & operator = ( const QVector<Value> & other );
		bool operator==( const Vector & other ) const { return m_data == other.m_data; }
		bool operator!=( const Vector & other ) const { return m_data != other.m_data; } 
		double & operator[] ( int i ) { return m_data[i]; }
		double operator[] ( int i ) const { return m_data[i]; }
		
	protected:
		QVector<double> m_data;
};


inline Vector operator *( double x, const Vector & v )
{
	return v * x;
}

#endif // VECTOR_H
