/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar
*                     f_edemar@linux.se
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

#ifndef XPARSERIFACE_H
#define XPARSERIFACE_H

#include <dcopobject.h>
#include <qcolor.h>
#include <qstringlist.h>

/**
@author Fredrik Edemar
*/
/// All functions in ParserIface are accessible with DCOP. For descriptions about the functions, see Parser and XParser.
class ParserIface : virtual public DCOPObject
{
	K_DCOP
k_dcop:
	virtual bool addFunction(const QString &extstr, bool f_mode, bool f1_mode, bool f2_mode, bool integral_mode, bool integral_use_precision, int linewidth, int f1_linewidth, int f2_linewidth, int integral_linewidth, const QString &str_dmin, const QString &str_dmax, const QString &str_startx, const QString &str_starty, double integral_precision, QRgb color, QRgb f1_color, QRgb f2_color, QRgb integral_color, QStringList str_parameter, int use_slider) = 0;
	
	virtual int addFunction(const QString &f_str) = 0;
	virtual bool delfkt(uint id) = 0;
	virtual bool setFunctionExpression(const QString &f_str, uint id) = 0;
	virtual uint countFunctions() = 0;
	virtual QStringList listFunctionNames() = 0;
	virtual int fnameToId(const QString &name) = 0;
	virtual double fkt(uint id, double x) = 0;
	virtual bool functionFVisible(uint id) = 0;
	virtual bool functionF1Visible(uint id) = 0;
	virtual bool functionF2Visible(uint id) = 0;
	virtual bool functionIntVisible(uint id) = 0;
	virtual bool setFunctionFVisible(bool visible, uint id) = 0;
	virtual bool setFunctionF1Visible(bool visible, uint id) = 0;
	virtual bool setFunctionF2Visible(bool visible, uint id) = 0;
	virtual bool setFunctionIntVisible(bool visible, uint id) = 0;
	virtual QString functionStr(uint id) = 0;
	virtual QColor functionFColor(uint id) = 0;
	virtual QColor functionF1Color(uint id) = 0;
	virtual QColor functionF2Color(uint id) = 0;
	virtual QColor functionIntColor(uint id) = 0;
	virtual bool setFunctionFColor(const QColor &color, uint id) = 0;
	virtual bool setFunctionF1Color(const QColor &color, uint id) = 0;
	virtual bool setFunctionF2Color(const QColor &color, uint id) = 0;
	virtual bool setFunctionIntColor(const QColor &color, uint id) = 0;
	virtual int functionFLineWidth(uint id) = 0;
	virtual int functionF1LineWidth(uint id) = 0;
	virtual int functionF2LineWidth(uint id) = 0;
	virtual int functionIntLineWidth(uint id) = 0;
	virtual bool setFunctionFLineWidth(int linewidth, uint id) = 0;
	virtual bool setFunctionF1LineWidth(int linewidth, uint id) = 0;
	virtual bool setFunctionF2LineWidth(int linewidth, uint id) = 0;
	virtual bool setFunctionIntLineWidth(int linewidth, uint id) = 0;
	virtual QStringList functionParameterList(uint id) = 0;
	virtual bool functionAddParameter(const QString &new_parameter, uint id) = 0;
	virtual bool functionRemoveParameter(const QString &remove_parameter, uint id) = 0;
	virtual QString functionMinValue(uint id) = 0;
	virtual QString functionMaxValue(uint id) = 0;
	virtual bool setFunctionMinValue(const QString &min, uint id) = 0;
	virtual bool setFunctionMaxValue(const QString &max, uint id) = 0;
	virtual QString functionStartXValue(uint id) = 0;
	virtual QString functionStartYValue(uint id) = 0;
	virtual bool setFunctionStartXValue(const QString &x, uint id) = 0;
	virtual bool setFunctionStartYValue(const QString &y, uint id) = 0;

};

#endif
