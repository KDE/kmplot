/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter Möller <kd.moeller@t-online.de>
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
/** @file kprinterdlg.h 
 * @brief Contains the KPrinterDlg class. */

#ifndef KMPLOT_KPRINTERDLG_H
#define KMPLOT_KPRINTERDLG_H

#include <QWidget>

// KDE includes

class QCheckBox;
class QComboBox;
class EquationEdit;

/** @short A dialog page for the print dialog.
 *
 * It manages the option to print or not to print the header table with addition plot information.
 * It is linked to the app-kmplot-printtable entry in the kmplotrc.
 */
class KPrinterDlg : public QWidget
{
	Q_OBJECT
public:
	/// Getting the common arguments.
	explicit KPrinterDlg( QWidget *parent = 0 );

	bool printHeaderTable();
	void setPrintHeaderTable( bool status );

	bool printBackground();
	void setPrintBackground( bool status );

	double printWidth();
	void setPrintWidth( double width );

	double printHeight();
	void setPrintHeight( double height );

	/// Reimplemented.
	bool isValid( QString& msg );
	
protected:
	enum LengthScaling { Pixels, Inches, Centimeters, Millimeters };
	/**
	 * \return the scaling (for converting the width or height to meters via
	 * multiplication).
	 */
	double lengthScaling() const;
	/**
	 * Converts the scaling unit to an absolute one in meters.
	 */
	static double scalingToMeter( LengthScaling scaling );
	/**
	 * Checkbox for whether to print the header table containing plot
	 * information.
	 */
	QCheckBox *m_printHeaderTable;
	/**
	 * Checkbox for whether to print the background.
	 */
	QCheckBox *m_transparentBackground;
	/**
	 * For the width of the plot.
	 */
	EquationEdit *m_widthEdit;
	/**
	 * For the height of the plot.
	 */
	EquationEdit *m_heightEdit;
	/**
	 * The units to use for lengths (such as the width and height).
	 */
	QComboBox *m_lengthScalingCombo;
};

#endif //KMPLOT_KPRINTERDLG_H
