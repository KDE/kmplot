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


#include <math.h>
#include <stdio.h>

#include <qpainter.h>


#define	Line				drawLine
#define	Lineh(x1, y, x2)	drawLine(x1, y, x2, y)
#define Linev(x, y1, y2)	drawLine(x, y1, x, y2) 

#define AXES		1		/**< visible axes */
#define ARROWS		2		/**< axes with arrows */
#define FRAME		4		/**< visible frame */
#define EXTFRAME	8		/**< bigger frame */
#define LABEL		16		/**< visible labels */ 

#define	LINIENRASTER	1	/**< line grid */
#define	KREUZRASTER     2	/**< crosses grid */
#define	KREISRASTER     3	/**< polar grid */ 


/**
 * This class manages the core drawing of axes, grid, plot and so on.
 */
class CDiagr
{
public:

	CDiagr();
	~CDiagr();

	void Create(QPoint Ref,
	               int lx, int ly,
	               double xmin, double xmax,
	               double ymin, double ymax);

	void Skal(double ex, double ey );	
	void Plot(QPainter* pDC);
	QRect GetPlotArea() {return PlotArea;}
	QRect GetFrame() {return m_frame;}

	/** @name Trnsformations 
	 * These functions convert real coordinates to pixel coordinates and vice versa.
	 */
	//@{
	int Transx(double);
	int Transy(double);
	double Transx(int);
	double Transy(int);
	//@}
	
	QRgb frameColor;	///< color of the border frame
	QRgb axesColor;		///< color of the axes
	QRgb gridColor;		///< color of the grid

	uint borderThickness,	///< current line width for the border frame
	     axesLineWidth,	///< current line width for the axes
	     gridLineWidth,	///< current line width for the grid
	     ticWidth,		///< current line width for the tics
	     ticLength,		///< current length of the tic lines
	     
	     xclipflg,		///< clipflg is set to 1 if the plot is out of the plot aerea.
	     yclipflg;		///< clipflg is set to 1 if the plot is out of the plot aerea.

         
private:

	/// Draw the coordinate axes.
	void drawAxes(QPainter*);
	/// Draw the grid.
	void drawGrid( QPainter* );
	/// Write labels.
	void drawLabels(QPainter*);
	/// Current grid style.
	int g_mode;

	double xmin,		///< x range minimum 
	xmax,      		///< x range maximum
	ymin,			///< y range minimum
	ymax,			///< y range maximum
	xmd, ymd,     	        ///< x/y clip boundage
	ex, ey,     	        ///< x/y axes units
	tsx, tsy,     	        ///< Position of the first tic
	ox, oy,                 ///< screen coordinates of the coordinate system origin
	skx, sky;               ///< scale factors
	QRect	PlotArea, 	///< plot area
	m_frame; 		///< frame around the plot
};

#endif // diagr_included
