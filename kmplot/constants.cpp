/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter Möller <kd.moeller@t-online.de>
*                     2006 David Saxton <david@bluehaze.org>
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/


#include "constants.h"
#include "xparser.h"

#include <ksimpleconfig.h>


//BEGIN class Constant
Constant::Constant( )
{
	type = Document;
}
//END class Constant



//BEGIN class Constants
Constants::Constants()
{
}


Constants::~Constants()
{
}


Value Constants::value( const QString & name ) const
{
	return m_constants[ name ].value;
}


bool Constants::have( const QString & name ) const
{
	return m_constants.contains( name );
}


void Constants::remove( const QString & name )
{
	if ( m_constants.remove( name ) > 0 )
		emit constantsChanged();
}


void Constants::add( const QString & name, const Constant & constant )
{
	m_constants[name] = constant;
	emit constantsChanged();
}


ConstantList Constants::list( int type ) const
{
	ConstantList list;
	
	for ( ConstantList::const_iterator it = m_constants.begin(); it != m_constants.end(); ++it )
	{
		if ( type & it.value().type )
			list.insert( it.key(), it.value() );
	}
	
	return list;
}


bool Constants::isValidName( const QString & name ) const
{
	// Don't allow empty names
	if ( name.isEmpty() )
		return false;
	
	// Don't allow constants names that are already used by a function
	if ( XParser::self()->predefinedFunctions().contains( name ) ||
			XParser::self()->predefinedFunctions().contains( name ) )
		return false;
	
	// special cases: don't allow predefined constants either
	if ( name == "pi" || name == QChar(960) || name == "e" || name == QChar(0x221E) )
		return false;
	
	// Now make sure that the constant name contains only letters
	for ( int i = 0; i < name.length(); ++i )
	{
		if ( !name.at(i).isLetter() )
			return false;
	}
	
	// All ok!
	return true;
}


QString Constants::generateUniqueName() const
{
	QString name;
	int at = 0;
	while (true)
	{
		at++;
		name.resize( at );
		for ( char c = 'A'; c <= 'Z'; ++c )
		{
			name[at-1] = c;
			if ( isValidName(name) && !have(name) )
				return name;
		}
	}
}


void Constants::load()
{
	KSimpleConfig conf ("kcalcrc");
	conf.setGroup("UserConstants");
	QString tmp;
	
	for( int i=0; ;i++)
	{
		tmp.setNum(i);
		QString name = conf.readEntry("nameConstant"+tmp, QString(" "));
		QString value = conf.readEntry("valueConstant"+tmp, QString(" "));
		
		if ( name == " " )
			return;
		
		if ( name.isEmpty() )
			continue;
		
		if ( !isValidName( name ) || have( name ) )
			name = generateUniqueName();
		
		Constant constant;
		constant.value = value;
		constant.type = Constant::Global;
		
		add( name, constant );
	}
}

void Constants::save()
{
	KSimpleConfig conf ("kcalcrc");
	conf.deleteGroup("Constants");
	
	// remove any previously saved constants
	conf.deleteGroup( "UserConstants" );
	
	conf.setGroup("UserConstants");
	QString tmp;
	
	ConstantList global = list( Constant::Global );
	
	int i = 0;
	for ( ConstantList::iterator it = global.begin(); it != global.end(); ++it )
	{
		tmp.setNum(i);
		conf.writeEntry( "nameConstant"+tmp, it.key() ) ;
		conf.writeEntry( "valueConstant"+tmp, it.value().value.expression() );
		
		i++;
	}
}
//END class Constants

#include "constants.moc"

