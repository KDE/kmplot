/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter M�ler
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

// standard c(++) includes
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>

// local includes
#include "parser.h"
#include "settings.h"
#include "xparser.h"
//Added by qt3to4:
#include <QList>

double Parser::m_anglemode = 0;

/// List of predefined functions.
Parser::Mfkt Parser::mfkttab[ FANZ ]=
{
	{"tanh", ltanh},		// Tangens hyperbolicus
	{"tan", ltan}, 		// Tangens
	{"sqrt", sqrt},		// Square root
	{"sqr", sqr}, 		// Square
	{"sinh", lsinh}, 	// Sinus hyperbolicus
	{"sin", lsin}, 		// Sinus
	{"sign", sign},         // Signum
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
	{"arsech", arsech},	// Area-secans hyperbolicus = invers of sech
        {"arctanh", artanh},     // The same as artanh
        {"arcsinh", arsinh},     // The same as arsinh
        {"arccosh", arcosh},     // The same as arcosh
	{"arctan", arctan},	// Arcus tangens = inverse of tan
	{"arcsin", arcsin}, 	// Arcus sinus = inverse of sin
	{"arcsec", arcsec},	// Arcus secans = inverse of sec
	{"arcoth", arcoth},	// Area-co-tangens hyperbolicus = inverse of coth
	{"arcosh", arcosh}, 	// Area-cosinus hyperbolicus = inverse of cosh
	{"arcosech", arcosech},	// Area-co-secans hyperbolicus = inverse of cosech
	{"arccot", arccot},	// Arcus co-tangens = inverse of cotan
	{"arccosec", arccosec},	// Arcus co-secans = inverse of cosec
	{"arccos", arccos}, 	// Arcus cosinus = inverse of cos
	{"abs", fabs}		// Absolute value
};


//BEGIN class Ufkt
Ufkt::Ufkt()
{
	id = 0;
	mem = 0;
	mptr = 0;
	k = 0;
	oldy = 0;
	f_mode = true;
	f1_mode = false;
	f2_mode = false;
	integral_mode = false;
	integral_use_precision = false;
	linewidth = 0.0;
	f1_linewidth = 0.0;
	f2_linewidth = 0.0;
	integral_linewidth = 0.0;
	dmin = 0.0;
	dmax = 0.0;
	oldyprim = 0.0;
	oldx = 0.0;
	starty = 0.0;
	startx = 0.0;
	integral_precision = 0.0;
	use_slider = -1;
	usecustomxmin = false;
	usecustomxmax = false;
}


Ufkt::~Ufkt()
{
	delete [] mem;
	mem = 0;
}


void Ufkt::copyFrom( const Ufkt & function )
{
	f_mode = function.f_mode;
	f1_mode = function.f1_mode;
	f2_mode = function.f2_mode;
	integral_mode = function.integral_mode;
	integral_use_precision = function.integral_use_precision;
	linewidth = function.linewidth;
	f1_linewidth = function.f1_linewidth;
	f2_linewidth = function.f2_linewidth;
	integral_linewidth = function.integral_linewidth;
	str_dmin = function.str_dmin;
	str_dmax = function.str_dmax;
	dmin = function.dmin;
	dmax = function.dmax;
	str_startx = function.str_startx;
	str_starty = function.str_starty;
	oldx = function.oldx;
	starty = function.starty;
	startx = function.startx;
	integral_precision = function.integral_precision;
	color = function.color;
	f1_color = function.f1_color;
	f2_color = function.f2_color;
	integral_color = function.integral_color;
	parameters = function.parameters;
	use_slider = function.use_slider;
	usecustomxmin = function.usecustomxmin;
	usecustomxmax = function.usecustomxmax;
}
//END class Ufkt



//BEGIN class Parser
Parser::Parser()
{
	m_evalPos = 0;
	evalflg = 0;
	m_nextFunctionID = 0;
	m_constants = new Constants( this );
	
	m_ownFunction = new Ufkt;
	m_ownFunction->id =  int(1e7); // a nice large ID that'll never get used
	m_ownFunction->mem = new unsigned char [MEMSIZE];
	
	current_item = m_ownFunction;
}


Parser::~Parser()
{
	kDebug() << "Exiting......" << endl;
	foreach ( Ufkt * function, m_ufkt )
		delete function;
	delete m_ownFunction;
	
	delete m_constants;
}

void Parser::setAngleMode(int angle)
{
        if(angle==0)
		m_anglemode = 1;
	else
		m_anglemode = M_PI/180;	
}

void Parser::setDecimalSymbol(const QString c)
{
	m_decimalsymbol = c;
}

double Parser::anglemode()
{
        return m_anglemode;
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

double Parser::eval(QString str)
{
	stack=new double [STACKSIZE];
	stkptr=stack;
	evalflg=1;
	
	fix_expression(str,0);
        
	if ( str.contains('y')!=0)
	{
		err = RecursiveFunctionCall;
		delete []stack;
		return 0;
	}
	for (int i=0;i<str.length();i++ )
	{
		if ( constants()->isValidName( str[i] ) )
		{
			err = UserDefinedConstantInExpression;
			delete []stack;
			return 0;
		}
	}
	
	m_eval = str;
	m_evalPos = 0;
	err = ParseSuccess;
	heir1();
	if( !evalRemaining().isEmpty() && err==ParseSuccess)
		err=SyntaxError;
	evalflg=0;
	double const erg=*stkptr;
	delete [] stack;
	if ( err == ParseSuccess )
	{
		errpos=0;
		return erg;
	}
	else
	{
		errpos = m_evalPos+1;
		return 0.;
	}
}


double Parser::fkt(uint const id, double const x)
{
	if ( !m_ufkt.contains( id ) )
	{
		err = NoSuchFunction;
		return 0;
	}
	else
		return fkt( m_ufkt[id], x );
}

double Parser::fkt(Ufkt *it, double const x)
{
	double *pd, (**pf)(double);
        double *stack, *stkptr;
	uint *puf;
	it->mptr=it->mem;
	stack=stkptr= new double [STACKSIZE];

	while(1)
	{
                switch(*it->mptr++)
                {
                        case KONST:
                                pd=(double*)it->mptr;
                                *stkptr=*pd++;
                                it->mptr=(unsigned char*)pd;
                                break;
                        case XWERT:
                                *stkptr=x;
                                break;
                        case YWERT:
                                *stkptr=it->oldy;
                                break;
                        case KWERT:
                                *stkptr=it->k;
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
                        case FKT:
                                pf=(double(**)(double))it->mptr;
                                *stkptr=(*pf++)(*stkptr);
                                it->mptr=(unsigned char*)pf;
                                break;
                        case UFKT:
                        {
                                puf=(uint*)it->mptr;
                                uint id = *puf++;
								if ( m_ufkt.contains( id ) )
									*stkptr=fkt( m_ufkt[id], *stkptr);
                                it->mptr=(unsigned char*)puf;
                                break;
                        }
                        case ENDE:
                                double const erg=*stkptr;
                                delete [] stack;
                                return erg;
                }
        }
}

int Parser::addfkt(QString str)
{
	QString const extstr = str;
	stkptr=stack=0;
	err = ParseSuccess;
	errpos=1;
	const int p1=str.indexOf('(');
	int p2=str.indexOf(',');
	const int p3=str.indexOf(")=");
	fix_expression(str,p1+4);
        
	if(p1==-1 || p3==-1 || p1>p3)
	{
		err = InvalidFunctionVariable;
		return -1;
	}
	if ( p3+2 == str.length()) //empty function
	{
		err = EmptyFunction;
		return -1;
	}
	if(p2==-1 || p2>p3) p2=p3;
	
	if( fnameToId(str.left(p1))!=-1 )
	{
		err = FunctionNameReused;
		return -1;
	}
	else
		err = ParseSuccess;
	
	if (str.mid(p1+1, p2-p1-1) == "e")
	{
		err = InvalidFunctionVariable;
		return -1;
	}
        
	Ufkt * temp = new Ufkt;
	temp->id = getNewId();
	temp->mem=new unsigned char [MEMSIZE];
	m_ufkt[ temp->id ] = temp;
	
	QString const fname = str.left(p1);
	temp->fstr=extstr;
        temp->mptr = 0;
        temp->fname=fname;
		temp->fvar=str.mid(p1+1, p2-p1-1);
		if(p2<p3) temp->fpar=str.mid(p2+1, p3-p2-1);
        else temp->fpar="";      //.resize(1);
        
//         kDebug() << "temp.id:" << temp->id << endl;
        
	if ( temp->fname != temp->fname.toLower() ) //isn't allowed to contain capital letters
	{
		delfkt(temp);
		err = CapitalInFunctionName;
		return -1;
	}
        current_item = temp;
	mem=mptr=temp->mem;
	m_eval = str;
	m_evalPos = p3 + 2;
	heir1();
	if ( !evalRemaining().isEmpty() && err==ParseSuccess)
		err = SyntaxError;
	addtoken(ENDE);
	if ( err != ParseSuccess )
	{
		errpos = m_evalPos + 1;
		delfkt(temp);
		return -1;
	}
	errpos=0;
	
	emit functionAdded( temp->id );
	return temp->id; //return the unique ID-number for the function
}

void Parser::reparse(int id)
{
	reparse( m_ufkt[id] );
}

void Parser::reparse(Ufkt *item)
{
	kDebug() << "Reparsing: " << item->fstr << endl;
	QString str = item->fstr;
	err = ParseSuccess;
	errpos=1;

	const int p1=str.indexOf('(');
	int p2=str.indexOf(',');
	const int p3=str.indexOf(")=");
	
	fix_expression(str,p1+4);
        
	if(p1==-1 || p3==-1 || p1>p3)
	{
		err = InvalidFunctionVariable;
		return;
	}
	if ( p3+2 == str.length()) //empty function
	{
		err = EmptyFunction;
		return;
	}
	if(p2==-1 || p2>p3) p2=p3;
	
	if (str.mid(p1+1, p2-p1-1) == "e")
	{
		err = InvalidFunctionVariable;
		return;
	}
	
	item->fname=str.left(p1);
	item->fvar=str.mid(p1+1, p2-p1-1);
	if(p2<p3) item->fpar=str.mid(p2+1, p3-p2-1);
	else item->fpar="";
	
	if ( item->fname != item->fname.toLower() ) //isn't allowed to contain capital letters
	{
		err = CapitalInFunctionName;
		return;
	}
	
	//ixa=ix;
        current_item = item;
	mem=mptr=item->mem;
	
	m_eval = str;
	m_evalPos = p3 + 2;
	heir1();
	if ( !evalRemaining().isEmpty() && err == ParseSuccess )
		err=SyntaxError;		// Syntaxfehler
	addtoken(ENDE);
	errpos=0;
}

void Parser::fix_expression(QString &str, int const pos)
{
        str.remove(" " );
        str=" "+str+" ";
		
		/// \todo tidy up code for dealing with capital letter H (heaviside step function)
        
        //insert '*' when it is needed
        QChar ch;
        bool function = false;
        for(int i=pos+1; i+1 <  str.length();i++)
		{
			ch = str.at(i);
			if ( str.at(i+1)=='(' && (ch.category()==QChar::Letter_Lowercase || ch=='H') )
			{
				// Work backwards to build up the full function name
				QString str_function(ch);
				int n=i-1;
				while (n>0 && str.at(n).category() == QChar::Letter_Lowercase )
				{
					str_function.prepend(str.at(n));
					--n;
				}
				
				for ( unsigned func = 0; func < FANZ; ++func )
				{
					if ( str_function == QString( mfkttab[func].mfstr ) )
					{
						function = true;
						break;
					}
				}
				
			if ( !function )
			{
				// Not a predefined function, so search through the user defined functions (e.g. f(x), etc)
				// to see if it is one of those
				foreach ( Ufkt * it, m_ufkt )
				{
					for ( int j=i; j>0 && (str.at(j).isLetter() || str.at(j).isNumber() ) ; --j)
					{
						if ( it->fname == str.mid(j,i-j+1) )
							function = true;
					}
				}
			}
		}
		else  if (function)
			function = false;
                
		// either a number or a likely constant (H is reserved for the Heaviside step function)
		bool chIsNumeric = ch.isNumber() || m_constants->isValidName( ch );
				
				if ( chIsNumeric && ( str.at(i-1).isLetter() || str.at(i-1) == ')' ) || (ch.isLetter() && str.at(i-1)==')') )
				{
					str.insert(i,'*');
// 					kDebug() << "inserted * before\n";
				}
				else if( (chIsNumeric || ch == ')') && ( str.at(i+1).isLetter() || str.at(i+1) == '(' ) || (ch.isLetter() && str.at(i+1)=='(' && !function ) )
                {
					str.insert(i+1,'*');
// 					kDebug() << "inserted * after, function="<<function<<" ch="<<ch<<"\n";
					i++;
                }
        }
        str.remove(" " );
        QString str_end = str.mid(pos);
        str_end = str_end.replace(m_decimalsymbol, "."); //replace the locale decimal symbol with a '.'
        str.truncate(pos);
        str.append(str_end);
        //kDebug() << "str:" << str << endl;
}

bool Parser::delfkt( Ufkt * item )
{
	kDebug() << "Deleting id:" << item->id << endl;
	if (!item->dep.isEmpty())
	{
		KMessageBox::sorry(0,i18n("This function is depending on an other function"));
		return false;
	}
	
	foreach ( Ufkt * it1, m_ufkt )
	{
		if (it1==item)
			continue;
		for(QList<int>::iterator it2=it1->dep.begin(); it2!=it1->dep.end(); ++it2)
			if ( (uint)*it2 == item->id )
				it2 = it1->dep.erase(it2);
	}
	
	uint const id = item->id;
	
	//kDebug() << "Deleting something" << endl;
	QChar const extstr_c = item->fstr.at(0);
	m_ufkt.remove(id);
	if ( item == current_item )
		current_item = m_ownFunction;
	delete item;
	
	if ( extstr_c == 'x')
	{
		if ( m_ufkt.contains(id+1) && m_ufkt[id+1]->fstr[0] == 'y' )
			delfkt( m_ufkt[id+1] );
	}
	else if ( extstr_c == 'y')
	{
		if ( m_ufkt.contains(id-1) && m_ufkt[id-1]->fstr[0] == 'x' )
			delfkt( m_ufkt[id-1] );
	}
	
	emit functionRemoved( id );
	return true;
}

bool Parser::delfkt(uint id)
{
	return m_ufkt.contains( id ) && delfkt( m_ufkt[id] );
}

uint Parser::countFunctions()
{
	return m_ufkt.count();
}

void Parser::heir1()
{
	QChar c;
	heir2();
	if(err!=ParseSuccess)
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
				if(err!=ParseSuccess)
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
	if(match("-"))
	{
		heir2();
		if(err!=ParseSuccess)
			return;
		addtoken(NEG);
	}
	else
		heir3();
}


void Parser::heir3()
{
	QChar c;
	heir4();
	if(err!=ParseSuccess)
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
				if(err!=ParseSuccess)
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
	if(err!=ParseSuccess)
		return;
	while(match("^"))
	{
		addtoken(PUSH);
		primary();
		if(err!=ParseSuccess)
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
			err=MissingBracket;
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
	foreach ( Ufkt * it, m_ufkt )
	{
		if ( evalRemaining() == "pi" || evalRemaining() == "e" )
			continue;

		if( match(it->fname.toLatin1().data()) )
		{
                        if (it == current_item)
                        {
							err=RecursiveFunctionCall;
                                return;
                        }
			primary();
			addtoken(UFKT);
                        addfptr( it->id );
			it->dep.append(current_item->id);
			return;
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
		err = NoSuchConstant;
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
	//if(match(ufkt[ixa].fvar.latin1()))
	if(match(current_item->fvar.toLatin1().data()))
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
	if(match(current_item->fpar.toLatin1().data()))
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
		err = SyntaxError;
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
		err = StackOverflow;
		return;
	}

	if(evalflg==0)
	{
                if(mptr>=&mem[MEMSIZE-10])
					err = MemoryOverflow;
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
	}
}


void Parser::addwert(double x)
{
        double *pd=(double*)mptr;

	if(evalflg==0)
	{
                if(mptr>=&mem[MEMSIZE-10])
					err = MemoryOverflow;
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
			err = MemoryOverflow;
        else
                {
                        *pf++=fadr;
                        mptr=(unsigned char*)pf;
                }
        }
        else
                *stkptr=(*fadr)(*stkptr);
}


void Parser::addfptr(uint id)
{
        uint *p=(uint*)mptr;
        if(evalflg==0)
	{
		if(mptr>=&mem[MEMSIZE-10])
			err=MemoryOverflow;
		else
		{
                        *p++=id;
			mptr=(unsigned char*)p;
		}
	}
	else
	{
		Ufkt * function = functionWithID( id );
		if ( function )
			*stkptr = fkt( function, *stkptr );
	}
}


int Parser::fnameToId(const QString &name)
{
	foreach ( Ufkt * it, m_ufkt )
	{
                if(name==it->fname)
                        return it->id;
	}
	return -1;     // Name nicht bekannt
}


Parser::Error Parser::parserError(bool showMessageBox)
{
	if (!showMessageBox)
		return err;
	switch(err)
	{
		case ParseSuccess:
			break;
		case SyntaxError:
			KMessageBox::sorry(0, i18n("Parser error at position %1:\n"
					"Syntax error").arg(QString::number(errpos)), "KmPlot");
			break;
		case MissingBracket:
			KMessageBox::sorry(0, i18n("Parser error at position %1:\n"
					"Missing parenthesis").arg(QString::number(errpos)), "KmPlot");
			break;
		case UnknownFunction:
			KMessageBox::sorry(0, i18n("Parser error at position %1:\n"
					"Function name unknown").arg(QString::number(errpos)), "KmPlot");
			break;
		case InvalidFunctionVariable:
			KMessageBox::sorry(0, i18n("Parser error at position %1:\n"
					"Void function variable").arg(QString::number(errpos)), "KmPlot");
			break;
		case TooManyFunctions:
			KMessageBox::sorry(0, i18n("Parser error at position %1:\n"
					"Too many functions").arg(QString::number(errpos)), "KmPlot");
			break;
		case MemoryOverflow:
			KMessageBox::sorry(0, i18n("Parser error at position %1:\n"
					"Token-memory overflow").arg(QString::number(errpos)), "KmPlot");
			break;
		case StackOverflow:
			KMessageBox::sorry(0, i18n("Parser error at position %1:\n"
					"Stack overflow").arg(QString::number(errpos)), "KmPlot");
			break;
		case FunctionNameReused:
			KMessageBox::sorry(0, i18n("Parser error at position %1:\n"
					"Name of function not free.").arg(QString::number(errpos)), "KmPlot");
			break;
		case RecursiveFunctionCall:
			KMessageBox::sorry(0, i18n("Parser error at position %1:\n"
					"recursive function not allowed.").arg(QString::number(errpos)), "KmPlot");
			break;
		case NoSuchConstant:
			KMessageBox::sorry(0, i18n("Could not find a defined constant at position %1." ).arg(QString::number(errpos)),
							   "KmPlot");
			break;
		case EmptyFunction:
			KMessageBox::sorry(0, i18n("Empty function"), "KmPlot");
			break;
		case CapitalInFunctionName:
			KMessageBox::sorry(0, i18n("The function name is not allowed to contain capital letters."), "KmPlot");
			break;
		case NoSuchFunction:
			KMessageBox::sorry(0, i18n("Function could not be found."), "KmPlot");
			break;
		case UserDefinedConstantInExpression:
			KMessageBox::sorry(0, i18n("The expression must not contain user-defined constants."), "KmPlot");
			break;
	}
	return err;
}


QString Parser::evalRemaining() const
{
	QString current( m_eval );
	return current.right( QMAX( 0, current.length() - m_evalPos ) );
}


Ufkt * Parser::functionWithID( int id ) const
{
	return m_ufkt.contains( id ) ? m_ufkt[id] : 0;
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
        return (1 / cos(x*Parser::anglemode()));
}

double cosec(double x)
{
        return (1 / sin(x*Parser::anglemode()));
}

double cot(double x)
{
        return (1 / tan(x*Parser::anglemode()));
}

double arcsec(double x)
{
        if ( !Parser::anglemode() )
                return ( 1/acos(x)* 180/M_PI );
        else
                return acos(1/x);
}

double arccosec(double x)
{
        return asin(1/x)* 1/Parser::anglemode();
}

double arccot(double x)
{
        return atan(1/x)* 1/Parser::anglemode();
}

// sech, cosech, coth and their inverses


double sech(double x)
{
        return (1 / cosh(x*Parser::anglemode()));
}

double cosech(double x)
{
        return (1 / sinh(x*Parser::anglemode()));
}

double coth(double x)
{
        return (1 / tanh(x*Parser::anglemode()));
}

double arsech(double x)
{
        return arcosh(1/x)* 1/Parser::anglemode();
}

double arcosech(double x)
{
        return arsinh(1/x)* 1/Parser::anglemode();
}

double arcoth(double x)
{   return artanh(1/x)* 1/Parser::anglemode();
}

//basic trigonometry functions

double lcos(double x)
{
        return cos(x*Parser::anglemode());
}
double lsin(double x)
{
        return sin(x*Parser::anglemode());
}
double ltan(double x)
{
        return tan(x*Parser::anglemode());
}

double lcosh(double x)
{
        return cosh(x*Parser::anglemode());
}
double lsinh(double x)
{
        return sinh(x*Parser::anglemode());
}
double ltanh(double x)
{
        return tanh(x*Parser::anglemode());
}

double arccos(double x)
{
        return acos(x) * 1/Parser::anglemode();
}
double arcsin(double x)
{
        return asin(x)* 1/Parser::anglemode();
}

double arctan(double x)
{
        return atan(x)* 1/Parser::anglemode();
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
	kDebug() << k_funcinfo << "m_constants.size()="<<m_constants.size()<<endl;
	foreach ( Constant c, m_constants )
	{
		tmp.setNum(i);
		conf.writeEntry("nameConstant"+tmp, QString( c.constant ) ) ;
		conf.writeEntry("valueConstant"+tmp, c.value);
		kDebug() << "wrote constant="<<c.constant<<" value="<<c.value<<endl;
		
		i++;
	}
}
//END class Constants

#include "parser.moc"
