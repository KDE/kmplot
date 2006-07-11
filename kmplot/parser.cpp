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


// local includes
#include "parser.h"
#include "parseradaptor.h"
#include "settings.h"
#include "xparser.h"

//KDE includes
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>

#include <QList>

// standard c(++) includes
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <cmath>

double Parser::m_radiansPerAngleUnit = 0;

/**
 * List of predefined functions.
 * \todo Some of the functions here are just variations of different spellings
 * of the same function (e.g. arccosh, arcosh) - it might be neater to handle
 * these duplications better (in particular, it would prevent duplication in the
 * drop-down list of functions in the EquationEditor dialog).
 */
ScalarFunction Parser::scalarFunctions[ ScalarCount ]=
{
	// Trigometric functions
	{"sin", lsin}, 			// Sinus
	{"cos", lcos}, 			// Cosinus
	{"tan", ltan}, 			// Tangens
	{"arcsin", arcsin}, 	// Arcus sinus = inverse of sin
	{"arccos", arccos}, 	// Arcus cosinus = inverse of cos
	{"arctan", arctan},		// Arcus tangens = inverse of tan
	
	// Hyperbolic trig
	{"sinh", lsinh},	 	// Sinus hyperbolicus
	{"cosh", lcosh}, 		// Cosinus hyperbolicus
	{"tanh", ltanh},		// Tangens hyperbolicus
	{"arcsinh", arsinh}, 	// Area-sinus hyperbolicus = inverse of sinh
	{"arccosh", arcosh}, 	// Area-cosinus hyperbolicus = inverse of cosh
	{"arctanh", artanh}, 	// Area-tangens hyperbolicus = inverse of tanh
	{"arsinh", arsinh},		// The same as arcsinh
	{"arcosh", arcosh},		// The same as arccosh
	{"artanh", artanh},		// The same as arctanh
	
	// Reciprocal-trig
	{"cosec", cosec},		// Co-Secans = 1/sin
	{"sec", sec},			// Secans = 1/cos
	{"cot", cot},			// Co-Tangens = 1/tan
	{"arccosec", arccosec},	// Arcus co-secans = inverse of cosec
	{"arcsec", arcsec},		// Arcus secans = inverse of sec
	{"arccot", arccot},		// Arcus co-tangens = inverse of cotan
	
	// Reciprocal-hyperbolic
	{"cosech", cosech},		// Co-Secans hyperbolicus
	{"sech", sech},			// Secans hyperbolicus
	{"coth", coth},			// Co-Tangens hyperbolicus
	{"arccosech", arcosech},// Area-co-secans hyperbolicus = inverse of cosech
	{"arcsech", arsech},	// Area-secans hyperbolicus = invers of sech
	{"arccoth", arcoth},	// Area-co-tangens hyperbolicus = inverse of coth
	{"arcosech", arcosech},	// Same as arccosech
	{"arsech", arsech},		// Same as arcsech
	{"arcoth", arcoth},		// Same as arccoth
	
	// Other
	{"sqrt", sqrt},			// Square root
	{"sqr", sqr}, 			// Square
	{"sign", sign},			// Signum
	{"H", heaviside},		// Heaviside step function
	{"log", llog},			// Logarithm base 10
	{"ln", ln}, 			// Logarithm base e
	{"exp", exp}, 			// Exponential function base e
	{"abs", fabs},			// Absolute value
	{"floor", floor},		// round down to nearest integer
	{"ceil", ceil},			// round up to nearest integer
	{"round", round},		// round to nearest integer
	{"P_0", legendre0},		// lengedre polynomial (n=0)
	{"P_1", legendre1},		// lengedre polynomial (n=1)
	{"P_2", legendre2},		// lengedre polynomial (n=2)
	{"P_3", legendre3},		// lengedre polynomial (n=3)
	{"P_4", legendre4},		// lengedre polynomial (n=4)
	{"P_5", legendre5},		// lengedre polynomial (n=5)
	{"P_6", legendre6},		// lengedre polynomial (n=6)
};

VectorFunction Parser::vectorFunctions[ VectorCount ]=
{
	{"min", min},			// minimum of a set of reals
	{"max", max},			// maximum of a set of reals
	{"mod", mod},			// l2 modulus of a set of reals
};



//BEGIN class Parser
Parser::Parser()
	: m_sanitizer( this )
{
	m_errorPosition = -1;
	m_evalPos = 0;
	m_nextFunctionID = 0;
	m_stack = new double [STACKSIZE];
	m_constants = new Constants( this );
	
	m_ownEquation = 0;
	m_currentEquation = 0;
}


Parser::~Parser()
{
	foreach ( Function * function, m_ufkt )
		delete function;
	delete m_ownEquation;
	delete m_constants;
	delete [] m_stack;
}


QStringList Parser::predefinedFunctions( ) const
{
	QStringList names;
	
	for ( int func = 0; func < ScalarCount; ++func )
		names << scalarFunctions[func].name;
			
	for ( int func = 0; func < VectorCount; ++func )
		names << vectorFunctions[func].name;
	
	names.sort();
	return names;
}


QStringList Parser::userFunctions( ) const
{
	QStringList names;
	
	foreach ( Function * f, m_ufkt )
	{
		foreach ( Equation * eq, f->eq )
			names << eq->name();
	}
	
	names.sort();
	return names;
}


void Parser::setAngleMode( AngleMode mode )
{
	switch ( mode )
	{
		case Radians:
			m_radiansPerAngleUnit = 1.0;
			break;
			
		case Degrees:
			m_radiansPerAngleUnit = M_PI/180;	
			break;
	}
}


double Parser::radiansPerAngleUnit()
{
	return m_radiansPerAngleUnit;
}


uint Parser::getNewId()
{
	uint i = m_nextFunctionID;
	while (1)
	{
		if ( !m_ufkt.contains( i ) )
		{
			m_nextFunctionID = i+1;
			return i;
		}
		++i;
	}
}

double Parser::eval( const QString & str )
{
	if ( !m_ownEquation )
		m_ownEquation = new Equation( Equation::Cartesian, 0 );
	
	if ( !m_ownEquation->setFstr( QString( "%1=%2" ).arg( XParser::self()->findFunctionName( "f", -1 ) ).arg( str ) ) )
		return 0;
	
	return fkt( m_ownEquation, Vector() );
}


double Parser::fkt(uint const id, int eq, double x )
{
	if ( !m_ufkt.contains( id ) || m_ufkt[id]->eq.size() <= eq )
	{
		m_error = NoSuchFunction;
		return 0;
	}
	
	return fkt( m_ufkt[id]->eq[eq], x );
}


double Parser::fkt( Equation * eq, double x )
{
	Function * function = eq->parent();
	
	switch ( function->type() )
	{
		case Function::Cartesian:
		case Function::Parametric:
		case Function::Polar:
		{
			Vector var(2);
			var[0] = x;
			var[1] = function->k;
			
			return fkt( eq, var );
		}
			
		case Function::Implicit:
		{
			Vector var(3);
			
			// Can only calculate when one of x, y is fixed
			assert( function->m_implicitMode != Function::UnfixedXY );
			
			if ( function->m_implicitMode == Function::FixedX )
			{
				var[0] = function->x;
				var[1] = x;
			}
			else
			{
				// fixed y
				var[0] = x;
				var[1] = function->y;
			}
			var[2] = function->k;
			
			return fkt( eq, var );
		}
		
		case Function::Differential:
		{
			kError() << k_funcinfo << "Do not use this function directly! Instead, call XParser::differential\n";
			return 0;
		}
	}
	
	kWarning() << k_funcinfo << "Unknown function type!\n";
	return 0;
}


double Parser::fkt( Equation * eq, const Vector & x )
{
	double *pDouble;
	double (**pScalarFunction)(double);
	double (**pVectorFunction)(const Vector &);
	uint *pUint;
	eq->mptr = eq->mem;
	double *stkptr = m_stack;

	while(1)
	{
		switch(*eq->mptr++)
		{
			case KONST:
				pDouble=(double*)eq->mptr;
				*stkptr=*pDouble++;
				eq->mptr=(unsigned char*)pDouble;
				break;
				
			case VAR:
			{
				pUint = (uint*)eq->mptr;
				uint var = *pUint++;
				assert( int(var) < x.size() );
				*stkptr = x[var];
				eq->mptr = (unsigned char*)pUint;
				break;
			}
				
			case PUSH:
				++stkptr;
				break;
			case PLUS:
				stkptr[-1]+=*stkptr;
				--stkptr;
				break;
			case MINUS:
				stkptr[-1]-=*stkptr;
				--stkptr;
				break;
				
			case PM:
			{
				pUint = (uint*)eq->mptr;
				uint whichPM = *pUint++;
				eq->mptr = (unsigned char*)pUint;
				
				assert( int(whichPM) < eq->pmSignature().size() );				
				bool plus = eq->pmSignature()[ whichPM ];
				
				if ( plus )
					stkptr[-1] += *stkptr;
				else
					stkptr[-1] -= *stkptr;
				
				--stkptr;
				break;
			}
			
			case MULT:
				stkptr[-1]*=*stkptr;
				--stkptr;
				break;
			case DIV:
				if(*stkptr==0.)
					*(--stkptr)=HUGE_VAL;
				else
				{
					stkptr[-1]/=*stkptr;
					--stkptr;
				}
				break;
			case POW:
				stkptr[-1]=pow(*(stkptr-1), *stkptr);
				--stkptr;
				break;
			case NEG:
				*stkptr=-*stkptr;
				break;
			case SQRT:
				*stkptr = sqrt(*stkptr);
				break;
				
			case FKT_1:
			{
				pScalarFunction=(double(**)(double))eq->mptr;
				*stkptr=(*pScalarFunction++)(*stkptr);
				eq->mptr=(unsigned char*)pScalarFunction;
				break;
			}
				
			case FKT_N:
			{
				pUint = (uint*)eq->mptr;
				int numArgs = *pUint++;
				eq->mptr = (unsigned char*)pUint;
						
				pVectorFunction = (double(**)(const Vector &))eq->mptr;
				
				Vector args( numArgs );
				for ( int i=0; i < int(numArgs); ++i )
					args[i] = *(stkptr-numArgs+1+i);
				
				stkptr[1-numArgs] = (*pVectorFunction++)(args);
				stkptr -= numArgs-1;
				
				eq->mptr = (unsigned char*)pVectorFunction;
				break;
			}
				
			case UFKT:
			{
				pUint=(uint*)eq->mptr;
				uint id = *pUint++;
				uint id_eq = *pUint++;
				
				// The number of arguments being passed to the function
				int numArgs = *pUint++;
				
				Vector args( numArgs );
				for ( int i=0; i<numArgs; ++i )
					args[i] = *(stkptr-numArgs+1+i);
				
				if ( m_ufkt.contains( id ) )
				{
					stkptr[1-numArgs] = fkt( m_ufkt[id]->eq[id_eq], args );
					stkptr -= numArgs-1;
				}
				
				eq->mptr=(unsigned char*)pUint;
				break;
			}
			
			case ENDE:
				return *stkptr;
		}
	}
}


int Parser::addFunction( const QString & str1, const QString & str2, Function::Type type )
{
	QString str[2] = { str1, str2 };
	
	Function * temp = new Function( type );
	
	for ( int i = 0; i < 2; ++i )
	{
		if ( str[i].isEmpty() || temp->eq.size() <= i )
			continue;
		
		if ( !temp->eq[i]->setFstr( str[i] ) )
		{
			kDebug() << "could not set fstr to \""<<str[i]<<"\"! error:"<<errorString()<<"\n";
			delete temp;
			return -1;
		}
	
		if ( fnameToID( temp->eq[i]->name() ) != -1 )
		{
			kDebug() << "function name reused.\n";
			m_error = FunctionNameReused;
			delete temp;
			return -1;
		}
	}
	
	temp->setId( getNewId() );
	m_ufkt[ temp->id() ] = temp;
	
	temp->plotAppearance( Function::Derivative0 ).color = temp->plotAppearance( Function::Derivative1 ).color = temp->plotAppearance( Function::Derivative2 ).color = temp->plotAppearance( Function::Integral ).color = XParser::self()->defaultColor( temp->id() );
	
	emit functionAdded( temp->id() );
	return temp->id(); //return the unique ID-number for the function
}


void Parser::initEquation( Equation * eq )
{
	if ( eq->parent() )
		eq->parent()->clearFunctionDependencies();
	
	m_error = ParseSuccess;
	m_errorPosition = -1;
	m_currentEquation = eq;
	mem = mptr = eq->mem;
	m_pmAt = 0;
	
	m_eval = eq->fstr();
	m_sanitizer.fixExpression( & m_eval );
	m_evalPos = m_eval.indexOf( '=' ) + 1;
	heir1();
	
	if ( !evalRemaining().isEmpty() && m_error == ParseSuccess )
		m_error = SyntaxError;
	
	if ( m_error != ParseSuccess )
		m_errorPosition = m_sanitizer.realPos( m_evalPos );
	
	addToken(ENDE);
}


bool Parser::removeFunction( Function * item )
{
	kDebug() << "Deleting id:" << item->id() << endl;
	
	foreach ( Function * it, m_ufkt )
	{
		if ( it == item )
			continue;
		
		if ( it->dependsOn( item ) )
		{
			KMessageBox::sorry(0,i18n("This function is depending on an other function"));
			return false;
		}
	}
	
	uint const id = item->id();
	m_ufkt.remove(id);
	delete item;
	
	emit functionRemoved( id );
	return true;
}

bool Parser::removeFunction(uint id)
{
	return m_ufkt.contains( id ) && removeFunction( m_ufkt[id] );
}

uint Parser::countFunctions()
{
	return m_ufkt.count();
}

void Parser::heir1()
{
	QChar c;
	heir2();
	
	if (m_error!=ParseSuccess)
		return;
	
	while(1)
	{
		if ( m_eval.length() <= m_evalPos )
			return;
		
		c = m_eval[m_evalPos];
		
		switch ( c.unicode() )
		{
			default:
				return;

			case 0xb1:
				if ( m_pmAt >= MAX_PM )
				{
					m_error = TooManyPM;
					return;
				}
				if ( m_currentEquation == m_ownEquation )
				{
					m_error = InvalidPM;
					return;
				}
				// no break
				
			case '+':
			case '-':
				++m_evalPos;
				addToken(PUSH);
				heir2();
				if(m_error!=ParseSuccess)
					return;
		}
		switch ( c.unicode() )
		{
			case '+':
				addToken(PLUS);
				break;
				
			case '-':
				addToken(MINUS);
				break;
				
			case 0xb1:
				addToken(PM);
				adduint( m_pmAt++ );
				break;
		}
	}
}


void Parser::heir2()
{
	if ( match("-") )
	{
		heir2();
		if(m_error!=ParseSuccess)
			return;
		addToken(NEG);
	}
	else if ( match( QChar(0x221a) ) ) // square root symbol
	{
		heir2();
		if(m_error!=ParseSuccess)
			return;
		addToken(SQRT);
	}
	else
		heir3();
}


void Parser::heir3()
{
	QChar c;
	heir4();
	if(m_error!=ParseSuccess)
		return;
	while(1)
	{
		if ( m_eval.length() <= m_evalPos )
			return;
		
		c = m_eval[m_evalPos];
		switch ( c.unicode() )
		{
			default:
				return;
			case '*':
			case '/':
				++m_evalPos;
				addToken(PUSH);
				heir4();
				if(m_error!=ParseSuccess)
					return ;
		}
		switch ( c.unicode() )
		{
			case '*':
				addToken(MULT);
				break;
			case '/':
				addToken(DIV);
				break;
		}
	}
}


void Parser::heir4()
{
	primary();
	if(m_error!=ParseSuccess)
		return;
	while(match("^"))
	{
		addToken(PUSH);
		primary();
		if(m_error!=ParseSuccess)
			return;
		addToken(POW);
	}
}


void Parser::primary()
{
	// Note that tryUserFunction has to go after tryVariable since differential
	// equations treat the function name as a variable
	
	tryFunction() || tryPredefinedFunction() || tryVariable() || tryUserFunction() || tryConstant() || tryNumber();
}


bool Parser::tryFunction()
{
	if ( !match("(") && !match(",") )
		return false;
	
	heir1();
	if ( !match(")") && !match(",") )
		m_error = MissingBracket;
	return true;
}
	
	
bool Parser::tryPredefinedFunction()
{
	for ( int i=0; i < ScalarCount; ++i )
	{
		if ( match(scalarFunctions[i].name) )
		{
			primary();
			addToken(FKT_1);
			addfptr(scalarFunctions[i].mfadr);
			return true;
		}
	}
	for ( int i=0; i < VectorCount; ++i )
	{
		if ( match(vectorFunctions[i].name) )
		{
			int argCount = readFunctionArguments();
			
			addToken(FKT_N);
			addfptr( vectorFunctions[i].mfadr, argCount );
			return true;
		}
	}
	
	return false;
}
	
	
bool Parser::tryVariable()
{
	QStringList variables = m_currentEquation->variables();
	
	// Sort the parameters by size, so that when identifying parameters, want to
	// match e.g. "ab" before "a"
	typedef QMultiMap <int, QString> ISMap;
	ISMap sorted;
	foreach ( QString var, variables )
		sorted.insert( -var.length(), var );
	
	foreach ( QString var, sorted )
	{
		if ( match( var ) )
		{
			addToken( VAR );
			adduint( variables.indexOf( var ) );
			return true;
		}
	}
	
	return false;
}
	

bool Parser::tryUserFunction()
{
	foreach ( Function * it, m_ufkt )
	{
		for ( int i = 0; i < it->eq.size(); ++i )
		{
			if ( !match( it->eq[i]->name()) )
				continue;
			
			if ( m_currentEquation->parent() )
				kDebug() << "it->id()="<<it->id()<<" m_currentEquation->fstr()="<<m_currentEquation->fstr()<<" m_currentEquation->parent()->id()="<<m_currentEquation->parent()->id()<<endl;
			
			if ( it->eq[i] == m_currentEquation || (m_currentEquation && it->dependsOn( m_currentEquation->parent() )) )
			{
				m_error = RecursiveFunctionCall;
				return true;
			}
			
			int argCount = readFunctionArguments();
			
			addToken(UFKT);
			addfptr( it->id(), i, argCount );
			if ( m_currentEquation->parent() )
				m_currentEquation->parent()->addFunctionDependency( it );
			
			return true;
		}
	}
	
	return false;
}


bool Parser::tryConstant()
{
	// Is it a user defined constant?
	if ( (m_eval.length()>m_evalPos) && constants()->have( m_eval[m_evalPos] ) )
	{
		QVector<Constant> constants = m_constants->all();
		foreach ( Constant c, constants )
		{
			QChar tmp = c.constant;
			if ( match( tmp ) )
			{
				addConstant(c.value);
				return true;
			}
		}
		
		assert( !"Could not find the constant!" ); // Should always be able to find the constant
		return true;
	}
	
	
	// Or a predefined constant?
#define CHECK_CONSTANT( a, b ) \
	if ( match(a) ) \
	{ \
		addConstant( b ); \
		return true; \
	}
	CHECK_CONSTANT( "pi", M_PI );
	CHECK_CONSTANT( QChar(960), M_PI );
	CHECK_CONSTANT( "e", M_E );
	CHECK_CONSTANT( QChar(0x221E), INFINITY );
	
	return false;
}


bool Parser::tryNumber()
{	
	QByteArray remaining = evalRemaining().toLatin1();
	char * lptr = remaining.data();
	char * p = 0;
	double const w = strtod(lptr, &p);
	if( lptr != p )
	{
		m_evalPos += p-lptr;
		addConstant(w);
		return true;
	}
	
	return false;
}


int Parser::readFunctionArguments()
{
	int argCount = 0;
	bool argLeft = true;
	do
	{
		argCount++;
		primary();
					
		argLeft = m_eval.at(m_evalPos-1) == ',';
		if (argLeft)
		{
			addToken(PUSH);
			m_evalPos--;
		}
	}
	while ( m_error == ParseSuccess && argLeft && !evalRemaining().isEmpty() );
	
	return argCount;
}


bool Parser::match( const QString & lit )
{
	if ( lit.isEmpty() )
		return false;
	
	if ( lit != evalRemaining().left( lit.length() ) )
		return false;
	
	m_evalPos += lit.length();
	return true;
}


void Parser::addToken( Token token )
{
	if ( mptr>=&mem[MEMSIZE-10] )
		m_error = MemoryOverflow;
	else
		*mptr++=token;
}


void Parser::addConstant(double x)
{
	addToken(KONST);
	
	double *pd=(double*)mptr;

	if(mptr>=&mem[MEMSIZE-10])
		m_error = MemoryOverflow;
	else
	{
		*pd++=x;
		mptr=(unsigned char*)pd;
	}
}


void Parser::adduint(uint x)
{
	uint *p=(uint*)mptr;

	if(mptr>=&mem[MEMSIZE-10])
		m_error = MemoryOverflow;
	else
	{
		*p++=x;
		mptr=(unsigned char*)p;
	}
}


void Parser::addfptr(double(*fadr)(double))
{
	double (**pf)(double)=(double(**)(double))mptr;
	
	if( mptr>=&mem[MEMSIZE-10] )
		m_error = MemoryOverflow;
	else
	{
		*pf++=fadr;
		mptr=(unsigned char*)pf;
	}
}


void Parser::addfptr( double(*fadr)(const Vector & ), int argCount )
{
	uint *p = (uint*)mptr;
	*p++ = argCount;
	mptr = (unsigned char*)p;
	
	double (**pf)(const Vector &) = (double(**)(const Vector &))mptr;
	
	if( mptr>=&mem[MEMSIZE-10] )
		m_error = MemoryOverflow;
	else
	{
		*pf++=fadr;
		mptr=(unsigned char*)pf;
	}
}


void Parser::addfptr( uint id, uint eq_id, uint args )
{
	uint *p=(uint*)mptr;
	
	if(mptr>=&mem[MEMSIZE-10])
		m_error=MemoryOverflow;
	else
	{
		*p++=id;
		*p++=eq_id;
		*p++=args;
		mptr=(unsigned char*)p;
	}
}


int Parser::fnameToID(const QString &name)
{
	foreach ( Function * it, m_ufkt )
	{
		foreach ( Equation * eq, it->eq )
		{
			if ( name == eq->name() )
				return it->id();
		}
	}
	return -1;     // Name not found
}


QString Parser::errorString() const
{
	switch(m_error)
	{
		case ParseSuccess:
			return QString();
			
		case SyntaxError:
			return i18n("Syntax error");
			
		case MissingBracket:
			return i18n("Missing parenthesis");
			
		case UnknownFunction:
			return i18n("Function name unknown");
			
		case MemoryOverflow:
			return i18n("Token-memory overflow");
			
		case StackOverflow:
			return i18n("Stack overflow");
			
		case FunctionNameReused:
			return i18n("Name of function is not free");
			
		case RecursiveFunctionCall:
			return i18n("recursive function not allowed");
			
		case EmptyFunction:
			return i18n("Empty function");
			
		case CapitalInFunctionName:
			return i18n("The function name is not allowed to contain capital letters");
			
		case NoSuchFunction:
			return i18n("Function could not be found");
			
		case UserDefinedConstantInExpression:
			return i18n("The expression must not contain user-defined constants");
			
		case ZeroOrder:
			return i18n("The differential equation must be at least first-order");
			
		case TooManyPM:
			return i18n("Too many plus-minus symbols");
			
		case InvalidPM:
			return i18n("Invalid plus-minus symbol (expression must be constant)");
			
		case TooManyArguments:
			return i18n("The function has too many arguments");
	}
	
	return QString();
}


Parser::Error Parser::parserError(bool showMessageBox)
{
	if (!showMessageBox)
		return m_error;
	
	QString message( errorString() );
	if ( !message.isEmpty() )
		KMessageBox::sorry(0, message, "KmPlot");
	return m_error;
}


QString Parser::evalRemaining() const
{
	QString current( m_eval );
	return current.right( qMax( 0, current.length() - m_evalPos ) );
}


Function * Parser::functionWithID( int id ) const
{
	return m_ufkt.contains( id ) ? m_ufkt[id] : 0;
}


// static
QString Parser::number( double value )
{
	QString str = QString::number( value, 'g', 6 );
	str.replace( 'e', "*10^" );
	return str;
}
//END class Parser


//BEGIN predefined mathematical functions
double ln(double x) {
	return log(x);
}
double llog(double x) {
	return log10(x);
}
double sqr(double x) {
	return x*x;
}
double arsinh(double x) {
	return log(x+sqrt(x*x+1));
}
double arcosh(double x) {
	return log(x+sqrt(x*x-1));
}
double artanh(double x) {
	return log((1+x)/(1-x))/2;
}
double sec(double x) {
	return (1 / cos(x*Parser::radiansPerAngleUnit()));
}
double cosec(double x) {
	return (1 / sin(x*Parser::radiansPerAngleUnit()));
}
double cot(double x) {
	return (1 / tan(x*Parser::radiansPerAngleUnit()));
}
double arcsec(double x) {
	return acos(1/x)* 1/Parser::radiansPerAngleUnit();
}
double arccosec(double x) {
	return asin(1/x)* 1/Parser::radiansPerAngleUnit();
}
double arccot(double x)
{
	return atan(1/x)* 1/Parser::radiansPerAngleUnit();
}
double sech(double x) {
	return (1 / cosh(x));
}
double cosech(double x) {
	return (1 / sinh(x));
}
double coth(double x) {
	return (1 / tanh(x));
}
double arsech(double x) {
	return arcosh(1/x);
}
double arcosech(double x) {
	return arsinh(1/x);
}
double arcoth(double x) {
	return artanh(1/x);
}
double lcos(double x) {
	return cos(x*Parser::radiansPerAngleUnit());
}
double lsin(double x) {
	return sin(x*Parser::radiansPerAngleUnit());
}
double ltan(double x) {
	return tan(x*Parser::radiansPerAngleUnit());
}
double lcosh(double x) {
	return cosh(x);
}
double lsinh(double x) {
	return sinh(x);
}
double ltanh(double x) {
	return tanh(x);
}
double arccos(double x) {
	return acos(x) * 1/Parser::radiansPerAngleUnit();
}
double arcsin(double x) {
	return asin(x)* 1/Parser::radiansPerAngleUnit();
}
double arctan(double x) {
	return atan(x)* 1/Parser::radiansPerAngleUnit();
}
double legendre0( double ) {
	return 1.0;
}
double legendre1( double x ) {
	return x;
}
double legendre2( double x ) {
	return (3*x*x-1)/2;
}
double legendre3( double x ) {
	return (5*x*x*x - 3*x)/2;
}
double legendre4( double x ) {
	return (35*x*x*x*x - 30*x*x +3)/8;
}
double legendre5( double x ) {
	return (63*x*x*x*x*x - 70*x*x*x + 15*x)/8;
}
double legendre6( double x ) {
	return (231*x*x*x*x*x*x - 315*x*x*x*x + 105*x*x - 5)/16;
}

double sign(double x)
{
	if(x<0.)
		return -1.;
	else if(x>0.)
		return 1.;
	return 0.;
}

double heaviside( double x )
{
	if ( x < 0.0 )
		return 0.0;
	else if ( x > 0.0 )
		return 1.0;
	else
		return 0.5;
}

double min( const Vector & args )
{
	double best = HUGE_VAL;
	for ( int i=0; i < args.size(); ++i )
	{
		if ( args[i] < best )
			best = args[i];
	}
	
	return best;
}

double max( const Vector & args )
{
	double best = -HUGE_VAL;
	for ( int i=0; i < args.size(); ++i )
	{
		if ( args[i] > best )
			best = args[i];
	}
	
	return best;
}


double mod( const Vector & args )
{
	double squared = 0;
	for ( int i=0; i < args.size(); ++i )
		squared += args[i]*args[i];
	
	return std::sqrt( squared );
}
//END predefined mathematical functions



//BEGIN class Constants
Constants::Constants( Parser * parser )
{
	m_parser = parser;
}


QVector< Constant >::iterator Constants::find( QChar name )
{
	QVector<Constant>::Iterator it;
	for ( it = m_constants.begin(); it != m_constants.end(); ++it )
	{
		if ( it->constant == name )
			break;
	}
	return it;
}


bool Constants::have( QChar name ) const
{
	for ( QVector<Constant>::ConstIterator it = m_constants.begin(); it != m_constants.end(); ++it )
	{
		if ( it->constant == name )
			return true;
	}
	return false;
}


void Constants::remove( QChar name )
{
	QVector<Constant>::iterator c = find( name );
	if ( c != m_constants.end() )
		m_constants.erase( c );
}


void Constants::add( Constant c )
{
	remove( c.constant );
	m_constants.append( c );
}


bool Constants::isValidName( QChar name )
{
	// special cases: disallow heaviside step function, pi symbol
	if ( name == 'H' || name == QChar(960) )
		return false;
	
	switch ( name.category() )
	{
		case QChar::Letter_Uppercase:
			return true;
			
		case QChar::Letter_Lowercase:
			// don't allow lower case letters of the Roman alphabet
			return ( (name.unicode() < 'a') || (name.unicode() > 'z') );
			
		default:
			return false;
	}
}


QChar Constants::generateUniqueName()
{
	for ( char c = 'A'; c <= 'Z'; ++c )
	{
		if ( !have( c ) )
			return c;
	}
	
	kWarning() << k_funcinfo << "Could not find a unique constant.\n";
	return 'C';
}


void Constants::load()
{
	KSimpleConfig conf ("kcalcrc");
	conf.setGroup("UserConstants");
	QString tmp;
	
	for( int i=0; ;i++)
	{
		tmp.setNum(i);
		QString tmp_constant = conf.readEntry("nameConstant"+tmp, QString(" "));
		QString tmp_value = conf.readEntry("valueConstant"+tmp, QString(" "));
		
		if ( tmp_constant == " " )
			return;
		
		if ( tmp_constant.isEmpty() )
			continue;
			
		double value = m_parser->eval(tmp_value);
		if ( m_parser->parserError(false) )
		{
			kWarning() << k_funcinfo << "Couldn't parse the value " << tmp_value << endl;
			continue;
		}
		
		QChar constant = tmp_constant[0].toUpper();
		
		if ( !isValidName( constant ) || have( constant ) )
			constant = generateUniqueName();
		
		add( Constant(constant, value) );
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
	
	int i = 0;
	foreach ( Constant c, m_constants )
	{
		tmp.setNum(i);
		conf.writeEntry("nameConstant"+tmp, QString( c.constant ) ) ;
		conf.writeEntry("valueConstant"+tmp, c.value);
		
		i++;
	}
}
//END class Constants


//BEGIN class ExpressionSanitizer
ExpressionSanitizer::ExpressionSanitizer( Parser * parser )
	: m_parser( parser )
{
	m_str = 0l;
	m_decimalSymbol = KGlobal::locale()->decimalSymbol();
}


void ExpressionSanitizer::fixExpression( QString * str )
{
	m_str = str;
	
	m_map.resize( m_str->length() );
	for ( int i = 0; i < m_str->length(); ++i )
		m_map[i] = i;
	
	// hack for implicit functions: change e.g. "y = x + 2" to "y - (x+2)" so
	// that they can be evaluated via equality with zero.
	if ( str->count( '=' ) > 1 )
	{
		int equalsPos = str->lastIndexOf( '=' );
		replace( equalsPos, 1, "-(" );
		append( ')' );
	}
	
	stripWhiteSpace();
	
	// make sure all minus-like signs (including the actual unicode minus sign)
	// are represented by a dash (unicode 0x002d)
	QChar dashes[6] = { 0x2012, 0x2013, 0x2014, 0x2015, 0x2053, 0x2212 };
	for ( unsigned i = 0; i < 6; ++i )
		replace( dashes[i], '-' );
	
	// replace the proper unicode divide sign by the forward-slash
	replace( QChar( 0xf7 ), '/' );
	replace( QChar( 0x2215 ), '/' );
	
	// replace the unicode middle-dot for multiplication by the star symbol
	replace( QChar( 0xd7 ), '*' );
	replace( QChar( 0x2219 ), '*' );
	
	// minus-plus symbol to plus-minus symbol
	replace( QChar( 0x2213 ), QChar( 0xb1 ) );
	
	// various power symbols
	replace( QChar(0x00B2), "^2" );
	replace( QChar(0x00B3), "^3" );
	replace( QChar(0x2070), "^0" );
	replace( QChar(0x2074), "^4" );
	replace( QChar(0x2075), "^5" );
	replace( QChar(0x2076), "^6" );
	replace( QChar(0x2077), "^7" );
	replace( QChar(0x2078), "^8" );
	replace( QChar(0x2079), "^9" );
	
	// fractions
	replace( QChar(0x00BC), "(1/4)" );
	replace( QChar(0x00BD), "(1/2)" );
	replace( QChar(0x00BE), "(3/4)" );
	replace( QChar(0x2153), "(1/3)" );
	replace( QChar(0x2154), "(2/3)" );
	replace( QChar(0x2155), "(1/5)" );
	replace( QChar(0x2156), "(2/5)" );
	replace( QChar(0x2157), "(3/5)" );
	replace( QChar(0x2158), "(4/5)" );
	replace( QChar(0x2159), "(1/6)" );
	replace( QChar(0x215a), "(5/6)" );
	replace( QChar(0x215b), "(1/8)" );
	replace( QChar(0x215c), "(3/8)" );
	replace( QChar(0x215d), "(5/8)" );
	replace( QChar(0x215e), "(7/8)" );
	
	// replace e.g. |x+2| with abs(x+2)
	str->replace( QChar(0x2223), '|' ); // 0x2223 is the unicode math symbol for abs
	while ( true )
	{
		int pos1 = str->indexOf( '|' );
		int pos2 = str->indexOf( '|', pos1+1 );
		if ( pos1 == -1 || pos2 == -1 )
			break;
		
		replace( pos2, 1, ")" );
		replace( pos1, 1, "abs(" );
	}
	
	str->replace(m_decimalSymbol, "."); //replace the locale decimal symbol with a '.'
	
	//insert '*' when it is needed
	QChar ch;
	bool function = false;
	QStringList predefinedFunctions = XParser::self()->predefinedFunctions();
	
	for(int i=1; i+1 <  str->length();i++)
	{
		ch = str->at(i);
		
		bool chIsFunctionLetter = false;
		chIsFunctionLetter |= ch.category()==QChar::Letter_Lowercase;
		chIsFunctionLetter |= (ch == 'H');
		chIsFunctionLetter |= (ch == 'P') && (str->at(i+1) == QChar('_'));
		chIsFunctionLetter |= (ch == '_' );
		chIsFunctionLetter |= (ch.isNumber() && (str->at(i-1) == QChar('_')));
		
		if ( str->at(i+1)=='(' && chIsFunctionLetter )
		{
			// Work backwards to build up the full function name
			QString str_function(ch);
			int n=i-1;
			while (n>0 && ((str->at(n).category() == QChar::Letter_Lowercase) || (str->at(n) == QChar('_')) || (str->at(n) == QChar('P')) && (str->at(n+1) == QChar('_'))) )
			{
				str_function.prepend(str->at(n));
				--n;
			}
			
			if ( predefinedFunctions.contains( str_function ) )
				function = true;
				
			if ( !function )
			{
				// Not a predefined function, so search through the user defined functions (e.g. f(x), etc)
				// to see if it is one of those
				foreach ( Function * it, m_parser->m_ufkt )
				{
					for ( int j=i; j>0 && (str->at(j).isLetter() || str->at(j).isNumber() ) ; --j)
					{
						foreach ( Equation * eq, it->eq )
						{
							if ( eq->name() == str->mid(j,i-j+1) )
								function = true;
						}
					}
				}
			}
		}
		else  if (function)
			function = false;
		
		bool chIsNumeric = ((ch.isNumber() && (str->at(i-1) != QChar('_'))) || m_parser->m_constants->have( ch ));
				
		if ( chIsNumeric && ( str->at(i-1).isLetter() || str->at(i-1) == ')' ) || (ch.isLetter() && str->at(i-1)==')') )
		{
			insert(i,'*');
// 			kDebug() << "inserted * before\n";
		}
		else if( (chIsNumeric || ch == ')') && ( str->at(i+1).isLetter() || str->at(i+1) == '(' ) || (ch.isLetter() && str->at(i+1)=='(' && !function ) )
		{
			insert(i+1,'*');
//  			kDebug() << "inserted * after, function="<<function<<" ch="<<ch<<"\n";
			i++;
		}
	}
// 	kDebug() << "str:" << *str << endl;
}


void ExpressionSanitizer::stripWhiteSpace()
{
	int i = 0;
	
	while ( i < m_str->length() )
	{
		if ( m_str->at(i).isSpace() )
		{
			m_str->remove( i, 1 );
			m_map.remove( i, 1 );
		}
		else
			i++;
	}
}


void ExpressionSanitizer::remove( const QString & str )
{
	int at = 0;
	
	do
	{
		at = m_str->indexOf( str, at );
		if ( at != -1 )
		{
			m_map.remove( at, str.length() );
			m_str->remove( at, str.length() );
		}
	}
	while ( at != -1 );
}


void ExpressionSanitizer::remove( const QChar & str )
{
	remove( QString(str) );
}


void ExpressionSanitizer::replace( QChar before, QChar after )
{
	m_str->replace( before, after );
}


void ExpressionSanitizer::replace( QChar before, const QString & after )
{
	if ( after.isEmpty() )
	{
		remove( before );
		return;
	}
	
	int at = 0;
	
	do
	{
		at = m_str->indexOf( before, at );
		if ( at != -1 )
		{
			int to = m_map[ at ];
			for ( int i = at + 1; i < at + after.length(); ++i )
				m_map.insert( i, to );
			
			m_str->replace( at, 1, after );
			at += after.length() - 1;
		}
	}
	while ( at != -1 );
}


void ExpressionSanitizer::replace( int pos, int len, const QString & after )
{
	int before = m_map[pos];
	m_map.remove( pos, len );
	m_map.insert( pos, after.length(), before );
	m_str->replace( pos, len, after );
}


void ExpressionSanitizer::insert( int i, QChar ch )
{
	m_map.insert( i, m_map[i] );
	m_str->insert( i, ch );
}


void ExpressionSanitizer::append( QChar str )
{
	m_map.insert( m_map.size(), m_map[ m_map.size() - 1 ] );
	m_str->append( str );
}


int ExpressionSanitizer::realPos( int evalPos )
{
	if ( m_map.isEmpty() || (evalPos < 0) )
		return -1;
	
	if ( evalPos >= m_map.size() )
	{
// 		kWarning() << k_funcinfo << "evalPos="<<evalPos<<" is out of range.\n";
// 		return m_map[ m_map.size() - 1 ];
		return -1;
	}
	
	return m_map[evalPos];
}


void ExpressionSanitizer::displayMap( )
{
	QString out('\n');
	
	for ( int i = 0; i < m_map.size(); ++i )
		out += QString("%1").arg( m_map[i], 3 );
	out += '\n';
	
	for ( int i = 0; i < m_str->length(); ++i )
		out += "  " + (*m_str)[i];
	out += '\n';
	
	kDebug() << out;
}
//END class ExpressionSanitizer


#include "parser.moc"
