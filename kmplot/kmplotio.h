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

#ifndef KMPLOTIO_H
#define KMPLOTIO_H

class QString;

/**
This class manages the file operations load and save

@author Klaus-Dieter Moeller
*/
class KmPlotIO
{
	public:
		KmPlotIO();

		~KmPlotIO();
		
		/**
		 * Store all information about the current saved plot in a xml file with the .fkt extension 
		 * in the filename file
		 * @param filename name of the file which will be saved
		 */
		static void save( const QString filename );
		
		/**
		 * Read a kmpdoc xml file to restaure the settings of a previously saved plot
		 * @param filename name of file which will be opened
		 */
		static void load( const QString filename );
	
	private:
		static void addTag( QDomDocument &doc, QDomElement &parentTag, const QString tagName, const QString tagValue );
		static void KmPlotIO::parseAxes( const QDomElement &n );
		static void KmPlotIO::parseGrid( const QDomElement &n );
		static void KmPlotIO::parseScale( const QDomElement &n );
		static void KmPlotIO::parseStep( const QDomElement &n );
		static void KmPlotIO::parseFunction( const QDomElement &n );
		static void parseParameters( const QDomElement &n, int ix );
};

#endif
