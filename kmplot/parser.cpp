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

// standard c(++) includes
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//KDE includes
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>

// local includes
#include "parser.h"
#include "settings.h"
#include "xparser.h"
//Added by qt3to4:
#include <QList>

double Parser::m_radiansPerAngleUnit = 0;

/// List of predefined functions.
Parser::Mfkt Parser::mfkttab[ FANZ ]=
{
	{"tanh", ltanh},	// Tangens hyperbolicus
	{"tan", ltan}, 		// Tangens
	{"sqrt", sqrt},		// Square root
	{"sqr", sqr}, 		// Square
	{"sinh", lsinh}, 	// Sinus hyperbolicus
	{"sin", lsin}, 		// Sinus
	{"sign", sign},		// Signum
	{"H", heaviside},	// Heaviside step function
	{"sech", sech},		// Secans hyperbolicus
	{"sec", sec},		// Secans
	{"log", llog}, 	        // Logarithm base 10
	{"ln", ln}, 		// Logarithm base e
	{"exp", exp}, 		// Exponential function base e
	{"coth", coth},		// Co-Tangens hyperbolicus
	{"cot", cot},		// Co-Tangens = 1/tan
	{"cosh", lcosh}, 	// Cosinus hyperbolicus
	{"cosech", cosech},	// Co-Secans hyperbolicus
	{"cosec", cosec},	// Co-Secans
	{"cos", lcos}, 		// Cosinus
	{"artanh", artanh}, 	// Area-tangens hyperbolicus = inverse of tanh
	{"arsinh", arsinh}, 	// Area-sinus hyperbolicus = inverse of sinh
	{"arsech", arsech},		// Area-secans hyperbolicus = invers of sech
	{"arctanh", artanh},	// The same as artanh
	{"arcsinh", arsinh},	// The same as arsinh
	{"arccosh", arcosh},	// The same as arcosh
	{"arctan", arctan},		// Arcus tangens = inverse of tan
	{"arcsin", arcsin}, 	// Arcus sinus = inverse of sin
	{"arcsec", arcsec},		// Arcus secans = inverse of sec
	{"arcoth", arcoth},		// Area-co-tangens hyperbolicus = inverse of coth
	{"arcosh", arcosh}, 	// Area-cosinus hyperbolicus = inverse of cosh
	{"arcosech", arcosech},	// Area-co-secans hyperbolicus = inverse of cosech
	{"arccot", arccot},		// Arcus co-tangens = inverse of cotan
	{"arccosec", arccosec},	// Arcus co-secans = inverse of cosec
	{"arccos", arccos}, 	// Arcus cosinus = inverse of cos
	{"abs", fabs},			// Absolute value
	{"floor", floor},		// round down to nearest integer
	{"ceil", ceil},			// round up to nearest integer
	{"round", round},		// round to nearest integer
};



//BEGIN class Parser
Parser::Parser()
	: m_sanitizer( this )
{
	m_evalPos = 0;
	evalflg = 0;
	m_nextFunctionID = 0;
	m_constants = new Constants( this );
	
	m_ownEquation = new Equation( Equation::Cartesian, 0 );
	m_currentEquation = m_ownEquation;
}


Parser::~Parser()
{
	kDebug() << "Exiting......" << endl;
	foreach ( Function * function, m_ufkt )
		delete function;
	delete m_ownEquation;
	
	delete m_constants;
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

double Parser::eval( QString str, unsigned evalPosOffset, bool fixExpression )
{
// 	kDebug() << k_funcinfo << "str=\""<<str<<"\"\n";
	m_currentEquation = m_ownEquation;
	m_currentEquation->setFstr( str, true );
	
	stack=new double [STACKSIZE];
	stkptr=stack;
	evalflg=1;
	
	if ( fixExpression )
		m_sanitizer.fixExpression( & str, evalPosOffset );
// 	kDebug() << "##### str="<<str<<endl;
	
	int yIndex = str.indexOf( 'y', evalPosOffset );
	
	if ( yIndex != -1 )
	{
		kDebug() << k_funcinfo << "RecursiveFunctionCall! yIndex="<<yIndex<<endl;
		m_error = RecursiveFunctionCall;
		m_errorPosition = m_sanitizer.realPos( yIndex );
		delete []stack;
		return 0;
	}
	for ( int i = evalPosOffset; i < str.length(); i++ )
	{
		if ( constants()->isValidName( str[i] ) )
		{
			m_error = UserDefinedConstantInExpression;
			m_errorPosition = m_sanitizer.realPos( i );
			delete []stack;
			return 0;
		}
	}
	
	m_eval = str;
	m_evalPos = evalPosOffset;
	m_error = ParseSuccess;
	heir1();
	if( !evalRemaining().isEmpty() && m_error==ParseSuccess)
		m_error=SyntaxError;
	evalflg=0;
	double const erg=*stkptr;
	delete [] stack;
	if ( m_error == ParseSuccess )
	{
		m_errorPosition = -1;
		return erg;
	}
	else
	{
		m_errorPosition = m_sanitizer.realPos( m_evalPos );
		return 0.;
	}
}


double Parser::fkt(uint const id, uint eq, double const x)
{
	if ( !m_ufkt.contains( id ) || (eq > 2) )
	{
		m_error = NoSuchFunction;
		return 0;
	}
	else
		return fkt( m_ufkt[id]->eq[eq], x );
}

double Parser::fkt( Equation * eq, double const x )
{
	double *pd, (**pf)(double);
	double *stack, *stkptr;
	uint *puf;
	eq->mptr=eq->mem;
	stack=stkptr= new double [STACKSIZE];

	while(1)
	{
		switch(*eq->mptr++)
		{
			case KONST:
				pd=(double*)eq->mptr;
				*stkptr=*pd++;
				eq->mptr=(unsigned char*)pd;
				break;
			case XWERT:
				*stkptr=x;
				break;
			case YWERT:
				*stkptr=eq->oldy;
				break;
			case KWERT:
				*stkptr=eq->parent()->k;
				break;
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
			case MULT:
				stkptr[-1]*=*stkptr;
				--stkptr;
				break;
			case DIV:
				if(*stkptr==0.)*(--stkptr)=HUGE_VAL;
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
			case FKT:
				pf=(double(**)(double))eq->mptr;
				*stkptr=(*pf++)(*stkptr);
				eq->mptr=(unsigned char*)pf;
				break;
			case UFKT:
			{
				puf=(uint*)eq->mptr;
				uint id = *puf++;
				uint id_eq = *puf++;
				if ( m_ufkt.contains( id ) )
					*stkptr=fkt( m_ufkt[id]->eq[id_eq], *stkptr);
				eq->mptr=(unsigned char*)puf;
				break;
			}
			case ENDE:
				double const erg=*stkptr;
				delete [] stack;
				return erg;
		}
	}
}


int Parser::addFunction( QString str1, QString str2, Function::Type type )
{
	kDebug() << k_funcinfo << "str1="<<str1<<" str2="<<str2<<endl;
	
	QString str[2] = { str1, str2 };
	
	Function * temp = new Function( type );
	
	for ( unsigned i = 0; i < 2; ++i )
	{
		if ( str[i].isEmpty() || !temp->eq[i] )
			continue;
		
		if ( !temp->eq[i]->setFstr( str[i] ) )
		{
			kDebug() << "could not set fstr (fstr=\""<<str[i]<<"\")!\n";
			delete temp;
			return -1;
		}
	
		if ( fnameToID( temp->eq[i]->fname() ) != -1 )
		{
			kDebug() << "function name reused.\n";
			m_error = FunctionNameReused;
			delete temp;
			return -1;
		}
	}
	
	temp->id = getNewId();
	m_ufkt[ temp->id ] = temp;
	
	emit functionAdded( temp->id );
	kDebug() << k_funcinfo << "all ok\n";
	return temp->id; //return the unique ID-number for the function
}


bool Parser::isFstrValid( QString str )
{
	stkptr = stack = 0;
	m_error = ParseSuccess;
	m_errorPosition = 0;

	const int p1=str.indexOf('(');
	int p2=str.indexOf(',');
	const int p3=str.indexOf(")=");
	
	m_sanitizer.fixExpression( & str, p1+4 );
        
	if(p1==-1 || p3==-1 || p1>p3)
	{
		/// \todo find the position of the capital and set into m_errorPosition
		m_error = InvalidFunctionVariable;
		return false;
	}
	if ( p3+2 == str.length()) //empty function
	{
		/// \todo find the position of the capital and set into m_errorPosition
		m_error = EmptyFunction;
		return false;
	}
	if(p2==-1 || p2>p3)
		p2=p3;
	
	if (str.mid(p1+1, p2-p1-1) == "e")
	{
		/// \todo find the position of the capital and set into m_errorPosition
		m_error = InvalidFunctionVariable;
		return false;
	}
	
	QString fname = str.left(p1);
	
	if ( fname != fname.toLower() ) //isn't allowed to contain capital letters
	{
		m_error = CapitalInFunctionName;
		/// \todo find the position of the capital and set into m_errorPosition
		return false;
	}
	
// 	m_currentEquation = m_ownEquation;
// 	m_currentEquation->setFstr( str, true );
	(double) eval( str, p3+2, false );
	return (m_error == ParseSuccess);
}


void Parser::initEquation( Equation * eq )
{
	m_error = ParseSuccess;
	m_currentEquation = eq;
	mem = mptr = eq->mem;
	
	m_eval = eq->fstr();
	m_sanitizer.fixExpression( & m_eval, m_eval.indexOf('(')+4 );
	m_evalPos = m_eval.indexOf( '=' ) + 1;
	heir1();
	if ( !evalRemaining().isEmpty() && m_error == ParseSuccess )
		m_error = SyntaxError;		// Syntaxfehler
	addtoken(ENDE);
	m_errorPosition = -1;
}


bool Parser::removeFunction( Function * item )
{
	kDebug() << "Deleting id:" << item->id << endl;
	if (!item->dep.isEmpty())
	{
		KMessageBox::sorry(0,i18n("This function is depending on an other function"));
		return false;
	}
	
	foreach ( Function * it1, m_ufkt )
	{
		if (it1==item)
			continue;
		for(QList<int>::iterator it2=it1->dep.begin(); it2!=it1->dep.end(); ++it2)
			if ( (uint)*it2 == item->id )
				it2 = it1->dep.erase(it2);
	}
	
	uint const id = item->id;
	
	//kDebug() << "Deleting something" << endl;
	m_ufkt.remove(id);
	for ( unsigned i = 0; i < 2; ++i )
	{
		if ( item->eq[i] && (item->eq[i] == m_currentEquation) )
			m_currentEquation = m_ownEquation;
	}
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
	if(m_error!=ParseSuccess)
		return ;

	while(1)
	{
		if ( m_eval.length() <= m_evalPos )
			return;
		
		c = m_eval[m_evalPos];
		switch ( c.unicode() )
		{
			default:
				return ;

			case ' ':
				++m_evalPos;
				continue;
			case '+':
			case '-':
				++m_evalPos;
				addtoken(PUSH);
				heir2();
				if(m_error!=ParseSuccess)
					return;
		}
		switch ( c.unicode() )
		{
			case '+':
				addtoken(PLUS);
				break;
			case '-':
				addtoken(MINUS);
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
		addtoken(NEG);
	}
	else if ( match( QChar(0x221a) ) ) // square root symbol
	{
		heir2();
		if(m_error!=ParseSuccess)
			return;
		addtoken(SQRT);
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
			case ' ':
				++m_evalPos;
				continue;
			case '*':
			case '/':
				++m_evalPos;
				addtoken(PUSH);
				heir4();
				if(m_error!=ParseSuccess)
					return ;
		}
		switch ( c.unicode() )
		{
			case '*':
				addtoken(MULT);
				break;
			case '/':
				addtoken(DIV);
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
		addtoken(PUSH);
		primary();
		if(m_error!=ParseSuccess)
			return;
		addtoken(POW);
	}
}


void Parser::primary()
{
	if(match("("))
	{
		heir1();
		if(match(")")==0)
			m_error=MissingBracket;
		return;
	}
        int i;
        for(i=0; i<FANZ; ++i)
        {
                if(match(mfkttab[i].mfstr))
                {
                        primary();
                        addtoken(FKT);
                        addfptr(mfkttab[i].mfadr);
                        return;
                }
	}
	foreach ( Function * it, m_ufkt )
	{
		if ( evalRemaining() == "pi" ||
				   evalRemaining() == "e" ||
				   evalRemaining() == QChar(0x221E) )
			continue;

		for ( unsigned i = 0; i < 2; ++i )
		{
			if ( it->eq[i] && match(it->eq[i]->fname()) )
			{
				if (it->eq[i] == m_currentEquation)
				{
					kDebug() << k_funcinfo << "Matched!\n";
					m_error=RecursiveFunctionCall;
					return;
				}
				primary();
				addtoken(UFKT);
				addfptr( it->id, i );
				if ( m_currentEquation->parent() )
					it->dep.append(m_currentEquation->parent()->id);
				return;
			}
		}
	}
        
	// A constant
	if ( (m_eval.length()>m_evalPos) && constants()->isValidName( m_eval[m_evalPos] ) )
	{
		QVector<Constant> constants = m_constants->all();
		foreach ( Constant c, constants )
		{
			QChar tmp = c.constant;
			if ( match( tmp ) )
			{
				addtoken(KONST);
				addwert(c.value);
				return;
			}
		}
		m_error = NoSuchConstant;
		return;
	}
        
	if ( match("pi") || match( QChar(960) ) )
	{
		addtoken(KONST);
		addwert(M_PI);
		return;
	}

	if(match("e"))
	{
		addtoken(KONST);
		addwert(M_E);
		return;
	}

	if( match( QChar(0x221E) ) )
	{
		addtoken(KONST);
		addwert( INFINITY );
		return;
	}
	
	//if(match(ufkt[ixa].fvar.latin1()))
	if(match(m_currentEquation->fvar()))
	{
                addtoken(XWERT);
		return;
	}
	
	if(match("y"))
	{
                addtoken(YWERT);
		return;
	}
	
	//if(match(ufkt[ixa].fpar.latin1()))
	if(match( m_currentEquation->fpar() ))
	{
                addtoken(KWERT);
		return;
	}

	QByteArray remaining = evalRemaining().toLatin1();
	char * lptr = remaining.data();
	char * p = 0;
	double const w = strtod(lptr, &p);
	if( lptr != p )
	{
		m_evalPos += p-lptr;
		addtoken(KONST);
		addwert(w);
	}
	else
		m_error = SyntaxError;
}


int Parser::match( const QString & lit )
{
	if ( lit.isEmpty() )
		return 0;
	
	while( (m_eval.length() > m_evalPos) && (m_eval[m_evalPos] == ' ') )
		++m_evalPos;
	
	if ( lit != evalRemaining().left( lit.length() ) )
		return 0;
	
	m_evalPos += lit.length();
	return 1;
}


void Parser::addtoken(unsigned char token)
{
	if(stkptr>=stack+STACKSIZE-1)
	{
		m_error = StackOverflow;
		return;
	}

	if(evalflg==0)
	{
                if(mptr>=&mem[MEMSIZE-10])
					m_error = MemoryOverflow;
		else
                        *mptr++=token;
        
		switch(token)
		{
                        case PUSH:
                                ++stkptr;
                                break;
                        case PLUS:
                        case MINUS:
                        case MULT:
                        case DIV:
                        case POW:
                                --stkptr;
		}
	}
	else switch(token)
	{
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
	}
}


void Parser::addwert(double x)
{
        double *pd=(double*)mptr;

	if(evalflg==0)
	{
                if(mptr>=&mem[MEMSIZE-10])
					m_error = MemoryOverflow;
		else
		{
                        *pd++=x;
			mptr=(unsigned char*)pd;
		}
	}
	else
                *stkptr=x;
}


void Parser::addfptr(double(*fadr)(double))
{
        double (**pf)(double)=(double(**)(double))mptr;
        if( evalflg==0 )
        {
        if( mptr>=&mem[MEMSIZE-10] )
			m_error = MemoryOverflow;
        else
                {
                        *pf++=fadr;
                        mptr=(unsigned char*)pf;
                }
        }
        else
                *stkptr=(*fadr)(*stkptr);
}


void Parser::addfptr( uint id, uint eq_id )
{
	uint *p=(uint*)mptr;
	if(evalflg==0)
	{
		if(mptr>=&mem[MEMSIZE-10])
			m_error=MemoryOverflow;
		else
		{
			*p++=id;
			*p++=eq_id;
			mptr=(unsigned char*)p;
		}
	}
	else
	{
		Function * function = functionWithID( id );
		if ( function )
			*stkptr = fkt( function->eq[eq_id], *stkptr );
	}
}


int Parser::fnameToID(const QString &name)
{
	foreach ( Function * it, m_ufkt )
	{
		for ( unsigned i = 0; i < 2; ++i )
		{
			if ( it->eq[i] && (name == it->eq[i]->fname()) )
				return it->id;
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
			return i18n("Parser error at position %1:\n"
					"Syntax error", m_errorPosition+1);
			
		case MissingBracket:
			return i18n("Parser error at position %1:\n"
					"Missing parenthesis", m_errorPosition+1);
			
		case UnknownFunction:
			return i18n("Parser error at position %1:\n"
					"Function name unknown", m_errorPosition+1);
			
		case InvalidFunctionVariable:
			return i18n("Parser error at position %1:\n"
					"Void function variable", m_errorPosition+1);
			
		case TooManyFunctions:
			return i18n("Parser error at position %1:\n"
					"Too many functions", m_errorPosition+1);
			
		case MemoryOverflow:
			return i18n("Parser error at position %1:\n"
					"Token-memory overflow", m_errorPosition+1);
			
		case StackOverflow:
			return i18n("Parser error at position %1:\n"
					"Stack overflow", m_errorPosition+1);
			
		case FunctionNameReused:
			return i18n("Parser error at position %1:\n"
					"Name of function not free.", m_errorPosition+1);
			
		case RecursiveFunctionCall:
			return i18n("Parser error at position %1:\n"
					"recursive function not allowed.", m_errorPosition+1);
			
		case NoSuchConstant:
			return i18n("Could not find a defined constant at position %1.", m_errorPosition+1);
			
		case EmptyFunction:
			return i18n("Empty function");
			
		case CapitalInFunctionName:
			return i18n("The function name is not allowed to contain capital letters.");
			
		case NoSuchFunction:
			return i18n("Function could not be found.");
			
		case UserDefinedConstantInExpression:
			return i18n("The expression must not contain user-defined constants.");
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
// 	kDebug() << "returning str="<<str<<endl;
	return str;
}
//END class Parser



double ln(double x)
{
        return log(x);
}

double llog(double x)
{
        return log10(x);
}

double sign(double x)
{
        if(x<0.)
                return -1.;
        else
                if(x>0.)
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

double sqr(double x)
{
        return x*x;
}

double arsinh(double x)
{
        return log(x+sqrt(x*x+1));
}


double arcosh(double x)
{
        return log(x+sqrt(x*x-1));
}


double artanh(double x)
{
        return log((1+x)/(1-x))/2;
}

// sec, cosec, cot and their inverses

double sec(double x)
{
        return (1 / cos(x*Parser::radiansPerAngleUnit()));
}

double cosec(double x)
{
        return (1 / sin(x*Parser::radiansPerAngleUnit()));
}

double cot(double x)
{
        return (1 / tan(x*Parser::radiansPerAngleUnit()));
}

double arcsec(double x)
{
        if ( !Parser::radiansPerAngleUnit() )
                return ( 1/acos(x)* 180/M_PI );
        else
                return acos(1/x);
}

double arccosec(double x)
{
        return asin(1/x)* 1/Parser::radiansPerAngleUnit();
}

double arccot(double x)
{
        return atan(1/x)* 1/Parser::radiansPerAngleUnit();
}

// sech, cosech, coth and their inverses


double sech(double x)
{
        return (1 / cosh(x*Parser::radiansPerAngleUnit()));
}

double cosech(double x)
{
        return (1 / sinh(x*Parser::radiansPerAngleUnit()));
}

double coth(double x)
{
        return (1 / tanh(x*Parser::radiansPerAngleUnit()));
}

double arsech(double x)
{
        return arcosh(1/x)* 1/Parser::radiansPerAngleUnit();
}

double arcosech(double x)
{
        return arsinh(1/x)* 1/Parser::radiansPerAngleUnit();
}

double arcoth(double x)
{   return artanh(1/x)* 1/Parser::radiansPerAngleUnit();
}

//basic trigonometry functions

double lcos(double x)
{
        return cos(x*Parser::radiansPerAngleUnit());
}
double lsin(double x)
{
        return sin(x*Parser::radiansPerAngleUnit());
}
double ltan(double x)
{
        return tan(x*Parser::radiansPerAngleUnit());
}

double lcosh(double x)
{
        return cosh(x*Parser::radiansPerAngleUnit());
}
double lsinh(double x)
{
        return sinh(x*Parser::radiansPerAngleUnit());
}
double ltanh(double x)
{
        return tanh(x*Parser::radiansPerAngleUnit());
}

double arccos(double x)
{
        return acos(x) * 1/Parser::radiansPerAngleUnit();
}
double arcsin(double x)
{
        return asin(x)* 1/Parser::radiansPerAngleUnit();
}

double arctan(double x)
{
        return atan(x)* 1/Parser::radiansPerAngleUnit();
}


//BEGIN class Constants
Constants::Constants( Parser * parser )
{
	m_parser = parser;
}


QVector< Constant >::iterator Constants::find( QChar name )
{
	QVector<Constant>::iterator it;
	for ( it = m_constants.begin(); it != m_constants.end(); ++it )
	{
		if ( it->constant == name )
			break;
	}
	return it;
}


bool Constants::have( QChar name )
{
	return ( find( name ) != m_constants.end() );
}


void Constants::remove( QChar name )
{
// 	kDebug() << k_funcinfo << "removing " << name << endl;
	
	QVector<Constant>::iterator c = find( name );
	if ( c != m_constants.end() )
		m_constants.erase( c );
}


void Constants::add( Constant c )
{
// 	kDebug() << k_funcinfo << "adding " << c.constant << endl;
	
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
// 		case QChar::Symbol_Math:
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
// 		kDebug() << "konstant: " << tmp_constant << endl;
// 		kDebug() << "value: " << tmp_value << endl;
// 		kDebug() << "**************" << endl;
		
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
	conf.deleteGroup( "UserConstants", KConfigBase::Recursive );
	conf.deleteGroup( "UserConstants", 0 ); /// \todo remove this line when fix bug in kconfigbase
	
	
	conf.setGroup("UserConstants");
	QString tmp;
	
	int i = 0;
	foreach ( Constant c, m_constants )
	{
		tmp.setNum(i);
		conf.writeEntry("nameConstant"+tmp, QString( c.constant ) ) ;
		conf.writeEntry("valueConstant"+tmp, c.value);
// 		kDebug() << "wrote constant="<<c.constant<<" value="<<c.value<<endl;
		
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


void ExpressionSanitizer::fixExpression( QString * str, int pos )
{
	m_str = str;
	
	m_map.resize( m_str->length() );
	for ( int i = 0; i < m_str->length(); ++i )
		m_map[i] = i;
	
	remove( ' ' );
	
	m_map.insert( 0, 0 );
	m_map.insert( m_map.size(), m_map[ m_map.size()-1 ] );
	*str = ' ' + *str + ' ';
	
	// make sure all minus-like signs (including the actual unicode minus sign)
	// are represented by a dash (unicode 0x002d)
	QChar dashes[6] = { 0x2012, 0x2013, 0x2014, 0x2015, 0x2053, 0x2212 };
	for ( unsigned i = 0; i < 6; ++i )
		replace( dashes[i], '-' );
	
	// replace the proper unicode divide sign by the forward-slash
	replace( QChar( 0x2215 ), '/' );
	
	// replace the unicode middle-dot for multiplication by the star symbol
	replace( QChar( 0x2219 ), '*' );
	
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
	
	//insert '*' when it is needed
	QChar ch;
	bool function = false;
	for(int i=pos+1; i+1 <  str->length();i++)
	{
		ch = str->at(i);
		if ( str->at(i+1)=='(' && (ch.category()==QChar::Letter_Lowercase || ch=='H') )
		{
			// Work backwards to build up the full function name
			QString str_function(ch);
			int n=i-1;
			while (n>0 && str->at(n).category() == QChar::Letter_Lowercase )
			{
				str_function.prepend(str->at(n));
				--n;
			}
				
			for ( unsigned func = 0; func < FANZ; ++func )
			{
				if ( str_function == QString( m_parser->mfkttab[func].mfstr ) )
				{
					function = true;
					break;
				}
			}
				
			if ( !function )
			{
				// Not a predefined function, so search through the user defined functions (e.g. f(x), etc)
				// to see if it is one of those
				foreach ( Function * it, m_parser->m_ufkt )
				{
					for ( int j=i; j>0 && (str->at(j).isLetter() || str->at(j).isNumber() ) ; --j)
					{
						for ( uint k=0; k<2; ++k )
						{
							if ( it->eq[k] && (it->eq[k]->fname() == str->mid(j,i-j+1)) )
								function = true;
						}
					}
				}
			}
		}
		else  if (function)
			function = false;
                
		// either a number or a likely constant (H is reserved for the Heaviside step function)
		bool chIsNumeric = ch.isNumber() || m_parser->m_constants->isValidName( ch );
				
		if ( chIsNumeric && ( str->at(i-1).isLetter() || str->at(i-1) == ')' ) || (ch.isLetter() && str->at(i-1)==')') )
		{
			insert(i,'*');
// 			kDebug() << "inserted * before\n";
		}
		else if( (chIsNumeric || ch == ')') && ( str->at(i+1).isLetter() || str->at(i+1) == '(' ) || (ch.isLetter() && str->at(i+1)=='(' && !function ) )
		{
			insert(i+1,'*');
//  		kDebug() << "inserted * after, function="<<function<<" ch="<<ch<<"\n";
			i++;
		}
	}
	remove(" " );
	QString str_end = str->mid(pos);
	str_end = str_end.replace(m_decimalSymbol, "."); //replace the locale decimal symbol with a '.'
	str->truncate(pos);
	str->append(str_end);
 //	kDebug() << "str:" << str << endl;
}


void ExpressionSanitizer::remove( const QString & str )
{
// 	kDebug() << "Before:\n";
// 	displayMap();
	
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
	
// 	kDebug() << "After:\n";
// 	displayMap();
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
	
// 	kDebug() << "Before:\n";
// 	displayMap();
	
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
	
// 	kDebug() << "After:\n";
// 	displayMap();
}


void ExpressionSanitizer::insert( int i, QChar ch )
{
	m_map.insert( i, m_map[i] );
	m_str->insert( i, ch );
}


int ExpressionSanitizer::realPos( int evalPos )
{
	if ( m_map.isEmpty() || (evalPos < 0) )
		return -1;
	
	if ( evalPos >= m_map.size() )
	{
		kWarning() << k_funcinfo << "evalPos="<<evalPos<<" is out of range.\n";
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
