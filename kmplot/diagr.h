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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/
/** @file diagr.h 
 * @brief Contains the CDiagr class. */

#ifndef diagr_included
#define diagr_included

// standard includes
#include <math.h>
#include <stdio.h>

// Qt includes 
#include <qpainter.h>

//@{
/// Some abbreviations for horizontal and vertical lines.
#define Line drawLine
#define Lineh(x1, y, x2) drawLine(x1, y, x2, y)
#define Linev(x, y1, y2) drawLine(x, y1, x, y2) 
//@}

//@{
/// Grid styles.
#define GRID_NONE	0
#define	GRID_LINES	1
#define	GRID_CROSSES	2
#define	GRID_POLAR	3
//@}

/** @short This class manages the core drawing of the axes and the grid. */
class CDiagr
{
public:
	/// Contructor. Members are set to initial values.
	///@see Create()
	CDiagr();
	/// Nothing to do for the destructor.
	~CDiagr();

	/// Sets all members to current values.
	void Create( QPoint Ref,
	               int lx, int ly,
	               double xmin, double xmax,
	               double ymin, double ymax );
	/// Sets the current values for the scaling factors
	void Skal( double ex, double ey );
	/// Draws all requested parts of the diagram (axes, labels, grid e.g.)
	void Plot( QPainter* pDC );
	/// Returns the rectangle around the core of the plot area.
	QRect GetPlotArea() { return PlotArea; }
	/// Returns the rectangle for the frame around the plot. Extra frame is bigger.
	QRect GetFrame() { return m_frame; }

	/** @name Transformations */
	//@{
	/// These functions convert real coordinates to pixel coordinates and vice versa.
	int Transx(double);
	int Transy(double);
	double Transx(int);
	double Transy(int);
	//@}
	
	/** @name Style options
	 * These members hold the current options for line widths and colors
	 */
	//@{
	QRgb frameColor;	///< color of the border frame
	QRgb axesColor;		///< color of the axes
	QRgb gridColor;		///< color of the grid

	uint borderThickness,	///< current line width for the border frame
	     axesLineWidth,	///< current line width for the axes
	     gridLineWidth,	///< current line width for the grid
	     ticWidth,		///< current line width for the tics
	     ticLength,		///< current length of the tic lines
	//@}
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

	//@{
	/// Plot range edge.
	double xmin, xmax, ymin, ymax;
	//@}
	//@{
	/// Clip boundage.
	double xmd, ymd;
	//@}
	//@{
	/// Axes tic distance.
	double ex, ey;  
	//@}
	//@{
	///Position of the first tic.      
	double tsx, tsy;
	//@}
	//@{
	/// Screen coordinates of the coordinate system origin.
	double ox, oy;
	//@}
	//@{
	/// Transformation factors.
	/// @see Skal
	double skx, sky;
	//@}
	
	QRect PlotArea;	///< plot area
	QRect m_frame;	///< frame around the plot
};

#endif // diagr_included
