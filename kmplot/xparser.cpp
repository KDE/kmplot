/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter Möller <kd.moeller@t-online.de>
*               2006, 2007 David Saxton <david@bluehaze.org>
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

#include "xparser.h"

#include <kmplot/config-kmplot.h>

// local includes
#include "parseradaptor.h"
#include "maindlg.h"

// KDE includes
#include <KLocalizedString>
#include <KMessageBox>

#include <QList>
#include <QDebug>

#include <assert.h>
#include <cmath>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif


XParser * XParser::m_self = 0;

XParser * XParser::self()
{
	if ( !m_self )
		m_self = new XParser();
	
	return m_self;
}


XParser::XParser()
{
	differentialFinite = true;
	differentialDiverge = 0;

	new ParserAdaptor(this);
	QDBusConnection::sessionBus().registerObject(QStringLiteral("/parser"), this);
}

XParser::~XParser()
{
}

bool XParser::getext( Function *item, const QString &fstr )
{
  	bool errflg = false;
   	int p1, p2, p3, pe;
	QString tstr;
	pe = fstr.length();
	if ( fstr.indexOf( 'N' ) != -1 )
		item->plotAppearance( Function::Derivative0 ).visible = false;
	else
	{
		if ( fstr.indexOf( QLatin1String("A1") ) != -1 )
			item->plotAppearance( Function::Derivative1 ).visible = true;
		if ( fstr.indexOf( QLatin1String("A2") ) != -1 )
			item->plotAppearance( Function::Derivative2 ).visible = true;
	}
	switch ( fstr[0].unicode() )
	{
		case 'x':
		case 'y':
		case 'r':
			item->plotAppearance( Function::Derivative1 ).visible = item->plotAppearance( Function::Derivative2 ).visible = false;
	}

	p1 = fstr.indexOf( QLatin1String("D[") );
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
	p1 = fstr.indexOf( QLatin1String("P[") );
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


double XParser::derivative( int n, Equation * eq, DifferentialState * state, double x, double h )
{
	if ( n < -1 )
	{
		qCritical() << "Can't handle derivative < -1\n";
		return 0.0;
	}
	
	switch ( n )
	{
		case -1:
			return differential( eq, & eq->differentialStates[0], x, h );
		
		case 0:
			if ( state )
				return differential( eq, state, x, h );
			else
				return fkt( eq, x );
			
		case 1:
			if ( state )
				return ( differential(eq, state, x + (h/2), h ) - differential( eq, state, x - (h/2), h ) ) / h;
			else
				return ( fkt(eq, x + (h/2) ) - fkt( eq, x - (h/2) ) ) / h;
			
		default:
			return ( derivative( n-1, eq, state, x+(h/2), (h/4) ) - derivative( n-1, eq, state, x-(h/2), (h/4) ) ) / h;
	}
}


double XParser::partialDerivative( int n1, int n2, Equation * eq, DifferentialState * state, double x, double y, double h1, double h2 )
{
	if ( n1 < 0 || n2 < 0 )
	{
		qCritical() << "Can't handle derivative < 0\n";
		return 0.0;
	}
	
	if ( n1 > 0 )
		return ( partialDerivative( n1-1, n2, eq, state, x+(h1/2), y, (h1/4), h2 ) - partialDerivative( n1-1, n2, eq, state, x-(h1/2), y, (h1/4), h2 ) ) / h1;
	
	Function * f = eq->parent();
	f->m_implicitMode = Function::FixedX;
	f->x = x;
	
	return derivative( n2, eq, state, y, h2 );
}


QString XParser::findFunctionName( const QString & preferredName, int id, const QStringList& neededPatterns )
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
			
			for ( Function * it : qAsConst(m_ufkt) )
			{
				if ( int(it->id()) == id )
					continue;
				
				for ( Equation * eq : qAsConst(it->eq) )
				{
					for ( const QString& pattern : neededPatterns) {
						if ( eq->name() == pattern.arg(name) )
							ok = false;
					}
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
	
	if ( p1 < 0 )
		return;
	
	for ( int i = p2+1; i < p3; ++i )
	{
		if ( !str.at(i).isSpace() )
			return;
	}
	
	QString const fname = str.left(p1);
	for ( Function * it : qAsConst(m_ufkt) )
	{
		if ( int(it->id()) == id )
			continue;
		
		for ( Equation * eq : qAsConst(it->eq) )
		{
			if ( eq->name() != fname )
				continue;
			
			str = str.mid(p1,str.length()-1);
			QString function_name;
			if ( type == Equation::ParametricX )
				function_name = 'x';
			else if ( type == Equation::ParametricY )
				function_name = 'y';
			else
				function_name = 'f';
			function_name = findFunctionName( function_name, id );
			str.prepend( function_name );
			return;
		}
	}
}


Vector XParser::rk4_f( int order, Equation * eq, double x, const Vector & y )
{
	bool useParameter = eq->usesParameter();
	
	m_result.resize( order );
	m_arg.resize( order+1 + (useParameter ? 1 : 0) );
	
	m_arg[0] = x;
	
	if ( useParameter )
		m_arg[1] = eq->parent()->k;
	
	memcpy( m_arg.data() + 1 + (useParameter ? 1 : 0), y.data(), order*sizeof(double) );
	memcpy( m_result.data(), y.data() + 1, (order-1)*sizeof(double) );
	
	m_result[order-1] = XParser::fkt( eq, m_arg );
	
	return m_result;
}


double XParser::differential( Equation * eq, DifferentialState * state, double x_target, double max_dx )
{
	differentialFinite = true;
	
	if ( eq->order() < 1 )
	{
		qWarning() << "Zero order!\n";
		return 0;
	}
	
	max_dx = qAbs(max_dx);
	assert( max_dx > 0 ); // in case anyone tries to pass us a zero h
	
	// the difference between h and dx is that h is only used as a hint for the
	// stepwidth; dx is made similar to h in size, yet tiles the gap between x
	// and the previous x perfectly
	
	// see if the initial integral point in the function is closer to our
	// required x value than the last one (or the last point is invalid)
	if ( qAbs( state->x0.value() - x_target ) < qAbs( state->x - x_target ) )
		state->resetToInitial();
	
	int order = eq->order();
	
	m_k1.resize( order );
	m_k2.resize( order );
	m_k3.resize( order );
	m_k4.resize( order );
	m_y_temp.resize( order );
	
	double x = state->x;
	m_y = state->y;
	if ( x_target == x )
		return m_y[0];
	
	int intervals = int( qAbs(x_target-x)/max_dx + 1 );
	double dx = (x_target-x) / double(intervals);
	
	for ( int i = 0; i < intervals; ++i )
	{
		// Update differentialDiverge before y possible becomes infinite
		differentialDiverge = x;
		
		x = state->x + i*dx;
		
		
		m_k1 = rk4_f( order, eq, x, m_y );
		
		m_y_temp.combine( m_y, dx/2, m_k1 );
		m_k2 = rk4_f( order, eq, x + dx/2, m_y_temp);
		
		m_y_temp.combine( m_y, dx/2, m_k2 );
		m_k3 = rk4_f( order, eq, x + dx/2, m_y_temp );
		
		m_y_temp.combine( m_y, dx, m_k3 );
		m_k4 = rk4_f( order, eq, x + dx, m_y_temp );
		
		m_y.addRK4( dx, m_k1, m_k2, m_k3, m_k4 );
		
		if ( !std::isfinite(m_y[0]) )
		{
			differentialFinite = false;
			state->resetToInitial();
			return 0;
		}
	}
	
	state->x = x + dx;
	state->y = m_y;
	
	return m_y[0];
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
	
	assert( !"Should not happen - XParser::defaultColor" );
	return QColor();
}

QStringList XParser::listFunctionNames()
{
	return userFunctions();
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

bool XParser::setFunctionFVisible(uint id, bool visible)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative0 ).visible = visible;
	MainDlg::self()->requestSaveCurrentState();
	return true;
}
bool XParser::setFunctionF1Visible(uint id, bool visible)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative1 ).visible = visible;
	MainDlg::self()->requestSaveCurrentState();
	return true;
}
bool XParser::setFunctionF2Visible(uint id, bool visible)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative2 ).visible = visible;
	MainDlg::self()->requestSaveCurrentState();
	return true;
}
bool XParser::setFunctionIntVisible(uint id, bool visible)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Integral ).visible = visible;
	MainDlg::self()->requestSaveCurrentState();
	return true;
}

QString XParser::functionStr(uint id, uint eq)
{
	if ( !m_ufkt.contains( id ) || (eq>=2) )
		return QLatin1String("");
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
bool XParser::setFunctionFColor(uint id, const QColor &color)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative0 ).color = color;
	MainDlg::self()->requestSaveCurrentState();
	return true;
}
bool XParser::setFunctionF1Color(uint id, const QColor &color)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative1 ).color = color;
	MainDlg::self()->requestSaveCurrentState();
	return true;
}		
bool XParser::setFunctionF2Color(uint id, const QColor &color)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative2 ).color = color;
	MainDlg::self()->requestSaveCurrentState();
	return true;
}
bool XParser::setFunctionIntColor(uint id, const QColor &color)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Integral ).color = color;
	MainDlg::self()->requestSaveCurrentState();
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
bool XParser::setFunctionFLineWidth(uint id, double linewidth)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative0 ).lineWidth = linewidth;
	MainDlg::self()->requestSaveCurrentState();
	return true;
}
bool XParser::setFunctionF1LineWidth(uint id, double linewidth)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative1 ).lineWidth = linewidth;
	MainDlg::self()->requestSaveCurrentState();
	return true;
}		
bool XParser::setFunctionF2LineWidth(uint id, double linewidth)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Derivative2 ).lineWidth = linewidth;
	MainDlg::self()->requestSaveCurrentState();
	return true;
}
bool XParser::setFunctionIntLineWidth(uint id, double linewidth)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->plotAppearance( Function::Integral ).lineWidth = linewidth;
	MainDlg::self()->requestSaveCurrentState();
	return true;
}

QString XParser::functionMinValue(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return 0;
  return m_ufkt[id]->dmin.expression();
}

bool XParser::setFunctionMinValue(uint id, const QString &min)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->dmin.expression() = min;
	MainDlg::self()->requestSaveCurrentState();
  return true;
}

QString XParser::functionMaxValue(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return 0;
  return m_ufkt[id]->dmax.expression();
}

bool XParser::setFunctionMaxValue(uint id, const QString &max)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	m_ufkt[id]->dmax.expression() = max;
	MainDlg::self()->requestSaveCurrentState();
  return true;
}

bool XParser::setFunctionStartValue(uint id, const QString &x, const QString &y)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	DifferentialState * state = & m_ufkt[id]->eq[0]->differentialStates[0];
	state->x0.updateExpression( x );
	state->y0[0].updateExpression( y );
	MainDlg::self()->requestSaveCurrentState();
	return true;
}

QString XParser::functionStartXValue(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return 0;
	DifferentialState * state = & m_ufkt[id]->eq[0]->differentialStates[0];
	return state->x0.expression();
}


QString XParser::functionStartYValue(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return 0;
	DifferentialState * state = & m_ufkt[id]->eq[0]->differentialStates[0];
	return state->y0[0].expression();
}

QStringList XParser::functionParameterList(uint id)
{
	if ( !m_ufkt.contains( id ) )
		return QStringList();
	Function *item = m_ufkt[id];
	QStringList str_parameter;
	for ( const Value &it : qAsConst(item->m_parameters.list) )
		str_parameter << it.expression();
	return str_parameter;
}
bool XParser::functionAddParameter(uint id, const QString &new_parameter)
{
	if ( !m_ufkt.contains( id ) )
		return false;
	Function *tmp_ufkt = m_ufkt[id];
	
	//check if the parameter already exists
	for ( const Value &it : qAsConst(tmp_ufkt->m_parameters.list) )
	{
		if ( it.expression() == new_parameter )
			return false;
	}
	
	Value value;
	if ( !value.updateExpression( new_parameter ) )
		return false;
	tmp_ufkt->m_parameters.list.append( value );
	MainDlg::self()->requestSaveCurrentState();
	return true;
}
bool XParser::functionRemoveParameter(uint id, const QString &remove_parameter)
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
	MainDlg::self()->requestSaveCurrentState();
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
	MainDlg::self()->requestSaveCurrentState();
	return id;
}

bool XParser::addFunction(const QString &fstr_const0, const QString &fstr_const1, bool f_mode, bool f1_mode, bool f2_mode, bool integral_mode, double linewidth, double f1_linewidth, double f2_linewidth, double integral_linewidth, const QString &str_dmin, const QString &str_dmax, const QString &str_startx, const QString &str_starty, double integral_precision, const QColor &color, const QColor &f1_color, const QColor &f2_color, const QColor &integral_color, const QStringList & str_parameter, int use_slider)
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
	
	added_function->dmin.updateExpression( str_dmin );
	added_function->usecustomxmin = !str_dmin.isEmpty();
	
	added_function->dmax.updateExpression( str_dmax );
	added_function->usecustomxmax = !str_dmax.isEmpty();
	
	DifferentialState * state = & added_function->eq[0]->differentialStates[0];
	state->x0.updateExpression( str_startx );
	state->y0[0].updateExpression( str_starty );
	
	added_function->eq[0]->differentialStates.setStep( Value( integral_precision ) );
	
	added_function->m_parameters.sliderID = use_slider;
	for( QStringList::ConstIterator it = str_parameter.begin(); it != str_parameter.end(); ++it )
	{
		added_function->m_parameters.list.append( *it );
	}
	MainDlg::self()->requestSaveCurrentState();
	return true;
}

bool XParser::setFunctionExpression(uint id, uint eq, const QString &f_str)
{
	Function * tmp_ufkt = functionWithID( id );
	if ( !tmp_ufkt )
		return false;
	QString const old_fstr = tmp_ufkt->eq[eq]->fstr();
	QString const fstr_begin = tmp_ufkt->eq[eq]->fstr().left(tmp_ufkt->eq[eq]->fstr().indexOf('=')+1);
	
	return tmp_ufkt->eq[eq]->setFstr( fstr_begin+f_str );
}

