/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter Möler
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
class XParser;

/**
This class manages the file operations load and save.
@author Klaus-Dieter Möller & Matthias Meßmer
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
		static void save( XParser *parser, const QString filename );
		
		/**
		 * Read a kmpdoc xml file to restaure the settings of a previously saved plot
		 * @param filename name of file which will be opened
		 */
		static void load( XParser *parser, const QString filename );
	
	private:
		/** Esay way to add a tag to the Dom tree
		 * @param doc The document.
		 * @param parentTag The parent tag to support encapsulated tags.
		 * @param tagName The Name of the tag.
		 * @param The data between the opening and cloding tag.
		 */
		static void addTag( QDomDocument &doc, QDomElement &parentTag, const QString tagName, const QString tagValue );
		/// Reads axes parameters from the node @a n.
		/// @param n Node containing the options.
		static void KmPlotIO::parseAxes( const QDomElement &n );
		/// Reads grid parameters from the node @a n.
		/// @param n Node containing the options.
		static void KmPlotIO::parseGrid( const QDomElement &n );
		/// Reads scale parameters from the node @a n.
		/// @param n Node containing the options.
		static void KmPlotIO::parseScale( const QDomElement &n );
		/// Reads function parameters from the node @a n.
		/// @param n Node containing the options.
		static void KmPlotIO::parseFunction( XParser *parser, const QDomElement &n );
		/// Reads parameter values for a function from the node @a n.
		/// @param n Node containing the options.
		/// @param ix Function index in the parser instance
		static void parseParameters( XParser *parser, const QDomElement &n, int ix );
};

#endif
