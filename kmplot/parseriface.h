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
#include <tqcolor.h>
#include <tqstringlist.h>

/**
@author Fredrik Edemar
*/
/// All functions in ParserIface are accessible with DCOP. For descriptions about the functions, see Parser and XParser.
class ParserIface : virtual public DCOPObject
{
	K_DCOP
k_dcop:
	virtual bool addFunction(const TQString &extstr, bool f_mode, bool f1_mode, bool f2_mode, bool integral_mode, bool integral_use_precision, int linewidth, int f1_linewidth, int f2_linewidth, int integral_linewidth, const TQString &str_dmin, const TQString &str_dmax, const TQString &str_startx, const TQString &str_starty, double integral_precision, QRgb color, QRgb f1_color, QRgb f2_color, QRgb integral_color, TQStringList str_parameter, int use_slider) = 0;
	
	virtual int addFunction(const TQString &f_str) = 0;
	virtual bool delfkt(uint id) = 0;
	virtual bool setFunctionExpression(const TQString &f_str, uint id) = 0;
	virtual uint countFunctions() = 0;
	virtual TQStringList listFunctionNames() = 0;
	virtual int fnameToId(const TQString &name) = 0;
	virtual double fkt(uint id, double x) = 0;
	virtual bool functionFVisible(uint id) = 0;
	virtual bool functionF1Visible(uint id) = 0;
	virtual bool functionF2Visible(uint id) = 0;
	virtual bool functionIntVisible(uint id) = 0;
	virtual bool setFunctionFVisible(bool visible, uint id) = 0;
	virtual bool setFunctionF1Visible(bool visible, uint id) = 0;
	virtual bool setFunctionF2Visible(bool visible, uint id) = 0;
	virtual bool setFunctionIntVisible(bool visible, uint id) = 0;
	virtual TQString functionStr(uint id) = 0;
	virtual TQColor functionFColor(uint id) = 0;
	virtual TQColor functionF1Color(uint id) = 0;
	virtual TQColor functionF2Color(uint id) = 0;
	virtual TQColor functionIntColor(uint id) = 0;
	virtual bool setFunctionFColor(const TQColor &color, uint id) = 0;
	virtual bool setFunctionF1Color(const TQColor &color, uint id) = 0;
	virtual bool setFunctionF2Color(const TQColor &color, uint id) = 0;
	virtual bool setFunctionIntColor(const TQColor &color, uint id) = 0;
	virtual int functionFLineWidth(uint id) = 0;
	virtual int functionF1LineWidth(uint id) = 0;
	virtual int functionF2LineWidth(uint id) = 0;
	virtual int functionIntLineWidth(uint id) = 0;
	virtual bool setFunctionFLineWidth(int linewidth, uint id) = 0;
	virtual bool setFunctionF1LineWidth(int linewidth, uint id) = 0;
	virtual bool setFunctionF2LineWidth(int linewidth, uint id) = 0;
	virtual bool setFunctionIntLineWidth(int linewidth, uint id) = 0;
	virtual TQStringList functionParameterList(uint id) = 0;
	virtual bool functionAddParameter(const TQString &new_parameter, uint id) = 0;
	virtual bool functionRemoveParameter(const TQString &remove_parameter, uint id) = 0;
	virtual TQString functionMinValue(uint id) = 0;
	virtual TQString functionMaxValue(uint id) = 0;
	virtual bool setFunctionMinValue(const TQString &min, uint id) = 0;
	virtual bool setFunctionMaxValue(const TQString &max, uint id) = 0;
	virtual TQString functionStartXValue(uint id) = 0;
	virtual TQString functionStartYValue(uint id) = 0;
	virtual bool setFunctionStartXValue(const TQString &x, uint id) = 0;
	virtual bool setFunctionStartYValue(const TQString &y, uint id) = 0;

};

#endif
