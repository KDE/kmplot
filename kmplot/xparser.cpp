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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

// KDE includes
#include <kapplication.h>
#include <kglobal.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

// local includes
#include "xparser.h"
#include <QList>

#include <assert.h>
#include <cmath>


XParser * XParser::m_self = 0;

XParser * XParser::self( bool * modified )
{
	if ( !m_self )
	{
		assert( modified );
		m_self = new XParser( *modified );
	}
	
	return m_self;
}


XParser::XParser(bool &mo) : m_modified(mo)
{
	// setup slider support
}

XParser::~XParser()
{
}

bool XParser::getext( Function *item, const QString fstr )
{
  	bool errflg = false;
   	int p1, p2, p3, pe;
	QString tstr;
	pe = fstr.length();
	if ( fstr.indexOf( 'N' ) != -1 )
		item->plotAppearance( Function::Derivative0 ).visible = false;
	else
	{
		if ( fstr.indexOf( "A1" ) != -1 )
			item->plotAppearance( Function::Derivative1 ).visible = true;
		if ( fstr.indexOf( "A2" ) != -1 )
			item->plotAppearance( Function::Derivative2 ).visible = true;
	}
	switch ( fstr[0].unicode() )
	{
		case 'x':
		case 'y':
		case 'r':
			item->plotAppearance( Function::Derivative1 ).visible = item->plotAppearance( Function::Derivative2 ).visible = false;
	}

	p1 = fstr.indexOf( "D[" );
	if ( p1 != -1 )
	{
		p1 += 2;
		const QString str = fstr.mid( p1, pe - p1);
		p2 = str.indexOf(',');
		p3 = str.indexOf(']');
		if ( p2 > 0 && p2 < p3 )
		{
			tstr = str.left( p2 );
			errflg |= !item->dmin.updateExpression( tstr );
			tstr = str.mid( p2 + 1, p3 - p2 - 1 );
			errflg |= !item->dmax.updateExpression( tstr );
			if ( item->dmin.value() > item->dmax.value() )
				errflg = true;
		}
		else
			errflg = true;
	}
	p1 = fstr.indexOf( "P[" );
	if ( p1 != -1 )
	{
		int i = 0;
		p1 += 2;
		QString str = fstr.mid( p1, 1000);
		p3 = str.indexOf( ']' );
		do
		{
			p2 = str.indexOf( ',' );
			if ( p2 == -1 || p2 > p3 )
				p2 = p3;
			tstr = str.left( p2++ );
			str = str.mid( p2, 1000 );
			Value value;
			if ( !value.updateExpression( tstr ) )
			if ( parserError(false) )
			{
				errflg = true;
				break;
			}
			item->m_parameters.list.append( value );
			p3 -= p2;
		}
		while ( p3 > 0 && i < 10 );
	}

	if ( errflg )
	{
		KMessageBox::error( 0, i18n( "Error in extension." ) );
		return false;
	}
	else
		return true;
}


double XParser::derivative( int n, Equation * eq, double x, double h )
{
// 	kDebug() << k_funcinfo << "n="<<n<<" h="<<h<<" pow(h,n)="<<pow(h,n)<<endl;
	
	if ( n < -1 )
	{
		kError() << k_funcinfo << "Can't handle derivative < -1\n";
		return 0.0;
	}
	
	switch ( n )
	{
		case -1:
			return integral( eq, x, h );
		
		case 0:
			return fkt( eq, x );
			
		case 1:
			return ( fkt(eq, x + (h/2) ) - fkt( eq, x - (h/2) ) ) / h;
			
		case 2:
			return ( fkt( eq, x + h ) - 2 * fkt( eq, x ) + fkt( eq, x - h ) ) / (h*h);
			
		default:
			return ( derivative( n-1, eq, x+(h/2), (h/4) ) - derivative( n-1, eq, x-(h/2), (h/4) ) ) / h;
	}
}


double XParser::partialDerivative( int n1, int n2, Equation * eq, double x, double y, double h1, double h2 )
{
	if ( n1 < 0 || n2 < 0 )
	{
		kError() << k_funcinfo << "Can't handle derivative < 0\n";
		return 0.0;
	}
	
	if ( n1 > 0 )
		return ( partialDerivative( n1-1, n2, eq, x+(h1/2), y, (h1/4), h2 ) - partialDerivative( n1-1, n2, eq, x-(h1/2), y, (h1/4), h2 ) ) / h1;
	
	Function * f = eq->parent();
	f->m_implicitMode = Function::FixedX;
	f->x = x;
	
	return derivative( n2, eq, y, h2 );
}


QString XParser::findFunctionName( const QString & preferredName, int id )
{
	// The position of the character attempting to replace
	int pos = preferredName.length()-1;
	
	QString name = preferredName;
	
	for ( ; ; ++pos)
	{
		for ( QChar lastChar = 'f'; lastChar<'x'; ++lastChar.unicode() )
		{
			bool ok = true;
			name[pos] = lastChar;
			
			foreach ( Function * it, m_ufkt )
			{
				if ( int(it->id) == id )
					continue;
				
				foreach ( Equation * eq, it->eq )
				{
					if ( eq->name() == name )
						ok = false;
				}
				
				if (!ok)
					break;
			}
			if ( !ok )
				continue;
			
			// Found a free name :)
			return name;
		}
		name[pos]='f';
		name.append('f');
	}
}


void XParser::fixFunctionName( QString &str, Equation::Type const type, int const id)
{
	int p1 = str.indexOf('(');
	int p2 = str.indexOf(')');
	int p3 = str.indexOf('=');
	
	bool hasBeginning = (p1>=0);
	for ( int i = p2+1; i < p3; ++i )
	{
		if ( !str.at(i).isSpace() )
			hasBeginning = false;
	}
	
	if ( hasBeginning )
	{
		QString const fname = str.left(p1);
		foreach ( Function * it, m_ufkt )
		{
			if ( int(it->id) == id )
				continue;
			
			foreach ( Equation * eq, it->eq )
			{
				if ( eq->name() != fname )
					continue;
				
				str = str.mid(p1,str.length()-1);
				QString function_name;
				if ( type == Equation::ParametricX )
					function_name = "x";
				else if ( type == Equation::ParametricY )
					function_name = "y";
				else
					function_name = "f";
				function_name = findFunctionName( function_name, id );
				str.prepend( function_name );
				return;
			}
		}
	}
	else if ( p1==-1 || !str.at(p1+1).isLetter() ||  p2==-1 || str.at(p2+1 )!= '=')
	{
		QString function_name;
		if ( type == Equation::ParametricX )
			function_name = "xf";
		else if ( type == Equation::ParametricY )
			function_name = "yf";
		else
			function_name = "f";
		str.prepend("(x)=");
		function_name = findFunctionName( function_name, id );
		str.prepend( function_name );
	}
}


Vector XParser::rk4_f( int order, Equation * eq, double x, Vector y )
{
	bool useParameter = eq->parameters().size() > order+1;
	
	Vector result( order );
	Vector arg( order+1 + (useParameter ? 1 : 0) );
	
	arg[0] = x;
	
	if ( useParameter )
		arg[1] = eq->parent()->k;
	
	for ( int i = 0; i < order; ++i )
	{
		arg[i+1 + (useParameter ? 1 : 0) ] = y[i];
		if ( i+1 < order )
			result[i] = y[i+1];
	}
	
	result[order-1] = XParser::fkt( eq, arg );
	
	return result;
}


double XParser::differential( Equation * eq, DifferentialState * state, double x_target, double h )
{
	if ( eq->order() < 1 )
	{
		kWarning() << k_funcinfo << "Zero order!\n";
		return 0;
	}
	
	h = qAbs(h);
	assert( h > 0 ); // in case anyone tries to pass us a zero h
	
	// the difference between h and dx is that h is only used as a hint for the
	// stepwidth; dx is made similar to h in size, yet tiles the gap between x
	// and the previous x perfectly
	
	// see if the initial integral point in the function is closer to our
	// required x value than the last one (or the last point is invalid)
	if ( qAbs( state->x0.value() - x_target ) < qAbs( state->x - x_target ) || !std::isfinite( state->y[0] ) )
		state->resetToInitial();
	
	int order = eq->order();
	
	Vector k1( order );
	Vector k2( order );
	Vector k3( order );
	Vector k4( order );
	
	double x = state->x;
	Vector y( state->y );
	if ( x_target == x )
		return y[0];
	
	int intervals = qMax( qRound( qAbs(x_target-x)/h ), 1 );
	double dx = (x_target-x) / double(intervals);
	
	for ( int i = 0; i < intervals; ++i )
	{
		x = state->x + i*dx;
		
		k1 = rk4_f( order, eq, x,			y );
		k2 = rk4_f( order, eq, x + dx/2,	y + (dx/2)*k1 );
		k3 = rk4_f( order, eq, x + dx/2,	y + (dx/2)*k2 );
		k4 = rk4_f( order, eq, x + dx,		y + dx*k3 );
		
		y += (dx/6)*(k1 + 2*k2 + 2*k3 + k4);
	}
	
	state->x = x + dx;
	state->y = y;
	
	return y[0];
}


double XParser::integral( Equation * eq, double x, double h )
{
	/// \todo merge this with the XParser::differential function
	
	h = qAbs(h);
	assert( h > 0 ); // in case anyone tries to pass us a zero h
	
	// the difference between h and dx is that h is only used as a hint for the
	// stepwidth; dx is made similar to h in size, yet tiles the gap between x
	// and the previous x perfectly
	
	// we use the 2nd degree Newton-Cotes formula (Simpson's rule)
	
	// see if the initial integral point in the function is closer to our
	// required x value than the last one
	if ( qAbs( eq->integralInitialX().value() - x ) < qAbs( eq->lastIntegralPoint.x() - x ) )
		eq->resetLastIntegralPoint();
	
	double a0 = eq->lastIntegralPoint.x();
	double y = eq->lastIntegralPoint.y();
	
	if ( a0 == x )
		return y;
	
	int intervals = qMax( qRound( qAbs(x-a0)/h ), 1 );
	double dx = (x-a0) / intervals;
	
	double f_a = fkt( eq, a0 );
	
	for ( int i = 0; i < intervals; ++i )
	{
		double b = a0 + (dx*(i+1));
		double m = b - (dx/2);
		
		double f_b = fkt( eq, b );
		double f_m = fkt( eq, m );
		
		y += (dx / 6.0)*(f_a + (4.0 * f_m) + f_b);
		
		f_a = f_b;
	}
	
	eq->lastIntegralPoint = QPointF( x, y );
	return y;
}


QColor XParser::defaultColor(int function)
{
	switch ( function % 10 )
	{
		case 0:
			return Settings::color0();
		case 1:
			return Settings::color1();
		case 2:
			return Settings::color2();
		case 3:
			return Settings::color3();
		case 4:
			return Settings::color4();
		case 5:
			return Settings::color5();
		case 6:
			return Settings::color6();
		case 7:
			return Settings::color7();
		case 8:
			return Settings::color8();
		case 9:
			return Settings::color9();
	}
	
	assert( !"Shouldn't happen - XParser::defaultColor" );
}

QStringList XParser::listFunctionNames()
{
	QStringList list;
	foreach ( Function * it, m_ufkt )
	{
		foreach ( Equation * eq, it->eq )
		{
			QString fname = eq->name();
			if ( !fname.isEmpty() )
				list << fname;
		}
	}
	return list;	
}

bool XParser::functionFVisible(uint id)
{
	return m_ufkt.contains(id) ? m_ufkt[id]->plotAppearance( Function::Derivative0 ).visible : false;
}
bool XParser::functionF1Visible(uint id)
{
	return m_ufkt.contains(id) ? m_ufkt[id]->plotAppearance( Function::Derivative1 ).visible : false;
}
bool XParser::functionF2Visible(uint id)
{
	return m_ufkt.contains(id) ? m_ufkt[id]->plotAppearance( Function::Derivative2 ).visible : false;
}
bool XParser::functionIntVisible(uint id)
{
	return m_ufkt.contains(id) ? m_ufkt[id]->plotAppearance( Function::Integral ).visible : false;
}

bool XParser::setFunctionFVisible(bool visible, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative0 ).visible = visible;
	m_modified = true;
	return true;
}
bool XParser::setFunctionF1Visible(bool visible, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative1 ).visible = visible;
	m_modified = true;
	return true;
}
bool XParser::setFunctionF2Visible(bool visible, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative2 ).visible = visible;
	m_modified = true;
	return true;
}
bool XParser::setFunctionIntVisible(bool visible, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Integral ).visible = visible;
	m_modified = true;
	return true;
}

QString XParser::functionStr(uint id, uint eq)
{
	if ( !m_ufkt.contains( id ) || (eq>=2) )
		return "";
	return m_ufkt[id]->eq[eq]->fstr();
}

QColor XParser::functionFColor(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return QColor();
	return QColor(m_ufkt[id]->plotAppearance( Function::Derivative0 ).color);
}
QColor XParser::functionF1Color(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return QColor();
	return QColor(m_ufkt[id]->plotAppearance( Function::Derivative1 ).color);
}
QColor XParser::functionF2Color(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return QColor();
	return QColor(m_ufkt[id]->plotAppearance( Function::Derivative2 ).color);
}
QColor XParser::functionIntColor(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return QColor();
	return QColor(m_ufkt[id]->plotAppearance( Function::Integral ).color);
}
bool XParser::setFunctionFColor(const QColor &color, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative0 ).color = color;
	m_modified = true;
	return true;
}
bool XParser::setFunctionF1Color(const QColor &color, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative1 ).color = color;
	m_modified = true;
	return true;
}		
bool XParser::setFunctionF2Color(const QColor &color, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative2 ).color = color;
	m_modified = true;
	return true;
}
bool XParser::setFunctionIntColor(const QColor &color, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Integral ).color = color;
	m_modified = true;
	return true;
}

double XParser::functionFLineWidth(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return 0;
	return m_ufkt[id]->plotAppearance( Function::Derivative0 ).lineWidth;
}
double XParser::functionF1LineWidth(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return 0;
	return m_ufkt[id]->plotAppearance( Function::Derivative1 ).lineWidth;
}
double XParser::functionF2LineWidth(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return 0;
	return m_ufkt[id]->plotAppearance( Function::Derivative2 ).lineWidth;
}
double XParser::functionIntLineWidth(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return 0;
	return m_ufkt[id]->plotAppearance( Function::Integral ).lineWidth;
}
bool XParser::setFunctionFLineWidth(double linewidth, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative0 ).lineWidth = linewidth;
	m_modified = true;
	return true;
}
bool XParser::setFunctionF1LineWidth(double linewidth, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative1 ).lineWidth = linewidth;
	m_modified = true;
	return true;
}		
bool XParser::setFunctionF2LineWidth(double linewidth, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative2 ).lineWidth = linewidth;
	m_modified = true;
	return true;
}
bool XParser::setFunctionIntLineWidth(double linewidth, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Integral ).lineWidth = linewidth;
	m_modified = true;
	return true;
}

QString XParser::functionMinValue(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return 0;
  return m_ufkt[id]->dmin.expression();
}

bool XParser::setFunctionMinValue(const QString &min, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
  m_ufkt[id]->dmin.expression() = min;
  m_modified = true;
  return true;
}

QString XParser::functionMaxValue(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return 0;
  return m_ufkt[id]->dmax.expression();
}

bool XParser::setFunctionMaxValue(const QString &max, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
  m_ufkt[id]->dmax.expression() = max;
  m_modified = true;
  return true;
}

bool XParser::setFunctionStartValue(const QString &x, const QString &y, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->eq[0]->setIntegralStart( x, y );
	m_modified = true;
	return true;
}

QString XParser::functionStartXValue(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return 0;
	return m_ufkt[id]->eq[0]->integralInitialX().expression();
}


QString XParser::functionStartYValue(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return 0;
	return m_ufkt[id]->eq[0]->integralInitialY().expression();
}

QStringList XParser::functionParameterList(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return QStringList();
	Function *item = m_ufkt[id];
	QStringList str_parameter;
	foreach ( Value it, item->m_parameters.list )
		str_parameter << it.expression();
	return str_parameter;
}
bool XParser::functionAddParameter(const QString &new_parameter, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	Function *tmp_ufkt = m_ufkt[id];
	
	//check if the parameter already exists
	foreach ( Value it, tmp_ufkt->m_parameters.list )
	{
		if ( it.expression() == new_parameter )
			return false;
	}
	
	Value value;
	if ( !value.updateExpression( new_parameter ) )
		return false;
	tmp_ufkt->m_parameters.list.append( value );
	m_modified = true;
	return true;
}
bool XParser::functionRemoveParameter(const QString &remove_parameter, uint id)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	Function *tmp_ufkt = m_ufkt[id];
	
	bool found = false;
	QList<Value>::iterator it;
	for ( it = tmp_ufkt->m_parameters.list.begin(); it != tmp_ufkt->m_parameters.list.end(); ++it)
	{
		if ( (*it).expression() == remove_parameter) //check if the parameter already exists
		{
			found = true;
			break;
		}
	}
	if (!found)
		return false;
	tmp_ufkt->m_parameters.list.erase(it);
	m_modified = true;
	return true;
}
int XParser::addFunction(const QString &f_str0, const QString &_f_str1)
{
	QString added_function(f_str0);
	QString f_str1(_f_str1);
	int const pos = added_function.indexOf(';');
	if (pos!=-1)
	  added_function = added_function.left(pos);
	
	fixFunctionName(added_function);
	if ( !f_str1.isEmpty() )
		fixFunctionName( f_str1 );
	
	Function::Type type;
	
	if ( !f_str1.isEmpty() )
		type = Function::Parametric;
	else if ( f_str0.count( '=' ) > 1 )
		type = Function::Implicit;
	else
		type = (added_function[0] == 'r') ? Function::Polar : Function::Cartesian;
	
	int const id = Parser::addFunction( added_function, f_str1, type );
	if (id==-1)
		return -1;
	Function *tmp_ufkt = m_ufkt[id];
	if ( pos!=-1 && !getext( tmp_ufkt, f_str0 ) )
	{
		Parser::removeFunction( tmp_ufkt );
		return -1;
	}
	m_modified = true;
	return id;
}

bool XParser::addFunction(const QString &fstr_const0, const QString &fstr_const1, bool f_mode, bool f1_mode, bool f2_mode, bool integral_mode, bool integral_use_precision, double linewidth, double f1_linewidth, double f2_linewidth, double integral_linewidth, const QString &str_dmin, const QString &str_dmax, const QString &str_startx, const QString &str_starty, double integral_precision, QColor color, QColor f1_color, QColor f2_color, QColor integral_color, QStringList str_parameter, int use_slider)
{
	QString fstr[2] = { fstr_const0, fstr_const1 };
	Function::Type type = Function::Cartesian;
	for ( unsigned i = 0; i < 2; ++i )
	{
		if ( fstr[i].isEmpty() )
			continue;
		
		switch ( fstr[i][0].unicode() )
		{
			case 'r':
			{
				fixFunctionName(fstr[i], Equation::Polar);
				type = Function::Polar;
				break;
			}
			case 'x':
				fixFunctionName(fstr[i], Equation::ParametricX);
				type = Function::Parametric;
				break;
			case 'y':
				fixFunctionName(fstr[i], Equation::ParametricY);
				type = Function::Parametric;
				break;
			default:
				fixFunctionName(fstr[i], Equation::Cartesian );
				type = Function::Cartesian;
				break;
		}
	}
	
	int const id = Parser::addFunction( fstr[0], fstr[1], type );
	if ( id==-1 )
		return false;
	Function *added_function = m_ufkt[id];
	
	PlotAppearance appearance;
	
	// f0
	appearance.visible = f_mode;
	appearance.color = color;
	appearance.lineWidth = linewidth;
	added_function->plotAppearance( Function::Derivative0 ) = appearance;
	
	// f1
	appearance.visible = f1_mode;
	appearance.color = f1_color;
	appearance.lineWidth = f1_linewidth;
	added_function->plotAppearance( Function::Derivative1 ) = appearance;
	
	// f2
	appearance.visible = f2_mode;
	appearance.color = f2_color;
	appearance.lineWidth = f2_linewidth;
	added_function->plotAppearance( Function::Derivative2 ) = appearance;
	
	// integral
	appearance.visible = integral_mode;
	appearance.color = integral_color;
	appearance.lineWidth = integral_linewidth;
	added_function->plotAppearance( Function::Integral ) = appearance;
	
	added_function->integral_use_precision = integral_use_precision;
	
	added_function->dmin.updateExpression( str_dmin );
	added_function->usecustomxmin = !str_dmin.isEmpty();
	
	added_function->dmax.updateExpression( str_dmax );
	added_function->usecustomxmax = !str_dmax.isEmpty();
	
	added_function->eq[0]->setIntegralStart( str_startx, str_starty );
	
	added_function->integral_precision = integral_precision;
	
	added_function->m_parameters.sliderID = use_slider;
	for( QStringList::Iterator it = str_parameter.begin(); it != str_parameter.end(); ++it )
	{
		added_function->m_parameters.list.append( *it );
	}
	m_modified = true;
	return true;
}

bool XParser::setFunctionExpression(const QString &f_str, uint id, uint eq)
{
	Function * tmp_ufkt = functionWithID( id );
	if ( !tmp_ufkt )
		return false;
	QString const old_fstr = tmp_ufkt->eq[eq]->fstr();
	QString const fstr_begin = tmp_ufkt->eq[eq]->fstr().left(tmp_ufkt->eq[eq]->fstr().indexOf('=')+1);
	
	return tmp_ufkt->eq[eq]->setFstr( fstr_begin+f_str );
}

