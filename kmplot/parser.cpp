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
#include <QDebug>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <kconfig.h>

#include <QList>

// standard c(++) includes
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <cmath>
#include <limits>
#include <locale.h>

double Parser::m_radiansPerAngleUnit = 0;

/**
 * List of predefined functions.
 * \note Some function names include other function names (e.g. "sinh" has the
 * string "sin" in it). The Parser will stop once it has found a matching
 * function name, so such functions must be in order of longest first.
 */
ScalarFunction Parser::scalarFunctions[ ScalarCount ]=
{
	// Hyperbolic trig
	{"sinh", QString(), sinh},	 				// Sinus hyperbolicus
	{"cosh", QString(), cosh}, 					// Cosinus hyperbolicus
	{"tanh", QString(), tanh},					// Tangens hyperbolicus
	{"arcsinh", "arsinh", asinh},		// Area-sinus hyperbolicus = inverse of sinh
	{"arccosh", "arcosh", acosh},		// Area-cosinus hyperbolicus = inverse of cosh
	{"arctanh", "artanh", atanh}, 		// Area-tangens hyperbolicus = inverse of tanh
	
	// Reciprocal-hyperbolic
	{"cosech", QString(), cosech},				// Co-Secans hyperbolicus
	{"sech", QString(), sech},					// Secans hyperbolicus
	{"coth", QString(), coth},					// Co-Tangens hyperbolicus
	{"arccosech", "arcosech", arcosech},// Area-co-secans hyperbolicus = inverse of cosech
	{"arcsech", "arsech", arsech},		// Area-secans hyperbolicus = invers of sech
	{"arccoth", "arcoth", arcoth},		// Area-co-tangens hyperbolicus = inverse of coth
	
	// Reciprocal-trig
	{"cosec", QString(), lcosec},				// Co-Secans = 1/sin
	{"sec", QString(), lsec},					// Secans = 1/cos
	{"cot", QString(), lcot},					// Co-Tangens = 1/tan
	{"arccosec", "arcosech", larccosec},// Arcus co-secans = inverse of cosec
	{"arcsec", "arsec", larcsec},		// Arcus secans = inverse of sec
	{"arccot", "arcot", larccot},		// Arcus co-tangens = inverse of cotan
	
	// Trigonometric functions
	{"sin", QString(), lsin}, 					// Sinus
	{"cos", QString(), lcos}, 					// Cosinus
	{"tan", QString(), ltan}, 					// Tangens
	{"arcsin", QString(), larcsin}, 			// Arcus sinus = inverse of sin
	{"arccos", QString(), larccos}, 			// Arcus cosinus = inverse of cos
	{"arctan", QString(), larctan},				// Arcus tangens = inverse of tan
	
	// Other
	{"sqrt", QString(), sqrt},					// Square root
	{"sqr", QString(), sqr}, 					// Square
	{"sign", QString(), sign},					// Signum
	{"H", QString(), heaviside},				// Heaviside step function
	{"log", QString(), log10},					// Logarithm base 10
	{"ln", QString(), log}, 					// Logarithm base e
	{"exp", QString(), exp}, 					// Exponential function base e
	{"abs", QString(), fabs},					// Absolute value
	{"floor", QString(), floor},				// round down to nearest integer
	{"ceil", QString(), ceil},					// round up to nearest integer
	{"round", QString(), round},				// round to nearest integer
	{"gamma", QString(), tgamma},			// gamma function
	{"lgamma", QString(), lgamma},			// log-gamma function
	{"factorial", QString(), factorial},		// factorial
	{"erfc", QString(), lerfc},			// error function
	{"erf", QString(), lerf},			// complementary error function
	
	// legendre
	{"P_0", QString(), legendre0},				// lengedre polynomial (n=0)
	{"P_1", QString(), legendre1},				// lengedre polynomial (n=1)
	{"P_2", QString(), legendre2},				// lengedre polynomial (n=2)
	{"P_3", QString(), legendre3},				// lengedre polynomial (n=3)
	{"P_4", QString(), legendre4},				// lengedre polynomial (n=4)
	{"P_5", QString(), legendre5},				// lengedre polynomial (n=5)
	{"P_6", QString(), legendre6},				// lengedre polynomial (n=6)
};

VectorFunction Parser::vectorFunctions[ VectorCount ]=
{
	{"min", min},						// minimum of a set of reals
	{"max", max},						// maximum of a set of reals
	{"mod", mod},						// l2 modulus of a set of reals
};


/**
 * Order by longest string first, useful in parsing since we want to at each point
 * match the longest string first, so e.g. "sinh(x)" shouldn't be read as "sin(h) * x"
 */
class LengthOrderedString : public QString
{
	public:
		LengthOrderedString() {}
		LengthOrderedString( const QString & s ) : QString(s) {}
		
		bool operator <( const LengthOrderedString & other ) const
		{
			return (length() > other.length()) ||
					((length() == other.length()) && (static_cast<const QString &>(*this) < static_cast<const QString &>(other)));
		}
};



//BEGIN class Parser
Parser::Parser()
	: m_sanitizer( this )
{
	m_evalPos = 0;
	m_nextFunctionID = 0;
	m_stack = new double [STACKSIZE];
	stkptr = m_stack;
	m_constants = new Constants;
	
	m_error = 0;
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


QStringList Parser::predefinedFunctions( bool includeAliases ) const
{
	QStringList names;
	
	for ( int func = 0; func < ScalarCount; ++func )
	{
		names << scalarFunctions[func].name1;
		if ( includeAliases && !scalarFunctions[func].name2.isEmpty() )
			names << scalarFunctions[func].name2;
	}
			
	for ( int func = 0; func < VectorCount; ++func )
		names << vectorFunctions[func].name;
	
	return names;
}


QStringList Parser::userFunctions( ) const
{
	QStringList names;
	
	foreach ( Function * f, m_ufkt )
	{
		foreach ( Equation * eq, f->eq )
		{
			if ( !eq->name().isEmpty() )
				names << eq->name();
		}
	}
	
	names.sort();
	return names;
}


void Parser::reparseAllFunctions()
{
    foreach ( Function * f, m_ufkt )
    {
        foreach ( Equation * eq, f->eq )
            initEquation( eq );
    }
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

double Parser::eval( const QString & str, Error * error, int * errorPosition )
{
	Error t1;
	if ( ! error )
		error = & t1;
	int t2;
	if ( ! errorPosition )
		errorPosition = & t2;
	
	
	if ( !m_ownEquation )
		m_ownEquation = new Equation( Equation::Constant, 0 );
	
	QString fName = XParser::self()->findFunctionName( "f", -1 );
	
	QString eq = QString( "%1=%2" ).arg( fName ).arg( str );
	if ( !m_ownEquation->setFstr( eq, (int*)error, errorPosition ) )
	{
		if ( errorPosition )
			*errorPosition -= fName.length()+1;
		return 0;
	}
	
	return fkt( m_ownEquation, Vector() );
}


double Parser::fkt(uint id, int eq, double x )
{
	if ( !m_ufkt.contains( id ) || m_ufkt[id]->eq.size() <= eq )
	{
		*m_error = NoSuchFunction;
		return 0;
	}
	
	return fkt( m_ufkt[id]->eq[eq], x );
}


double Parser::fkt( Equation * eq, double x )
{
	Function * function = eq->parent();
	Q_ASSERT_X( function->type() != Function::Differential, "Parser::fkt", "Do not use this function directly! Instead, call XParser::differential" );
	
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
			return 0;
	}
	
	qWarning() << "Unknown function type!\n";
	return 0;
}


double Parser::fkt( Equation * eq, const Vector & x )
{
	if ( eq->mem.isEmpty() )
		return 0;
	
	// Consistency check: Make sure that we leave the stkptr at the same place
	// that we started it
	double * stkInitial = stkptr;
	
	double *pDouble;
	double (**pScalarFunction)(double);
	double (**pVectorFunction)(const Vector &);
	uint *pUint;
	eq->mptr = eq->mem.data();
	
	// Start with zero in our stackpointer
	// 
	*stkptr = 0;
	
	while(1)
	{
// 		qDebug() << "*eq->mptr: "<<int(*eq->mptr);
		
		switch(*eq->mptr++)
		{
			case KONST:
			{
				pDouble=(double*)eq->mptr;
				*stkptr=*pDouble++;
				eq->mptr=(char*)pDouble;
				break;
			}
				
			case VAR:
			{
				pUint = (uint*)eq->mptr;
				uint var = *pUint++;
				if ( int(var) >= x.size() )
				{
					// Assume variable has value zero
					*stkptr = 0;
				}
				else
					*stkptr = x[var];
				eq->mptr = (char*)pUint;
				break;
			}
				
			case PUSH:
			{
				++stkptr;
				break;
			}
			
			case PLUS:
			{
				stkptr[-1]+=*stkptr;
				--stkptr;
				break;
			}
			
			case MINUS:
			{
				stkptr[-1]-=*stkptr;
				--stkptr;
				break;
			}
			
			case GT:
			{
				stkptr[-1] = (*(stkptr-1) > *stkptr) ? 1 : 0;
				stkptr--;
				break;
			}
			
			case GE:
			{
				stkptr[-1] = (*(stkptr-1) >= *stkptr) ? 1 : 0;
				stkptr--;
				break;
			}
			
			case LT:
			{
				stkptr[-1] = (*(stkptr-1) < *stkptr) ? 1 : 0;
				stkptr--;
				break;
			}
			
			case LE:
			{
				stkptr[-1] = (*(stkptr-1) <= *stkptr) ? 1 : 0;
				stkptr--;
				break;
			}
				
			case PM:
			{
				pUint = (uint*)eq->mptr;
				uint whichPM = *pUint++;
				eq->mptr = (char*)pUint;
				
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
			{
				stkptr[-1]*=*stkptr;
				--stkptr;
				break;
			}
			
			case DIV:
			{
				if(*stkptr==0.)
					*(--stkptr)=HUGE_VAL;
				else
				{
					stkptr[-1]/=*stkptr;
					--stkptr;
				}
				break;
			}
			
			case POW:
			{
				stkptr[-1]=pow(*(stkptr-1), *stkptr);
				--stkptr;
				break;
			}
			
			case NEG:
			{
				*stkptr=-*stkptr;
				break;
			}
			
			case SQRT:
			{
				*stkptr = sqrt(*stkptr);
				break;
			}
			
			case FACT:
			{
				*stkptr = factorial(*stkptr);
				break;
			}
				
			case FKT_1:
			{
				pScalarFunction=(double(**)(double))eq->mptr;
				*stkptr=(*pScalarFunction++)(*stkptr);
				eq->mptr=(char*)pScalarFunction;
				break;
			}
				
			case FKT_N:
			{
				pUint = (uint*)eq->mptr;
				int numArgs = *pUint++;
				
				eq->mptr = (char*)pUint;
						
				pVectorFunction = (double(**)(const Vector &))eq->mptr;
				
				Vector args( numArgs );
				for ( int i=0; i < int(numArgs); ++i )
					args[i] = *(stkptr-numArgs+1+i);
				
				if ( numArgs > 0 )
					stkptr += 1-numArgs;
				*stkptr = (*pVectorFunction++)(args);
				
				eq->mptr = (char*)pVectorFunction;
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
					if ( numArgs > 0 )
						stkptr += 1-numArgs;
					*stkptr = fkt( m_ufkt[id]->eq[id_eq], args );
				}
				
				eq->mptr=(char*)pUint;
				break;
			}
			
			case ENDE:
			{
				// If the stack isn't where we started at, then we've gone
				// up / down the wrong number of places - definitely a bug (and
				// will lead to crashes over time as memory rapidly runs out).
				assert( stkptr == stkInitial );
				return *stkptr;
			}
			case ERROR:
			{
				// something went wrong due to a incorrect formular or
				// missing const.
				qDebug()  << "Error in equation " << eq->fstr();
				// Adjust stack again. Stack is wrong, only if we have a PUSH token
				// just before the ERROR token, afaik.
				while( stkptr != stkInitial )
				{
					stkptr--;
				}
				return *stkptr;
			}
		}
	}
}


int Parser::addFunction( const QString & str1, const QString & str2, Function::Type type, bool force )
{
	QString str[2] = { str1, str2 };
	
	Function * temp = new Function( type );
	temp->setId( getNewId() );
	
	for ( int i = 0; i < 2; ++i )
	{
		if ( str[i].isEmpty() || temp->eq.size() <= i )
			continue;
		
		int error;
		if ( !temp->eq[i]->setFstr( str[i], & error, 0, force ) && !force )
		{
			qDebug() << "could not set fstr to \""<<str[i]<<"\"!  error:"<<errorString(Error(error))<<"\n";
			delete temp;
			return -1;
		}
	
		bool duplicate = (fnameToID( temp->eq[i]->name() ) != -1);
		if ( temp->eq[i]->looksLikeFunction() && duplicate && !force )
		{
			qDebug() << "function name reused.\n";
			*m_error = FunctionNameReused;
			delete temp;
			return -1;
		}
	}
	
	m_ufkt[ temp->id() ] = temp;
	
	temp->plotAppearance( Function::Derivative0 ).color = temp->plotAppearance( Function::Derivative1 ).color = temp->plotAppearance( Function::Derivative2 ).color = temp->plotAppearance( Function::Integral ).color = XParser::self()->defaultColor( temp->id() );
	
	emit functionAdded( temp->id() );
	return temp->id(); //return the unique ID-number for the function
}


void Parser::initEquation( Equation * eq, Error * error, int * errorPosition )
{
	Error t1;
	if ( ! error )
		error = & t1;
	int t2;
	if ( ! errorPosition )
		errorPosition = & t2;
	
	
	if ( eq->parent() )
		eq->parent()->clearFunctionDependencies();
	
	m_error = error;
	
	*m_error = ParseSuccess;
	*errorPosition = -1;
	
	m_currentEquation = eq;
	mem = & eq->mem;
	mptr = mem->data();
	m_pmAt = 0;
	
	m_eval = eq->fstr();
	m_sanitizer.fixExpression( & m_eval );
	m_evalRemaining = m_eval;
	m_evalPos = m_eval.indexOf( '=' ) + 1;
	heir0();
	
	if ( !evalRemaining().isEmpty() && *m_error == ParseSuccess )
		*m_error = SyntaxError;
	
	if ( *m_error != ParseSuccess )
	{
		*errorPosition = m_sanitizer.realPos( m_evalPos );
		qDebug() << "add an error token for " << eq->fstr();
		// add an error token and let the user decide
		addToken( ERROR );
	}
	addToken( ENDE );
}


bool Parser::removeFunction( Function * item )
{
	// Build up a list of functions that need to be removed is this function is removed
	QList<Function *> toRemove;
	QStringList otherRemoveNames;
	QList<Function *> newFunctions; // Added since the last iteration
	
	toRemove << item;
	newFunctions << item;
	
	while ( ! newFunctions.isEmpty() )
	{
		QList<Function *> currentFunctions = newFunctions;
		newFunctions.clear();
		
		foreach ( Function *f, currentFunctions )
		{
			foreach ( Function *other, m_ufkt )
			{
				if ( (other==f) || toRemove.contains(other) )
					continue;
				
				if ( other->dependsOn( f ) )
				{
					toRemove << other;
					otherRemoveNames << other->name();
					newFunctions << other;
				}
			}
		}
	}
	
	if ( toRemove.size() > 1 )
	{
		KGuiItem buttonContinue = KStandardGuiItem::cont();
		buttonContinue.setText( i18n("Remove all") );
		
		int answer = KMessageBox::warningContinueCancel( 0, i18n( "The function %1 is depended upon by the following functions: %2. These must be removed in addition.", item->name(), otherRemoveNames.join(", ") ), QString(), buttonContinue );
		
		if ( answer == KMessageBox::Cancel )
			return false;
	}
	
	foreach ( Function *f, toRemove )
	{
		uint id = f->id();
		m_ufkt.remove( id );
		delete f;
		emit functionRemoved( id );
	}
	
	return true;
}

bool Parser::removeFunction(uint id)
{
	return m_ufkt.contains( id ) && removeFunction( m_ufkt[id] );
}


void Parser::removeAllFunctions()
{
	while ( !m_ufkt.isEmpty() )
	{
		Function *f = *m_ufkt.begin();
		int id = f->id();
		m_ufkt.remove( id );
		delete f;
		emit functionRemoved( id );
	}
}


uint Parser::countFunctions()
{
	return m_ufkt.count();
}

void Parser::heir0()
{
	heir1();
	
	if (*m_error!=ParseSuccess)
		return;
	
	while(1)
	{
		if ( m_eval.length() <= m_evalPos )
			return;
		
		QChar c = m_eval[m_evalPos];
		
		switch ( c.unicode() )
		{
			default:
				return;
				
			case '<':
			case '>':
			case 0x2264: // less than or equal
			case 0x2265: // greater than or equal
				++m_evalPos;
				addToken(PUSH);
				heir1();
				if ( *m_error != ParseSuccess )
					return;
		}
		switch ( c.unicode() )
		{
			case '<':
				addToken(LT);
				break;
				
			case '>':
				addToken(GT);
				break;
				
			case 0x2264: // less than or equal
				addToken(LE);
				break;
				
			case 0x2265: // greater than or equal
				addToken(GE);
				break;
		}
	}
}

void Parser::heir1()
{
	heir2();
	
	if (*m_error!=ParseSuccess)
		return;
	
	while(1)
	{
		if ( m_eval.length() <= m_evalPos )
			return;
		
		QChar c = m_eval[m_evalPos];
		
		switch ( c.unicode() )
		{
			default:
				return;

			case 0xb1:
				if ( m_pmAt >= MAX_PM )
				{
					*m_error = TooManyPM;
					return;
				}
				if ( m_currentEquation == m_ownEquation )
				{
					*m_error = InvalidPM;
					return;
				}
				// no break
			case '+':
			case '-':
				++m_evalPos;
				addToken(PUSH);
				heir2();
				if(*m_error!=ParseSuccess)
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
	if ( match( SqrtSymbol ) ) // square root symbol
	{
		heir2();
		if(*m_error!=ParseSuccess)
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
	if(*m_error!=ParseSuccess)
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
				if(*m_error!=ParseSuccess)
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
    if (match("-")) {
        heir4();
        if (*m_error != ParseSuccess)
            return;
        addToken(NEG);
    } else if (match("+")) {
        heir4();
    } else {
        heir5();
    }
}


void Parser::heir5()
{
	primary();
	if(*m_error!=ParseSuccess)
		return;
	
	while ( true )
	{
		if ( match("^") )
		{
			addToken(PUSH);
			heir4();
			if(*m_error!=ParseSuccess)
				return;
			addToken(POW);
		}
		else if ( match("!") )
			addToken(FACT);
		else
			return;
	}
}


void Parser::primary()
{
	// Notes:
	// - tryUserFunction has to go after tryVariable since differential
	//   equations treat the function name as a variable
	// - tryConstant has to go before tryUserFunction. This solves a problem,
	//   when a function and a constant share the same name
	
	tryFunction() || tryPredefinedFunction() || tryVariable() || tryConstant() || tryUserFunction() || tryNumber();
}


bool Parser::tryFunction()
{
	if ( !match("(") && !match(",") )
		return false;
	
	heir0();
	if ( !match(")") && !match(",") )
		*m_error = MissingBracket;
	return true;
}
	
	
bool Parser::tryPredefinedFunction()
{
	for ( int i=0; i < ScalarCount; ++i )
	{
		if ( match(scalarFunctions[i].name1) || match(scalarFunctions[i].name2) )
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
	foreach ( const QString &var, variables )
		sorted.insert( -var.length(), var );
	
	foreach ( const QString &var, sorted )
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
			
			if ( it->eq[i] == m_currentEquation || (m_currentEquation && it->dependsOn( m_currentEquation->parent() )) )
			{
				*m_error = RecursiveFunctionCall;
				return true;
			}
			
			int argCount = readFunctionArguments();
			if ( argCount != it->eq[i]->variables().size() )
			{
				*m_error = IncorrectArgumentCount;
				return true;
			}
			
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
#define CHECK_CONSTANT( a, b ) \
	if ( match(a) ) \
	{ \
		addConstant( b ); \
		return true; \
	}
	
	ConstantList constants = m_constants->list( Constant::All );
	
	QMap<LengthOrderedString, Constant> orderedConstants;
	for ( ConstantList::iterator i = constants.begin(); i != constants.end(); ++i )
		orderedConstants[ i.key() ] = i.value();
	
	for ( QMap<LengthOrderedString, Constant>::iterator i = orderedConstants.begin(); i != orderedConstants.end(); ++i )
		CHECK_CONSTANT( i.key(), i.value().value.value() );
	
	// Or a predefined constant?
	CHECK_CONSTANT( "pi", M_PI );
	CHECK_CONSTANT( PiSymbol, M_PI );
	CHECK_CONSTANT( "e", M_E );
	CHECK_CONSTANT( InfinitySymbol, std::numeric_limits<double>::infinity());
	
	return false;
}


bool Parser::tryNumber()
{	
	QByteArray remaining = evalRemaining().toLatin1();
	char * lptr = remaining.data();
	char * p = 0;
	// we converted all to "C" format in fixExpression
	char *oldLocale = setlocale(LC_NUMERIC, "C");
	double const w = strtod(lptr, &p);
	setlocale(LC_NUMERIC, oldLocale);
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
	if ( !evalRemaining().startsWith( '(' ) )
		return 0;
	
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
	while ( *m_error == ParseSuccess && argLeft && !evalRemaining().isEmpty() );
	
	return argCount;
}


void Parser::growEqMem( int growth )
{
	int pos = mptr - mem->data();
	mem->resize( mem->size() + growth );
	mptr = mem->data() + pos;
}


void Parser::addToken( Token token )
{
	growEqMem( sizeof(Token) );
	*mptr++=token;
}


void Parser::addConstant(double x)
{
	addToken(KONST);
	
	growEqMem( sizeof(double) );
	double *pd=(double*)mptr;
	
	*pd++=x;
	mptr=(char*)pd;
}


void Parser::adduint(uint x)
{
	growEqMem( sizeof(uint) );
	uint *p=(uint*)mptr;
	*p++=x;
	mptr=(char*)p;
}


void Parser::addfptr( double(*fadr)(double) )
{
	typedef double(**sfPtr)(double);
	
	growEqMem( sizeof(sfPtr) );
// 	double (**pf)(double)=(double(**)(double))mptr;
	
	sfPtr pf = (sfPtr)mptr;
	*pf++=fadr;
	mptr=(char*)pf;
}


void Parser::addfptr( double(*fadr)(const Vector &), int argCount )
{
	typedef double(**vfPtr)(const Vector &);
	
	growEqMem( sizeof(uint) );
	uint *p = (uint*)mptr;
	*p++ = argCount;
	mptr = (char*)p;
	
	growEqMem( sizeof(vfPtr) );
	vfPtr pf = (vfPtr)mptr;
	*pf++=fadr;
	mptr=(char*)pf;
}


void Parser::addfptr( uint id, uint eq_id, uint args )
{
	growEqMem( 3*sizeof(uint) );
	
	uint *p=(uint*)mptr;
	*p++=id;
	*p++=eq_id;
	*p++=args;
	mptr=(char*)p;
}


int Parser::fnameToID(const QString &name)
{
	foreach ( Function * it, m_ufkt )
	{
		foreach ( Equation * eq, it->eq )
		{
			if ( eq->looksLikeFunction() && (name == eq->name()) )
				return it->id();
		}
	}
	return -1;     // Name not found
}


// static
QString Parser::errorString( Error error )
{
	switch ( error )
	{
		case ParseSuccess:
			return QString();
			
		case SyntaxError:
			return i18n("Syntax error");
			
		case MissingBracket:
			return i18n("Missing parenthesis");
			
		case StackOverflow:
			return i18n("Stack overflow");
			
		case FunctionNameReused:
			return i18n("Name of function is not free");
			
		case RecursiveFunctionCall:
			return i18n("recursive function not allowed");
			
		case EmptyFunction:
			return i18n("Empty function");
			
		case NoSuchFunction:
			return i18n("Function could not be found");
			
		case ZeroOrder:
			return i18n("The differential equation must be at least first-order");
			
		case TooManyPM:
			return i18n("Too many plus-minus symbols");
			
		case InvalidPM:
			return i18n("Invalid plus-minus symbol (expression must be constant)");
			
		case TooManyArguments:
			return i18n("The function has too many arguments");
			
		case IncorrectArgumentCount:
			return i18n("The function does not have the correct number of arguments");
	}
	
	return QString();
}


void Parser::displayErrorDialog( Error error )
{	
	QString message( errorString(error) );
	if ( !message.isEmpty() )
		KMessageBox::sorry(0, message, "KmPlot");
}


QString Parser::evalRemaining()
{
	/// note changing this code may need to change code in match() as well; similar
	int newLength = qMax( 0, m_eval.length() - m_evalPos );
	if ( newLength != m_evalRemaining.length() )
		m_evalRemaining = m_eval.right( newLength );
	return m_evalRemaining;
}


bool Parser::match( const QString & lit )
{
	if ( lit.isEmpty() )
		return false;
	
	/// note changing this code may need to change code in evalRemaining() as well; similar
	// Do we need to update m_evalRemaining ?
	int newLength = qMax( 0, m_eval.length() - m_evalPos );
	if ( newLength != m_evalRemaining.length() )
		evalRemaining();
	
	if ( !m_evalRemaining.startsWith( lit ) )
		return false;
	
	m_evalPos += lit.length();
	return true;
}


Function * Parser::functionWithID( int id ) const
{
	return m_ufkt.contains( id ) ? m_ufkt[id] : 0;
}


// static
QString Parser::number( double value )
{
	QString str = QString::number( value, 'g', 16 );
	str.replace( 'e', "*10^" );
	return str;
}
//END class Parser


//BEGIN predefined mathematical functions
double sqr(double x) {
	return x*x;
}
double lsec(double x) {
	return (1 / cos(x*Parser::radiansPerAngleUnit()));
}
double lcosec(double x) {
	return (1 / sin(x*Parser::radiansPerAngleUnit()));
}
double lcot(double x) {
	return (1 / tan(x*Parser::radiansPerAngleUnit()));
}
double larcsec(double x) {
	return acos(1/x) / Parser::radiansPerAngleUnit();
}
double larccosec(double x) {
	return asin(1/x) / Parser::radiansPerAngleUnit();
}
double larccot(double x) {
	return (M_PI/2 - atan(x)) / Parser::radiansPerAngleUnit();
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
	return acosh(1/x);
}
double arcosech(double x) {
	return asinh(1/x);
}
double arcoth(double x) {
	return atanh(1/x);
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
double larccos(double x) {
	return acos(x) / Parser::radiansPerAngleUnit();
}
double larcsin(double x) {
	return asin(x) / Parser::radiansPerAngleUnit();
}
double larctan(double x) {
	return atan(x) / Parser::radiansPerAngleUnit();
}
double factorial( double x ) {
	return tgamma(x+1);
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

double lerf(double x) {
	return erf(x);
}

double lerfc(double x) {
	return erfc(x);
}

//END predefined mathematical functions



//BEGIN class ExpressionSanitizer
enum StringType { ConstantString, NumberString, UnknownLetter, FunctionString, Other };


ExpressionSanitizer::ExpressionSanitizer( Parser * parser )
	: m_parser( parser )
{
	m_str = 0l;
	m_decimalSymbol = QLocale().decimalPoint();
}


void ExpressionSanitizer::fixExpression( QString * str )
{
	m_str = str;
	
	m_map.resize( m_str->length() );
	for ( int i = 0; i < m_str->length(); ++i )
		m_map[i] = i;
	
	// greater-equal, less-equal with proper symbols
	// note that this must go before the next code for implicit equations, since
	// it removes the spurious equals signs
	replace( ">=", GeSymbol );
	replace( "<=", LeSymbol );
	
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
	replace( QChar( 0x2213 ), PmSymbol );
	
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
	
	//BEGIN replace e.g. |x+2| with abs(x+2)
	str->replace( AbsSymbol, '|' );
	
	int maxDepth = str->count( '(' );
	QVector<bool> absAt( maxDepth+1 );
	for ( int i = 0; i < maxDepth+1; ++i )
		absAt[i] = false;
	
	int depth = 0;
	
	for ( int i = 0; i < str->length(); ++i )
	{
		if ( str->at(i) == '|' )
		{
			if ( absAt[depth] )
			{
				// Closing it
				replace( i, 1, ")" );
				absAt[depth] = false;
			}
			else
			{
				// Opening it
				replace( i, 1, "abs(" );
				i += 3;
				absAt[depth] = true;
			}
		}
		else if ( str->at(i) == '(' )
			depth++;
		else if ( str->at(i) == ')' )
		{
			depth--;
			if ( depth < 0 )
				depth = 0;
		}
	}
	//END replace e.g. |x+2| with abs(x+2)
	
	str->replace(m_decimalSymbol, "."); //replace the locale decimal symbol with a '.'
	
	//BEGIN build up strings
	QMap< LengthOrderedString, StringType > strings;
	
	QStringList predefinedFunctions = XParser::self()->predefinedFunctions( true );
	foreach ( const QString &f, predefinedFunctions )
		strings[f] = FunctionString;
	
	foreach ( Function * it, m_parser->m_ufkt )
	{
		foreach ( Equation * eq, it->eq )
			strings[eq->name()] = FunctionString;
	}
	
	QStringList constantNames = m_parser->constants()->names();
	foreach ( const QString &name, constantNames )
		strings[name] = ConstantString;
	strings[QString("pi")] = ConstantString;
	//END build up strings
	
	strings.remove( QString() );
	
	StringType prevType = Other;
	
	for(int i=1; i < str->length(); )
	{
		int incLength = 1;
		StringType currentType = Other;
		QChar ch = str->at(i);
		
		QString remaining = str->right( str->length() - i );
		
		QMap< LengthOrderedString, StringType >::const_iterator end = strings.constEnd();
		for ( QMap< LengthOrderedString, StringType >::const_iterator it = strings.constBegin(); it != end; ++it )
		{
			if ( !remaining.startsWith( it.key() ) )
				continue;
			
			currentType = it.value();
			incLength = it.key().length();
			break;
		}
		
		if ( currentType == Other )
		{
			if ( ch.isNumber() || (ch == '.') )
				currentType = NumberString;
		}
		
		if ( currentType == Other )
		{
			if ( ch.isLetter() )
				currentType = UnknownLetter; // probably a variable
		}
		
		if ( ((currentType == FunctionString) || (ch == '(') || (currentType == UnknownLetter) || (currentType == ConstantString)) &&
					 ((prevType == NumberString) || (prevType == ConstantString) || (prevType == UnknownLetter) || (str->at(i-1) == ')')) )
		{
			insert( i, '*' );
			incLength++;
		}
		
		prevType = currentType;
		i += incLength;
	}
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


void ExpressionSanitizer::replace( const QString & before, const QString & after )
{
	int index;
	while ( (index = m_str->indexOf(before)) > -1 )
		replace( index, before.length(), after );
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
// 		qWarning() << "evalPos="<<evalPos<<" is out of range.\n";
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
	
	qDebug() << out;
}
//END class ExpressionSanitizer
