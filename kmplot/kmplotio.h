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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

#ifndef KMPLOTIO_H
#define KMPLOTIO_H

#include <kurl.h>
#include "xparser.h"

class QDomDocument;
class QString;
class XParser;

/** @short This class manages the file operations load and save.
 *
 * @author Klaus-Dieter Möller & Matthias Meßmer
 */
class KmPlotIO
{
	public:
		/// Nothing to do here; only static functions needed.
		KmPlotIO( XParser *parser);
		/// Empty.
		~KmPlotIO();
		
		/**
		 * Store all information about the current saved plot in a xml file with the .fkt extension 
		 * in the filename file.
		 * @param url Name (URL) of the file which will be saved.
		 */
		bool save( const KUrl &url );
		
		/**
		 * @return a QDomDocument describing the current KmPlot state (settings
		 * and functions).
		 */
		QDomDocument currentState();
		
		/**
		 * Read a kmpdoc xml file to restore the settings of a previously saved plot
		 * @param url Name (URL) of file which will be opened
		 */
		bool load( const KUrl &url );
		
		/**
		 * Restore KmPlot to the state described in the given QDomDocument.
		 * @return success status
		 */
		bool restore( const QDomDocument & doc );
		
		/**
		 * Adds a QDomElement for \p function to the given \p document
		 */
		static void addFunction( QDomDocument & doc, QDomElement & root, Ufkt * function );
		
		/**
		 * Reads function parameters from the node @a n.
		 * @param parser points to the parser instance.
		 * @param n Node containing the options.
		 * @param allowRename whether to check function names for uniqueness
		 */
		static void parseFunction( XParser *parser, const QDomElement &n, bool allowRename = false );
	
	private:
		/** Esay way to add a tag to the Dom tree
		 * @param &doc The document.
		 * @param parentTag The parent tag to support encapsulated tags.
		 * @param tagName The Name of the tag.
		 * @param tagValue The data between the opening and cloding tag.
		 */
		static void addTag( QDomDocument &doc, QDomElement &parentTag, const QString tagName, const QString tagValue );
		/// Reads axes parameters from the node @a n.
		/// @param n Node containing the options.
		void parseAxes( const QDomElement &n );
		/// Reads grid parameters from the node @a n.
		/// @param n Node containing the options.
		void parseGrid( const QDomElement &n );
		/// Reads scale parameters from the node @a n.
		/// @param n Node containing the options.
		void parseScale( const QDomElement &n );
		/// Reads parameter values for a function from the node @a n.
		/// @param parser points to the parser instance.
		/// @param n Node containing the options.
		/// @param ix Function index in the parser instance
		static void parseParameters( XParser *parser, const QDomElement &n, Ufkt &ufkt);
		
		/// For KDE 3.3
		static void parseThreeDotThreeParameters( XParser *parser, const QDomElement &n, Ufkt &ufkt);
		
                ///For KDE <3.3
                /// This is the same as parseScale but is made for old Kmplot-files
                void oldParseScale( const QDomElement & n );
                /// This is the same as parseFunction but is made for old Kmplot-files
                void oldParseFunction( XParser *parser, const QDomElement &n );
                /// This is the same as parseAxes but is made for old Kmplot-files
                void oldParseAxes( const QDomElement &n );
                
                XParser *m_parser;
                
};

#endif

