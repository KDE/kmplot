/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter Möller
*               2000, 2002 kd.moeller@t-online.de
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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#ifndef xparser_included
#define xparser_included

#include "parser.h"
#include <qcolor.h>


class XParser : public Parser
{
public:

	XParser();
	XParser( int anz, int m_size, int s_size );

	int getext( int );
	int delfkt( int );
	double a1fkt( int, double, double h = 1e-3 ),
	a2fkt( int, double, double h = 1e-3 );

	~XParser();

	int dicke0;				// Linienstärke Voreinstellung

	struct FktExt           // Funktionsattribute:
	{
		char f_mode,  		// 1 => Graph von f zeichnen
		f1_mode, 		// 1 => Graph von f' zeichnen
		f2_mode;		// 1 => Graph von f" zeichnen
		int dicke,  		// Stiftdicke
		k_anz;  		// Länge der Parameterliste
		double dmin,     	// Definitionsbereich
		dmax,
		k_liste[ 10 ];	// Parameterliste

		QString extstr;     // vollständiger Eingabeterm
		QRgb farbe,          // Farbe des Graphen
		farbe0;		// Farbvoreinstellung
	}
	*fktext;
};

#endif //xparser_included

