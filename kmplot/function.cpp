/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter Möller <kd.moeller@t-online.de>
*               2006 David Saxton <david@bluehaze.org>
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

#include "function.h"
#include "ksliderwindow.h"
#include "settings.h"
#include "view.h"
#include "xparser.h"

#include <QImage>
#include <QLinearGradient>
#include <QPainter>

#include <assert.h>
#include <cmath>

int MAX_PM = 4;


//BEGIN class Value
Value::Value( const QString & expression )
{
	m_value = 0.0;
	if ( expression.isEmpty() )
		m_expression = '0';
	else
		updateExpression( expression );
}


Value::Value( double value )
{
	updateExpression( value );
}


bool Value::updateExpression( const QString & expression )
{
	Parser::Error error;
	double newValue = XParser::self()->eval( expression, & error );
	if ( error != Parser::ParseSuccess )
		return false;
	
	m_value = newValue;
	m_expression = expression;
	return true;
}


void Value::updateExpression( double value )
{
	m_value = value;
	m_expression = Parser::number( value );
}


bool Value::operator == ( const Value & other ) const
{
	return m_expression == other.expression();
}
//END class Value



//BEGIN class PlotAppearance
PlotAppearance::PlotAppearance( )
{
	lineWidth = 0.3;
	color = Qt::black;
	useGradient = false;
	visible = false;
	style = Qt::SolidLine;
	showExtrema = false;
	showTangentField = false;
	showPlotName = false;
}


bool PlotAppearance::operator !=( const PlotAppearance & other ) const
{
	return
			(lineWidth != other.lineWidth) ||
			(color != other.color) ||
			(useGradient != other.useGradient) ||
			(gradient.stops() != other.gradient.stops()) ||
			(visible != other.visible) ||
			(style != other.style) ||
			(showExtrema != other.showExtrema) ||
			(showTangentField != other.showTangentField) ||
			(showPlotName != other.showPlotName);
}


QString PlotAppearance::penStyleToString( Qt::PenStyle style )
{
	switch ( style )
	{
		case Qt::NoPen:
			return "NoPen";
			
		case Qt::SolidLine:
			return "SolidLine";
			
		case Qt::DashLine:
			return "DashLine";
			
		case Qt::DotLine:
			return "DotLine";
			
		case Qt::DashDotLine:
			return "DashDotLine";
		
		case Qt::DashDotDotLine:
			return "DashDotDotLine";
			
		case Qt::MPenStyle:
		case Qt::CustomDashLine:
			qWarning() << "Unsupported pen style\n";
			break;
	}
	
	qWarning() << "Unknown style " << style ;
	return "SolidLine";
}


Qt::PenStyle PlotAppearance::stringToPenStyle( const QString & style )
{
	if ( style == "NoPen" )
		return Qt::NoPen;
			
	if ( style == "SolidLine" )
		return Qt::SolidLine;
			
	if ( style == "DashLine" )
		return Qt::DashLine;
			
	if ( style == "DotLine" )
		return Qt::DotLine;
			
	if ( style == "DashDotLine" )
		return Qt::DashDotLine;
		
	if ( style == "DashDotDotLine" )
		return Qt::DashDotDotLine;
	
	qWarning() << "Unknown style " << style ;
	return Qt::SolidLine;
}
//END class PlotAppearance



//BEGIN class DifferentialState
DifferentialState::DifferentialState()
{
	x = 0;
}


DifferentialState::DifferentialState( int order )
{
	x = 0;
	setOrder( order );
}


void DifferentialState::setOrder( int order )
{
	bool orderWasZero = (y0.size() == 0);
	
	y.resize( order );
	y0.resize( order );
	
	if ( orderWasZero && order >= 1 )
		y0[0].updateExpression( "1" );
	
	resetToInitial();
}


bool DifferentialStates::setStep( const Value & step )
{
	if ( step.value() <= 0 )
		return false;
	
	m_step = step;
	return true;
}


void DifferentialState::resetToInitial()
{
	x = x0.value();
	y = y0;
}


bool DifferentialState::operator == ( const DifferentialState & other ) const
{
	return (x0 == other.x0) && (x == other.x) && (y0 == other.y0) && (y == other.y);
}
//END class DifferentialState



//BEGIN class DifferentialStates
DifferentialStates::DifferentialStates()
{
	m_uniqueState = false;
	m_order = 0;
	m_step.updateExpression( 0.05 );
}


void DifferentialStates::setOrder( int order )
{
	m_order = order;
	for ( int i = 0; i < m_data.size(); ++i )
		m_data[i].setOrder( order );
}


DifferentialState * DifferentialStates::add()
{
	if ( !m_uniqueState || m_data.isEmpty() )
		m_data << DifferentialState( order() );
	else
		qDebug() << "Unable to add another state!\n";
	
	return & m_data[ size() - 1 ];
}


void DifferentialStates::setUniqueState( bool unique )
{
	m_uniqueState = unique;
	if ( m_uniqueState && m_data.size() > 1 )
	{
		// Remove any states other than the first
		m_data.resize( 1 );
	}
}


void DifferentialStates::resetToInitial( )
{
	for ( int i = 0; i < m_data.size(); ++i )
		m_data[i].resetToInitial();
}
//END class DifferentialStates



//BEGIN class Equation
Equation::Equation( Type type, Function * parent )
	: m_type( type ),
	  m_parent( parent )
{
	m_usesParameter = false;
	mptr = 0;
	
	if ( type == Differential || type == Cartesian )
	{
		differentialStates.setUniqueState( type == Cartesian );
		differentialStates.setOrder( order() );
		differentialStates.add();
	}
}


Equation::~ Equation()
{
}


int Equation::order( ) const
{
	if ( type() == Cartesian )
	{
		// For drawing integrals
		return 1;
	}
	else
		return name(false).count( '\'' );
}


int Equation::pmCount() const
{
	return m_fstr.count( PmSymbol );
}


QString Equation::name( bool removePrimes ) const
{
	if ( m_fstr.isEmpty() )
		return QString();
	
	int open = m_fstr.indexOf( '(' );
	int equals = m_fstr.indexOf( '=' );
	
	if ( (equals == -1) && (open == -1) )
		return QString();
	
	int pos;
	if ( ((equals > open) && (open != -1)) || (equals == -1) )
		pos = open;
	else
		pos = equals;
	
	QString n = m_fstr.left( pos ).trimmed();
	
	if ( removePrimes )
		n.remove( '\'' );
	
	return n;
}


bool Equation::looksLikeFunction( ) const
{
	int open = m_fstr.indexOf( '(' );
	int equals = m_fstr.indexOf( '=' );
	
	if ( (open != -1) && (open < equals) )
		return true;
	
	switch ( type() )
	{
		case Cartesian:
		case Differential:
		case ParametricY:
			return (name() != "y");
			
		case Polar:
			return (name() != "r");
		
		case ParametricX:
			return (name() != "x");
		
		case Implicit:
			return false;
            
        case Constant:
            return false;
	}
	
	return true;
}


void Equation::updateVariables()
{
    if ( type() == Constant )
    {
        return;
    }
    
	m_variables.clear();
	
	if ( looksLikeFunction() )
	{
		int p1 = m_fstr.indexOf( '(' );
		int p2 = m_fstr.indexOf( ')' );
	
		const QStringList listSplit = ( (p1 != -1) && (p2 != -1) ) ? m_fstr.mid( p1+1, p2-p1-1 ).split( ',', QString::SkipEmptyParts ) : QStringList();
		
		// Variables shouldn't contain spaces!
		for ( QString s : listSplit ) {
			s = s.remove(' ');
			if ( !s.isEmpty() )
				m_variables << s;
		}
	}
	else switch ( type() )
	{
		case Cartesian:
		case Differential:
			m_variables << "x" << "k";
			break;
			
		case Polar:
			m_variables << QChar( 0x3b8 ) << "k"; // (theta)
			break;
		
		case ParametricX:
		case ParametricY:
			m_variables << "t" << "k";
			break;
		
		case Implicit:
			m_variables << "x" << "y" << "k";
			break;
            
        case Constant:
            break;
	}
	
	// If we are a differential equation, then add on y, y', etc
	if ( type() == Differential && !name().isEmpty() )
	{
		QString n = name();
		
		int order = this->order();
		for ( int i = 0; i < order; ++i )
		{
			m_variables << n;
			n += '\'';
		}
	}
	
	
	//BEGIN Update whether we accept a parameter or not
	int expectedNumVariables = 0;
	
	switch ( m_type )
	{
		case Cartesian:
		case ParametricX:
		case ParametricY:
		case Polar:
			expectedNumVariables = 1;
			break;
			
		case Implicit:
			expectedNumVariables = 2;
			break;
			
		case Differential:
			expectedNumVariables = order()+1;
			break;
            
		case Constant:
			expectedNumVariables = 0;
			break;
	}
	
	m_usesParameter = (variables().size() > expectedNumVariables);
	//END Update whether we accept a parameter or not
}


QString Equation::parameterName( ) const
{
	if ( !usesParameter() )
		return QString();
	
	int parAt = (type() == Implicit) ? 2 : 1;
	return variables()[parAt];
}


bool Equation::setFstr( const QString & fstr, int * error, int * errorPosition, bool force )
{
#define HANDLE_ERROR \
	if ( !force ) \
	{ \
		m_fstr = prevFstr; \
		updateVariables(); \
	} \
	else \
	{ \
		qDebug() << "fstr "<<fstr<<" invalid, but forcing anyway: " << Parser::errorString( Parser::Error(*error) ) << " at position " << *errorPosition; \
		mem.clear(); \
	}
	
	int temp1, temp2;
	if ( !error )
		error = & temp1;
	if ( !errorPosition )
		errorPosition = & temp2;
	
	*error = Parser::ParseSuccess;
	*errorPosition = -1;
	
	QString prevFstr = m_fstr;
	m_fstr = fstr;
	updateVariables();
	
	if ( !fstr.contains('=') || QString(fstr).right( fstr.length() - fstr.indexOf('=') - 1).simplified().isEmpty() )
	{
		*error = Parser::SyntaxError;
		HANDLE_ERROR;
		return false;
	}
	
	// require order to be greater than 0 for differential equations
	if ( (type() == Differential) && (order() < 1) )
	{
		*error = Parser::ZeroOrder;
		HANDLE_ERROR;
		/// \todo indicate the position of the error
		return false;
	}
	
	int maxArg = order() + (( type() == Implicit ) ? 3 : 2);
	if ( variables().size() > maxArg )
	{
		*error = Parser::TooManyArguments;
		HANDLE_ERROR;
		/// \todo indicate the position of the invalid argument?
		return false;
	}
	
	XParser::self()->initEquation( this, (Parser::Error*)error, errorPosition );
	if ( *error != Parser::ParseSuccess )
	{
		HANDLE_ERROR;
		if ( !force )
			XParser::self()->initEquation( this );
		return false;
	}
	
	differentialStates.setOrder( order() );
	return true;
}


void Equation::setPMSignature( QVector< bool > pmSignature )
{
	differentialStates.resetToInitial();
	m_pmSignature = pmSignature;
}


bool Equation::operator !=( const Equation & other )
{
	return (fstr() != other.fstr()) ||
			(differentialStates != other.differentialStates);
}


Equation & Equation::operator =( const Equation & other )
{
	setFstr( other.fstr() );
	differentialStates = other.differentialStates;
	
	return * this;
}
//END class Equation



//BEGIN class Function
Function::Function( Type type )
	: m_type( type )
{
	x = y = 0;
	m_implicitMode = UnfixedXY;
	
	usecustomxmin = false;
	usecustomxmax = false;
	
	dmin.updateExpression( QChar('0') );
	if ( Settings::anglemode() == Parser::Radians )
		dmax.updateExpression( QString(QChar('2')) + PiSymbol );
	else
		dmax.updateExpression( "360" );
	
	switch ( m_type )
	{
		case Cartesian:
			eq << new Equation( Equation::Cartesian, this );
			break;
			
		case Polar:
			eq << new Equation( Equation::Polar, this );
			usecustomxmin = true;
			usecustomxmax = true;
			break;
			
		case Parametric:
			eq << new Equation( Equation::ParametricX, this );
			eq << new Equation( Equation::ParametricY, this );
			usecustomxmin = true;
			usecustomxmax = true;
			break;
			
		case Implicit:
			eq << new Equation( Equation::Implicit, this );
			break;
			
		case Differential:
			eq << new Equation( Equation::Differential, this );
			break;
	}
	
	m_id = 0;
	f0.visible = true;
	
	k = 0;
}


Function::~Function()
{
	for ( Equation * e : qAsConst(eq) )
		delete e;
}


bool Function::copyFrom( const Function & function )
{
	bool changed = false;
	int i = 0;
#define COPY_AND_CHECK(s) \
	{ \
		if ( s != function.s ) \
		{ \
			s = function.s; \
			changed = true; \
		} \
	} \
	i++;
	
	COPY_AND_CHECK( f0 );				// 0
	if ( type() == Cartesian )
	{
		COPY_AND_CHECK( f1 );				// 1
		COPY_AND_CHECK( f2 );				// 2
		COPY_AND_CHECK( f3 );				// 3
		COPY_AND_CHECK( integral );			// 4
	}
	COPY_AND_CHECK( dmin );				// 5,1
	COPY_AND_CHECK( dmax );				// 6,2
	COPY_AND_CHECK( usecustomxmin );	// 7,3
	COPY_AND_CHECK( usecustomxmax );	// 8,4
	COPY_AND_CHECK( m_parameters );		// 9,5
	
	// handle equations separately
	for ( int i = 0; i < eq.size(); ++i )
	{
		if ( *eq[i] != *function.eq[i] )
		{
			changed = true;
			*eq[i] = *function.eq[i];
		}
	}
	
	return changed;
}


QString Function::name() const
{
	QString n = eq[0]->fstr();
	for ( int i = 1; i < eq.size(); ++i )
		n += '\n' + eq[i]->fstr();
	
	return n;
}


PlotAppearance & Function::plotAppearance( PMode plot )
{
	// NOTE: This function is identical to the const one, so changes to this should be applied to both
	
	switch ( plot )
	{
		case Function::Derivative0:
			return f0;
		case Function::Derivative1:
			return f1;
		case Function::Derivative2:
			return f2;
		case Function::Derivative3:
			return f3;
		case Function::Integral:
			return integral;
	}
	
	qCritical() << "Unknown plot " << plot << endl;
	return f0;
}
PlotAppearance Function::plotAppearance( PMode plot ) const
{
	// NOTE: This function is identical to the none-const one, so changes to this should be applied to both
	
	switch ( plot )
	{
		case Function::Derivative0:
			return f0;
		case Function::Derivative1:
			return f1;
		case Function::Derivative2:
			return f2;
		case Function::Derivative3:
			return f3;
		case Function::Integral:
			return integral;
	}
	
	qCritical() << "Unknown plot " << plot << endl;
	return f0;
}


bool Function::allPlotsAreHidden( ) const
{
	return !f0.visible && !f1.visible && !f2.visible && !integral.visible;
}


QString Function::typeToString( Type type )
{
	switch ( type )
	{
		case Cartesian:
			return "cartesian";
			
		case Parametric:
			return "parametric";
			
		case Polar:
			return "polar";
			
		case Implicit:
			return "implicit";
			
		case Differential:
			return "differential";
	}
	
	qWarning() << "Unknown type " << type ;
	return "unknown";
}


Function::Type Function::stringToType( const QString & type )
{
	if ( type == "cartesian" )
		return Cartesian;
	
	if ( type == "parametric" )
		return Parametric;
	
	if ( type == "polar" )
		return Polar;
	
	if ( type == "implicit" )
		return Implicit;
	
	if ( type == "differential" )
		return Differential;
	
	qWarning() << "Unknown type " << type ;
	return Cartesian;
}


QList< Plot > Function::plots( PlotCombinations combinations ) const
{
	QList< Plot > list;
	
	if ( allPlotsAreHidden() )
		return list;
	
	Plot plot;
	plot.setFunctionID( id() );
	plot.plotNumberCount = m_parameters.useList ? m_parameters.list.size() + (m_parameters.useSlider?1:0) : 1;
	
	bool singlePlot = (!m_parameters.useList && !m_parameters.useSlider) ||
			m_parameters.animating ||
			(~combinations & DifferentParameters) ||
			(!m_parameters.useSlider && m_parameters.useList && m_parameters.list.isEmpty());
	
	if ( singlePlot )
	{
		if ( m_parameters.animating )
			plot.parameter = Parameter( Parameter::Animated );
		
		list << plot;
	}
	else
	{
		int i = 0;
		
		if ( m_parameters.useSlider )
		{
			Parameter param( Parameter::Slider );
			param.setSliderID( m_parameters.sliderID );
			plot.parameter = param;
			plot.plotNumber = i++;
			list << plot;
		}
		
		if ( m_parameters.useList )
		{
			const int listsize = m_parameters.list.size();
			for ( int pos = 0; pos < listsize; ++pos )
			{
				Parameter param( Parameter::List );
				param.setListPos( pos );
				plot.parameter = param;
				plot.plotNumber = i++;
				list << plot;
			}
		}
	}
	
	
	// Copy each plot in the list for other variations
	if ( (type() == Cartesian) && (combinations & DifferentDerivatives) )
	{
		QList< Plot > duplicated;
		
		for ( PMode p = Derivative0; p <= Integral; p = PMode(p+1) )
		{
			for ( Plot plot : qAsConst(list) ) {
				if ( !plotAppearance(p).visible )
					continue;
				plot.plotMode = p;
				duplicated << plot;
			}
		}
		
		list = duplicated;
	}
	
	if ( (type() == Differential) && (combinations & DifferentInitialStates) )
	{
		QList< Plot > duplicated;
		
		for ( int i = 0; i < eq[0]->differentialStates.size(); ++i )
		{
			for ( Plot plot : qAsConst(list) ) {
				plot.stateNumber = i;
				duplicated << plot;
			}
		}
		
		list = duplicated;
	}
	
	if ( combinations & DifferentPMSignatures )
	{
		int size = 0;
		for ( Equation * equation : qAsConst(eq) )
			size += equation->pmCount();
	
		unsigned max = unsigned( std::pow( 2.0, (double)size ) );
		QVector< QVector<bool> > signatures( max );
	
		for ( unsigned i = 0; i < max; ++i )
		{
			QVector<bool> sig( size );
			
			for ( int j = 0; j < size; ++j )
				sig[ j ] = i & (1<<j);
			
			signatures[i] = sig;
		}
	
		// Generate a plot for each signature in signatures
		QList< Plot > duplicated;
		for ( const QVector<bool> &signature : qAsConst(signatures) )
		{
			int at = 0;
			QList< QVector<bool> > pmSignature;
		
			for ( Equation * equation : qAsConst(eq) )
			{
				int pmCount = equation->pmCount();
				QVector<bool> sig( pmCount );
				for ( int i = 0; i < pmCount; ++i )
					sig[i] = signature[ i + at];
				at += pmCount;
			
				pmSignature << sig;
			}
		
			for ( Plot plot : qAsConst(list) ) {
				plot.pmSignature = pmSignature;
				duplicated << plot;
			}
		}
		list = duplicated;
	}
	
	return list;
}


void Function::addFunctionDependency( Function * function )
{
	if ( !function || m_dependencies.contains( function->id() ) )
		return;
	
	Q_ASSERT_X( !function->dependsOn( this ), "addFunctionDependency", "circular dependency" );
	
	m_dependencies << function->id();
}


bool Function::dependsOn( Function * function ) const
{
	if ( !function )
		return false;
	
	if ( m_dependencies.contains( function->id() ) )
		return true;
	
	for ( int functionId : qAsConst(m_dependencies) )
	{
		Function * f = XParser::self()->functionWithID( functionId );
		
		if ( f->dependsOn( function ) )
			return true;
	}
	
	return false;
}
//END class Function



//BEGIN class ParameterSettings
ParameterSettings::ParameterSettings()
{
	animating = false;
	useSlider = false;
	sliderID = 0;
	useList = false;
}


bool ParameterSettings::operator == ( const ParameterSettings & other ) const
{
	return ( useSlider == other.useSlider ) &&
			( sliderID == other.sliderID ) &&
			( useList == other.useList ) &&
			( list == other.list );
}
//END class ParameterSettings



//BEGIN class Parameter
Parameter::Parameter( Type type )
	: m_type( type )
{
	m_sliderID = -1;
	m_listPos = -1;
}


bool Parameter::operator == ( const Parameter & other ) const
{
	return ( type() == other.type() ) &&
			( listPos() == other.listPos() ) &&
			( sliderID() == other.sliderID() );
}
//END class Parameter



//BEGIN class Plot
Plot::Plot( )
{
	stateNumber = -1;
	plotNumberCount = 1;
	plotNumber = 0;
	m_function = 0;
	m_functionID = -1;
	plotMode = Function::Derivative0;
}


bool Plot::operator ==( const Plot & other ) const
{
	return ( m_functionID == other.functionID() ) &&
			( plotMode == other.plotMode ) &&
			( parameter == other.parameter ) &&
			( stateNumber == other.stateNumber );
}


void Plot::setFunctionID( int id )
{
	m_functionID = id;
	updateCached();
}


void Plot::updateCached()
{
	m_function = XParser::self()->functionWithID( m_functionID );
}


QString Plot::name( ) const
{
	if ( !m_function )
		return QString();
	
	QString n = m_function->name();
	
	if ( m_function->eq[0]->usesParameter() )
		n += QString( "\n%1 = %2" ).arg( m_function->eq[0]->parameterName() ).arg( Parser::number( parameterValue() ) );

	if ( plotMode == Function::Derivative1 )
		n = n.section('=', 0, 0).replace('(', "\'(");

	if ( plotMode == Function::Derivative2 )
		n = n.section('=', 0, 0).replace('(', "\'\'(");

	if ( plotMode == Function::Integral )
	{
		QString functionName = n.section('=', 0, 0);
		n = QChar(0x222B) + ' ' + functionName + 'd' + functionName.section('(', 1, 1).remove(')').section(',', 0, 0);
	}

	return n;
}


void Plot::updateFunction() const
{
	if ( !m_function )
		return;
	
	// Update the plus-minus signature
	assert( pmSignature.size() <= m_function->eq.size() );
	for ( int i = 0; i < pmSignature.size(); ++i )
		m_function->eq[i]->setPMSignature( pmSignature[i] );
	
	if ( parameter.type() != Parameter::Animated )
		m_function->setParameter( parameterValue() );
}


double Plot::parameterValue() const
{
	switch ( parameter.type() )
	{
		case Parameter::Unknown:
			return 0;
			
		case Parameter::Slider:
		{
			KSliderWindow * sw = View::self()->m_sliderWindow;
			
			if ( !sw )
			{
				// Slider window isn't open. Ask View to open it
				View::self()->updateSliders();
				
				// It should now be open
				sw = View::self()->m_sliderWindow;
				assert( sw );
			}
			
			return sw->value( parameter.sliderID() );
		}
			
		case Parameter::List:
		{
			if ( (parameter.listPos() >= 0) && (parameter.listPos() < m_function->m_parameters.list.size()) )
				return m_function->m_parameters.list[ parameter.listPos() ].value();
			return 0;
		}
		
		case Parameter::Animated:
		{
			qWarning() << "Shouldn't use this function for animated parameter!\n";
			return 0;
		}
	}
	
	return 0;
}


void Plot::differentiate()
{
	switch ( plotMode )
	{
		case Function::Integral:
			plotMode = Function::Derivative0;
			break;

		case Function::Derivative0:
			plotMode = Function::Derivative1;
			break;

		case Function::Derivative1:
			plotMode = Function::Derivative2;
			break;

		case Function::Derivative2:
			plotMode = Function::Derivative3;
			break;

		case Function::Derivative3:
			qWarning() << "Can't handle this yet!\n";
			break;
	}
}


void Plot::integrate()
{
	switch ( plotMode )
	{
		case Function::Integral:
			qWarning() << "Can't handle this yet!\n";
			break;

		case Function::Derivative0:
			plotMode = Function::Integral;
			break;

		case Function::Derivative1:
			plotMode = Function::Derivative0;
			break;

		case Function::Derivative2:
			plotMode = Function::Derivative1;
			break;

		case Function::Derivative3:
			plotMode = Function::Derivative2;
			break;
	}
}


QColor Plot::color( ) const
{
	Function * f = function();
	assert(f); // Shouldn't call color without a function
	PlotAppearance appearance = f->plotAppearance( plotMode );
	
	if ( (plotNumberCount <= 1) || !appearance.useGradient )
		return appearance.color;
	
	// Is a gradient
	
	int x = plotNumber;
	int l = plotNumberCount;
	
	QLinearGradient lg( 0, 0, l-1, 0 );
	lg.setStops( appearance.gradient.stops() );
	QImage im( l, 1, QImage::Format_RGB32 );
	QPainter p(&im);
	p.setPen( QPen( lg, 1 ) );
	p.drawLine( 0, 0, l, 0 );
	return im.pixel( x, 0 );
}


int Plot::derivativeNumber( ) const
{
	switch ( plotMode )
	{
		case Function::Integral:
			return -1;
		case Function::Derivative0:
			return 0;
		case Function::Derivative1:
			return 1;
		case Function::Derivative2:
			return 2;
		case Function::Derivative3:
			return 3;
	}
	
	qWarning() << "Unknown derivative number.\n";
	return 0;
}


DifferentialState * Plot::state( ) const
{
	if ( !function() || (stateNumber < 0) )
		return 0;
	
	if ( function()->eq[0]->differentialStates.size() <= stateNumber )
		return 0;
	
	return & function()->eq[0]->differentialStates[stateNumber];
}
//END class Plot

