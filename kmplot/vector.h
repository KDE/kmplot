/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

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
		Vector() {}
		explicit Vector( int size ) : m_data(size) {}
		Vector( const Vector & other ) : m_data( other.m_data ) {}
		
		int size() const { return m_data.size(); }
		void resize( int s ) { if ( size() != s ) m_data.resize( s ); }
		double *data() { return m_data.data(); }
		const double *data() const { return m_data.data(); }
		Vector operator * ( double x ) const;
		Vector & operator *= ( double x );
		Vector operator + ( const Vector & other ) const;
		Vector & operator += ( const Vector & other );
		Vector operator - ( const Vector & other ) const;
		Vector & operator -= ( const Vector & other );
		Vector & operator = ( const Vector & other );
		Vector & operator = ( const QVector<Value> & other );
		bool operator==( const Vector & other ) const { return m_data == other.m_data; }
		bool operator!=( const Vector & other ) const { return m_data != other.m_data; } 
		double & operator[] ( int i ) { return m_data[i]; }
		double operator[] ( int i ) const { return m_data[i]; }
		/**
		 * Optimization for use in solving differential equations. Sets the
		 * contents of this vector to a+k*b.
		 */
		void combine( const Vector & a, double k, const Vector & b );
		/**
		 * Another optimization for use in solving differential equations.
		 */
		void addRK4( double dx, const Vector & k1, const Vector & k2, const Vector & k3, const Vector & k4 );
		
	protected:
		QVector<double> m_data;
};


inline Vector operator *( double x, const Vector & v )
{
	return v * x;
}

#endif // VECTOR_H
