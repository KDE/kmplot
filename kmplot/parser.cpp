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
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

// local includes
#include "parser.h"
#include "settings.h"
#include "xparser.h"

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
                                   

Parser::Parser()
{   ps_init( UFANZ, MEMSIZE, STACKSIZE );
}


Parser::Parser( int anz, int m_size, int s_size )
{   ps_init( anz, m_size, s_size );
}


void Parser::ps_init(int anz, int m_size, int s_size)
{	int ix;

	ufanz=anz;
	memsize=m_size;
	stacksize=s_size;
	ufkt=new Ufkt[ufanz];
	evalflg=ixa=0;
	for(ix=0; ix<ufanz; ++ix)
	{	ufkt[ix].memsize=memsize;
		ufkt[ix].stacksize=stacksize;
		ufkt[ix].fname="";      //.resize(1);
		ufkt[ix].fvar="";       //.resize(1);
		ufkt[ix].fpar="";       //.resize(1);
		ufkt[ix].fstr="";       //.resize(1);
		ufkt[ix].mem=new unsigned char [memsize];
	}
}


Parser::~Parser()
{   delete [] ufkt;
}


Parser::Ufkt::Ufkt()
{
}


Parser::Ufkt::~Ufkt()
{   delete [] mem;
}


void Parser::setAngleMode(int angle)
{	if(angle==0)
		m_anglemode = 1;
	else
		m_anglemode = M_PI/180;	
}

void Parser::setDecimalSymbol(const QString c)
{
	m_decimalsymbol = c;
}

double Parser::anglemode()
{	return m_anglemode;
}

double Parser::eval(QString str)
{	double erg;
	stack=new double [stacksize];
	stkptr=stack;
	evalflg=1;
	
	str.remove(" " );
	//insert '*' when it is needed
	for(int i=0; i < (int)str.length();i++)
	{
		if( (str.at(i).isNumber() || str.at(i).category()==QChar::Letter_Uppercase )&& ( str.at(i-1).isLetter() || str.at(i-1) == ')' ) )
		{
			str.insert(i,'*');
		}
		else if( (str.at(i).isNumber() || str.at(i) == ')' || str.at(i).category()==QChar::Letter_Uppercase) && ( str.at(i+1).isLetter() || str.at(i+1) == '(' ) )
		{
			str.insert(i+1,'*');
			i++;
		}
	}
	if ( str.contains('y')!=0)
	{
		err=9;
		return 0;
	}
	
	lptr=str.latin1();
	err=0;
	heir1();
	if(*lptr!=0 && err==0) err=1;
	evalflg=0;
	erg=*stkptr;
	delete [] stack;
	if(err==0)
	{	errpos=0;
		return erg;
	}
	else
	{	errpos=lptr-(str.latin1())+1;
		return 0.;
	}
}


double Parser::Ufkt::fkt(double x)
{	unsigned char token;
	double *pd, (**pf)(double);
	double erg, *stack, *stkptr;
	Ufkt **puf;
	
	mptr=mem;
	stack=stkptr= new double [stacksize];
	while(1)
	{   switch(token=*mptr++)
	{  case	KONST:  pd=(double*)mptr;
			            *stkptr=*pd++;
				    mptr=(unsigned char*)pd;
				    break;
           case	XWERT:  *stkptr=x;
			            break;
           case	YWERT:  *stkptr=oldy;
			            break;
           case KWERT:  *stkptr=k;
			            break;

           case PUSH:   ++stkptr;
			            break;

           case PLUS:   stkptr[-1]+=*stkptr;
			            --stkptr;
			            break;

           case MINUS:  stkptr[-1]-=*stkptr;
			            --stkptr;
			            break;

           case MULT:   stkptr[-1]*=*stkptr;
			            --stkptr;
			            break;

           case DIV:    if(*stkptr==0.)*(--stkptr)=HUGE_VAL;
			            else
			            {   stkptr[-1]/=*stkptr;
				            --stkptr;
			            }
			            break;

           case POW:    stkptr[-1]=pow(*(stkptr-1), *stkptr);
			            --stkptr;
			            break;

           case NEG:    *stkptr=-*stkptr;
			            break;

           case	FKT:    pf=(double(**)(double))mptr;
			            *stkptr=(*pf++)(*stkptr);
			            mptr=(unsigned char*)pf;
			            break;

           case UFKT:   puf=(Ufkt**)mptr;
			            *stkptr=(*puf++)->fkt(*stkptr);
			            mptr=(unsigned char*)puf;
			            break;

           case ENDE:   erg=*stkptr;
				delete [] stack;
				return erg;
		}
	}
}

int Parser::getNextIndex()
{
	int ix = 0;
	while( ( ix < ufanz ) && !ufkt[ ix ].fname.isEmpty() ) ix++;
	if( ix == ufanz ) ix = -1;
	return ix;
}

int Parser::addfkt(QString str)
{   int ix;
	
	stkptr=stack=0;
	err=0;
	errpos=1;
	str.remove(" " );
	const int p1=str.find('(');
	int p2=str.find(',');
	const int p3=str.find(")=");
	
	//insert '*' when it is needed
	for(int i=p1+3; i < (int) str.length();i++)
	{
		if( (str.at(i).isNumber() || str.at(i).category()==QChar::Letter_Uppercase )&& ( str.at(i-1).isLetter() || str.at(i-1) == ')' ) )
		{
			str.insert(i,'*');
		}
		else if( (str.at(i).isNumber() || str.at(i) == ')' || str.at(i).category()==QChar::Letter_Uppercase) && ( str.at(i+1).isLetter() || str.at(i+1) == '(' ) )
		{
			str.insert(i+1,'*');
			i++;
		}
	}
		
	if(p1==-1 || p3==-1 || p1>p3)
	{   err=4;
		return -1;
	}
	if ( p3+2 == (int) str.length()) //empty function
	{   err=11;
		return -1;
	}
	if(p2==-1 || p2>p3) p2=p3;
	if(getfix(str.left(p1))!=-1)
	{   err=8;
		return -1;
	}
	else err=0;
	
	if (str.mid(p1+1, p2-p1-1) == "e")
	{   err=4;
		return -1;
	}

	QString str_end = str.mid(p3+3);
	str_end = str_end.replace(m_decimalsymbol, "."); //replace the locale decimal symbol with a '.'
	str.truncate(p3+3);
	str.append(str_end);
	
	for(ix=0; ix<ufanz; ++ix)
	{   if(ufkt[ix].fname.isEmpty())
		{   ufkt[ix].fname=str.left(p1);
			ufkt[ix].fvar=str.mid(p1+1, p2-p1-1);
			ufkt[ix].fstr=str;
			if(p2<p3) ufkt[ix].fpar=str.mid(p2+1, p3-p2-1);
			else ufkt[ix].fpar="";      //.resize(1);
			break;
		}
	}
	if ( ufkt[ix].fname != ufkt[ix].fname.lower() ) //isn't allowed to contain capital letters
	{
		err=12;
		return -1;
	}
	if(ix==ufanz)
	{   err=5;
		return -1;
	}	// zu viele Funktionen

	ixa=ix;
	mem=mptr=ufkt[ix].mem;
	lptr=(str.latin1())+p3+2;
	heir1();
	if(*lptr!=0 && err==0) err=1;		// Syntaxfehler
	addtoken(ENDE);

	if(err!=0)
	{   ufkt[ix].fname="";  //.resize(1);
		errpos=lptr-(str.latin1())+1;
		return -1;
	}

	
	errpos=0;
	return ix;
}

void Parser::reparse(int ix)
{
	kdDebug() << "Reparsing: " << ufkt[ix].fstr << endl;
	QString str = ufkt[ix].fstr.latin1();
	err=0;
	errpos=1;
	str.remove(" " );
	const int p1=str.find('(');
	int p2=str.find(',');
	const int p3=str.find(")=");
	
	//insert '*' when it is needed
	for(int i=p1+3; i < (int) str.length();i++)
	{
		if( (str.at(i).isNumber() || str.at(i).category()==QChar::Letter_Uppercase )&& ( str.at(i-1).isLetter() || str.at(i-1) == ')' ) )
		{
			str.insert(i,'*');
		}
		else if( (str.at(i).isNumber() || str.at(i) == ')' || str.at(i).category()==QChar::Letter_Uppercase) && ( str.at(i+1).isLetter() || str.at(i+1) == '(' ) )
		{
			str.insert(i+1,'*');
			i++;
		}
	}
		
	if(p1==-1 || p3==-1 || p1>p3)
	{   err=4;
	return;
	}
	if ( p3+2 == (int) str.length()) //empty function
	{   err=11;
	return;
	}
	if(p2==-1 || p2>p3) p2=p3;
	
	if (str.mid(p1+1, p2-p1-1) == "e")
	{   err=4;
	return;
	}

	QString str_end = str.mid(p3+3);
	str_end = str_end.replace(m_decimalsymbol, "."); //replace the locale decimal symbol with a '.'
	str.truncate(p3+3);
	str.append(str_end);
	
	ufkt[ix].fname=str.left(p1);
	ufkt[ix].fvar=str.mid(p1+1, p2-p1-1);
	if(p2<p3) ufkt[ix].fpar=str.mid(p2+1, p3-p2-1);
	else ufkt[ix].fpar="";
	
	if ( ufkt[ix].fname != ufkt[ix].fname.lower() ) //isn't allowed to contain capital letters
	{
		err=12;
		return;
	}
	
	ixa=ix;
	mem=mptr=ufkt[ix].mem;
	lptr=(str.latin1())+p3+2;
	heir1();
	if(*lptr!=0 && err==0) err=1;		// Syntaxfehler
	addtoken(ENDE);
	errpos=0;
}

int Parser::delfkt(QString name)
{   int ix;

	ix=getfix(name);
	if(ix!=-1) ufkt[ix].fname="";   //.resize(1);	// Name l�chen
	return ix;
}


int Parser::delfkt(int ix)
{   if(ix<0 || ix>=ufanz) return -1;	// ungltiger Index

	ufkt[ix].fname="";      //.resize(1);			// Name l�chen
	return ix;
}


double Parser::fkt(QString name, double x)
{   int ix;

	ix=getfix(name);
	if(ix==-1) return 0.;

	return ufkt[ix].fkt(x);
}


void Parser::heir1()
{   char c;

	heir2();
	if(err!=0) return ;

	while(1)
	{   switch(c=*lptr)
		{  default:     return ;

           case ' ':    ++lptr;
			            continue;

           case '+':
           case '-':    ++lptr;
			            addtoken(PUSH);
			            heir2();
			            if(err!=0) return ;
		}

		switch(c)
		{  case '+':    addtoken(PLUS);
			            break;

           case '-':    addtoken(MINUS);
		}
	}
}


void Parser::heir2()
{   if(match("-"))
	{   heir2();
		if(err!=0) return;

		addtoken(NEG);
	}

	else heir3();
}


void Parser::heir3()
{   char c;

	heir4();
	if(err!=0) return;

	while(1)
	{   switch(c=*lptr)
		{  default:     return ;

           case ' ':    ++lptr;
			            continue;

           case '*':
		   case '/':    ++lptr;
			            addtoken(PUSH);
			            heir4();
			            if(err!=0) return ;
		}

		switch(c)
		{  case '*':    addtoken(MULT);
			            break;

           case '/':    addtoken(DIV);
		}
	}
}


void Parser::heir4()
{   primary();
	if(err!=0) return;

	while(match("^"))
	{   addtoken(PUSH);
		primary();
		if(err!=0) return;
		addtoken(POW);
	}
}


void Parser::primary()
{   char *p;
	int i;
	double w;

	if(match("("))
	{   heir1();
		if(match(")")==0) err=2;	// fehlende Klammer
		return;
	}

	for(i=0; i<FANZ; ++i)
	{   if(match(mfkttab[i].mfstr))
		{   primary();
			addtoken(FKT);
			addfptr(mfkttab[i].mfadr);
			return;
		}
	}

	for(i=0; i<ufanz; ++i)
	{   if(ufkt[i].fname[0]==0) continue;
		if(match(ufkt[i].fname.latin1()))
		{   if(i==ixa) {err=9; return;}
		
			primary();
			addtoken(UFKT);
			addfptr(&ufkt[i]);
			return;
		}
	}
	// A constant
	if(lptr[0] >='A' && lptr[0]<='Z' )
	{   char tmp[2];
	tmp[1] = '\0';
	for( int i = 0; i< (int)constant.size();i++)
	{	
		tmp[0] = constant[i].constant;
		if ( match( tmp) )
		{
			addtoken(KONST);
			addwert(constant[i].value);
			return;
		}

	}
	err = 10;
	return;
	}
	
		
	if(match("pi"))
	{   addtoken(KONST);
		addwert(M_PI);
		return;
	}

	if(match("e"))
	{   addtoken(KONST);
		addwert(M_E);
		return;
	}

	if(match(ufkt[ixa].fvar.latin1()))
	{   addtoken(XWERT);
		return;
	}
	
	if(match("y"))
	{   addtoken(YWERT);
		return;
	}
	
	if(match(ufkt[ixa].fpar.latin1()))
	{   addtoken(KWERT);
		return;
	}

	w=strtod(lptr, &p);
	if(lptr!=p)
	{   lptr=p;
		addtoken(KONST);
		addwert(w);
	}
	else err=1;				// Syntax-Fehler
}


int Parser::match(const char *lit)
{   const char *p;

	if(*lit==0) return 0;
    
	while(*lptr==' ') ++lptr;
	p=lptr;
	while(*lit)
	{   if(*lit++!=*p++) return 0;
	}
	lptr=p;
	return 1;
}


void Parser::addtoken(unsigned char token)
{   if(stkptr>=stack+stacksize-1)
	{   err=7;
		return;
	}

	if(evalflg==0)
	{   if(mptr>=&mem[memsize-10]) err=6;
		else *mptr++=token;
        
		switch(token)
		{  case PUSH:   ++stkptr;
			            break;

           case PLUS:
		   case MINUS:
		   case MULT:
		   case DIV:
		   case POW:    --stkptr;
		}
	}
	else switch(token)
	{  case PUSH:   ++stkptr;
			        break;

       case PLUS:   stkptr[-1]+=*stkptr;
			        --stkptr;
			        break;

       case MINUS:  stkptr[-1]-=*stkptr;
			        --stkptr;
			        break;

       case MULT:   stkptr[-1]*=*stkptr;
			        --stkptr;
			        break;

       case DIV:    if(*stkptr==0.) *(--stkptr)=HUGE_VAL;
			        else
			        {   stkptr[-1]/=*stkptr;
				        --stkptr;
			        }
			        break;

       case POW:    stkptr[-1]=pow(*(stkptr-1), *stkptr);
			        --stkptr;
			        break;
	   case NEG:    *stkptr=-*stkptr;
	}
}


void Parser::addwert(double x)
{   double *pd=(double*)mptr;

	if(evalflg==0)
	{   if(mptr>=&mem[memsize-10]) err=6;
		else
		{   *pd++=x;
			mptr=(unsigned char*)pd;
		}
	}
	else *stkptr=x;
}


void Parser::addfptr(double(*fadr)(double))
{   double (**pf)(double)=(double(**)(double))mptr;

	if(evalflg==0)
	{   if(mptr>=&mem[memsize-10]) err=6;
		else
		{   *pf++=fadr;
			mptr=(unsigned char*)pf;
		}
	}
	else *stkptr=(*fadr)(*stkptr);
}


void Parser::addfptr(Ufkt *adr)
{   Ufkt **p=(Ufkt**)mptr;

	if(evalflg==0)
	{   if(mptr>=&mem[memsize-10]) err=6;
		else
		{   *p++=adr;
			mptr=(unsigned char*)p;
		}
	}
	else *stkptr=adr->fkt(*stkptr);
}


int Parser::chkfix(int ix)
{   if(ix<0 || ix>=ufanz) return -1;		// ungltiger Index
	if(ufkt[ix].fname.isEmpty()) return -1;	// keine Funktion
	return ix;
}


int Parser::getfkt(int ix, QString& name, QString& str)
{   if(ix<0 || ix>=ufanz) return -1;		// ungltiger Index
	if(ufkt[ix].fname.isEmpty()) return -1;	// keine Funktion
	name=ufkt[ix].fname.copy();
	str=ufkt[ix].fstr.copy();
	return ix;
}


int Parser::getfix(QString name)
{   int ix;

	err=0;
	for(ix=0; ix<ufanz; ++ix)
	{   if(name==ufkt[ix].fname) return ix;
	}
	err=3;					// Name nicht bekannt
	return -1;
}


int Parser::errmsg()
{   switch(err)
	{  case 1:  KMessageBox::error(0, i18n("Parser error at position %1:\n"
		                                   "Syntax error").arg(QString::number(errpos)), "KmPlot");
		        break;

       case 2:  KMessageBox::error(0, i18n("Parser error at position %1:\n"
		                                   "Missing parenthesis").arg(QString::number(errpos)), "KmPlot");
		        break;

       case 3:  KMessageBox::error(0, i18n("Parser error at position %1:\n"
		                                   "Function name unknown").arg(QString::number(errpos)), "KmPlot");
		        break;

       case 4:  KMessageBox::error(0, i18n("Parser error at position %1:\n"
		                                   "Void function variable").arg(QString::number(errpos)), "KmPlot");
		        break;

       case 5:  KMessageBox::error(0, i18n("Parser error at position %1:\n"
		                                   "Too many functions").arg(QString::number(errpos)), "KmPlot");
		        break;

       case 6:  KMessageBox::error(0, i18n("Parser error at position %1:\n"
		                                   "Token-memory overflow").arg(QString::number(errpos)), "KmPlot");
		        break;

       case 7:  KMessageBox::error(0, i18n("Parser error at position %1:\n"
		                                   "Stack overflow").arg(QString::number(errpos)), "KmPlot");
		        break;

       case 8:  KMessageBox::error(0, i18n("Parser error at position %1:\n"
		                                   "Name of function not free").arg(QString::number(errpos)), "KmPlot");
		        break;
	   
       case 9:  KMessageBox::error(0, i18n("Parser error at position %1:\n"
		                                   "recursive function not allowed").arg(QString::number(errpos)), "KmPlot");
		        break;
       case 10:  KMessageBox::error(0, i18n("Could not find a defined constant at position %1" ).arg(QString::number(errpos)),
                                                   "KmPlot");
     			break;
       case 11:  KMessageBox::error(0, i18n("Empty function"), "KmPlot");
		        break;
       case 12:  KMessageBox::error(0, i18n("The function name is not allowed to contain capital letters"), "KmPlot");
			break;
	}
    
	return err;
}

double ln(double x)
{   if (x>0)
	return log(x);
}

double llog(double x)
{   if (x>0)
	return log10(x);
}

double sign(double x)
{   if(x<0.) return -1.;
    else if(x>0.) return 1.;
    return 0.;
}

double sqr(double x)
{   return x*x;
}

double arsinh(double x)
{   return log(x+sqrt(x*x+1));
}


double arcosh(double x)
{   return log(x+sqrt(x*x-1));
}


double artanh(double x)
{   return log((1+x)/(1-x))/2;
}

// sec, cosec, cot and their inverses

double sec(double x)
{   return (1 / cos(x*Parser::anglemode()));
}

double cosec(double x)
{   return (1 / sin(x*Parser::anglemode()));
}

double cot(double x)
{   return (1 / tan(x*Parser::anglemode()));
}

double arcsec(double x)
{   if ( !Parser::anglemode() ) return ( 1/acos(x)* 180/M_PI );
    else return acos(1/x);
}

double arccosec(double x)
{   return asin(1/x)* 1/Parser::anglemode();
}

double arccot(double x)
{   return atan(1/x)* 1/Parser::anglemode();
}

// sech, cosech, coth and their inverses


double sech(double x)
{   return (1 / cosh(x*Parser::anglemode()));
}

double cosech(double x)
{   return (1 / sinh(x*Parser::anglemode()));
}

double coth(double x)
{   return (1 / tanh(x*Parser::anglemode()));
}

double arsech(double x)
{   return arcosh(1/x)* 1/Parser::anglemode();
}

double arcosech(double x)
{   return arsinh(1/x)* 1/Parser::anglemode();
}

double arcoth(double x)
{   return artanh(1/x)* 1/Parser::anglemode();
}

//basic trigonometry functions

double lcos(double x)
{   return cos(x*Parser::anglemode());
}
double lsin(double x)
{   return sin(x*Parser::anglemode());
}
double ltan(double x)
{   return tan(x*Parser::anglemode());
}

double lcosh(double x)
{   return cosh(x*Parser::anglemode());
}
double lsinh(double x)
{   return sinh(x*Parser::anglemode());
}
double ltanh(double x)
{   return tanh(x*Parser::anglemode());
}

double arccos(double x)
{   return acos(x) * 1/Parser::anglemode();
}
double arcsin(double x)
{   return asin(x)* 1/Parser::anglemode();
}

double arctan(double x)
{   return atan(x)* 1/Parser::anglemode();
}