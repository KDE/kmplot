/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter Möller
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


#include "parser.h"

//#define M_PI	3.14159265358979
//#define M_E	2.71828182845905


//	Bezeichnungen und Adressen
//	der möglichen math. Funktionen :

Parser::Mfkt Parser::mfkttab[FANZ]={{"abs", fabs},          // Absolutbetrag
                                    {"sign", sign},         // Signum
                                    {"sqrt", sqrt}, 	    // Quadratwurzel
                                    {"sqr", sqr}, 			// Quadrat
                                    {"exp", exp}, 			// e-Funktion
                                    {"ln", log}, 			// nat. Logarithmus
                                    {"log", log10}, 		// 10-er Logarithmus
                                    {"sinh", sinh}, 		// Sinushyperbolikus
                                    {"cosh", cosh}, 		// Cosinushyperbolikus
                                    {"tanh", tanh}, 		// Tangenshyperbolikus
                                    {"sin", sin}, 			// Sinus
                                    {"cos", cos}, 			// Cosinus
                                    {"tan", tan}, 			// Tangens
                                    {"arsinh", arsinh}, 	// Area-Sinushyperbolikus
                                    {"arcosh", arcosh}, 	// Area-Cosinushyperbolikus
                                    {"artanh", artanh}, 	// Area-Tangenshyperbolikus
                                    {"arcsin", asin}, 		// Arcussinus
                                    {"arccos", acos}, 		// Arcuscosinus
                                    {"arctan", atan}		// Arcustangens
                                   };

Parser::Parser()
{   ps_init(UFANZ, MEMSIZE, STACKSIZE);
}


Parser::Parser(int anz, int m_size, int s_size)
{   ps_init(anz, m_size, s_size);
}


void Parser::ps_init(int anz, int m_size, int s_size)
{   int ix;

	ufanz=anz;
	memsize=m_size;
	stacksize=s_size;
	ufkt=new Ufkt[ufanz];
	evalflg=ixa=0;
	for(ix=0; ix<ufanz; ++ix)
	{   ufkt[ix].memsize=memsize;
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


double Parser::eval(QString str)
{   double erg;

	stack=new double [stacksize];
	stkptr=stack;
	evalflg=1;
	lptr=str.latin1();
	err=0;
	heir1();
	if(*lptr!=0 && err==0) err=1;
	evalflg=0;
	erg=*stkptr;
	delete [] stack;
	if(err==0)
	{   errpos=0;
		return erg;
	}
	else
	{   errpos=lptr-(str.latin1())+1;
		return 0.;
	}
}


double Parser::Ufkt::fkt(double x)
{   unsigned char token;
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


int Parser::addfkt(QString str)
{   int ix, p1, p2, p3;

	stkptr=stack=0;
	err=0;
	errpos=1;
	p1=str.find('(');
	p2=str.find(',');
	p3=str.find(")=");
	if(p1==-1 || p3==-1 || p1>p3)
	{   err=4;
		return -1;
	}
    
	if(p2==-1 || p2>p3) p2=p3;
	if(getfix(str.left(p1))!=-1)
	{   err=8;
		return -1;
	}
	else err=0;

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


int Parser::delfkt(QString name)
{   int ix;

	ix=getfix(name);
	if(ix!=-1) ufkt[ix].fname="";   //.resize(1);	// Name löschen
	return ix;
}


int Parser::delfkt(int ix)
{   if(ix<0 || ix>=ufanz) return -1;	// ungültiger Index

	ufkt[ix].fname="";      //.resize(1);			// Name löschen
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
{   if(ix<0 || ix>=ufanz) return -1;		// ungültiger Index
	if(ufkt[ix].fname.isEmpty()) return -1;	// keine Funktion
	return ix;
}


int Parser::getfkt(int ix, QString& name, QString& str)
{   if(ix<0 || ix>=ufanz) return -1;		// ungültiger Index
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
	}
    
	return err;
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
