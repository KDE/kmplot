/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter M�ler
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

#ifndef diagr_included
#define diagr_included


#include "misc.h"

#include <math.h>
#include <stdio.h>

#include <qpainter.h>


#define	Line				drawLine
#define	Lineh(x1, y, x2)	drawLine(x1, y, x2, y)
#define Linev(x, y1, y2)	drawLine(x, y1, x, y2) 

#define	AXES			1		/**< visible axes */
#define ARROWS			2		/**< axes with arrows */
#define FRAME			4		/**< visible frame */
#define	EXTFRAME		8		/**< bigger frame */
#define LABEL	16		/**< visible labels */ 

#define	LINIENRASTER	1	/**< line grid */
#define	KREUZRASTER     2	/**< crosses grid */
#define	KREISRASTER     3	/**< polar grid */ 


class CDiagr
{
public:

	CDiagr();
	~CDiagr();

	void	Create(QPoint Ref,
	               int lx, int ly,
	               double xmin, double xmax,
	               double ymin, double ymax,
	               char mode);

	void Skal(double ex, double ey, char g_mode);	
	void Plot(QPainter* pDC);
	QRect GetPlotArea() {return PlotArea;}
	QRect GetFrame() {return Rahmen;}

    int	Transx(double),
    	Transy(double);

    double Transx(int),
           Transy(int);

    QRgb RahmenFarbe,
	     axesColor,
         gridColor;

	uint borderThickness,
	     axesThickness,
	     gridThickness,
	     gradThickness,
	     gradLength,
	     xclipflg,            	// clipflg wird auf 1 gesetzt, wenn die
	     yclipflg;			    // Koordinaten au�rhalb des Diagramms liegen

         
private:

	void Achsen(QPainter*);
	void Raster(QPainter*);
	void Beschriftung(QPainter*);
	char mode,
	     g_mode;

	double xmin, xmax,      // x-Wertebereich
	ymin, ymax,             // y-Wertebereich
	xmd, ymd,     	        // x/y Begrenzung
	ex, ey,     	        // x/y-Achsenteilung
	tsx, tsy,     	        // Positionen der ersten Teilstriche
	ox, oy,                 // Bildschirm-Koordinaten des Ursprungs
	skx, sky;               // Skalierungsfaktoren
	QRect	PlotArea,     	// Plotbereich
	Rahmen;                 // Rahmen
};

#endif // diagr_included
