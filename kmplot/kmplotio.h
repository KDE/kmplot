//
// C++ Interface: kmplotio
//
// Description: 
//
//
// Author: Klaus-Dieter Moeller <kd.moeller@t-online.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
};

#endif
