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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.
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
	linewidth = 0;
	f1_linewidth = 0;
	f2_linewidth = 0;
	integral_linewidth = 0;
	double dmin = 0.0;
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
}


Parser::Parser()
{
	ps_init();
}


void Parser::ps_init()
{
	evalflg=0;
        Ufkt temp;
        temp.fname = temp.fvar = temp.fpar = temp.fstr = "";
        temp.mem=new unsigned char [MEMSIZE];
        ufkt.append(temp );
        current_item = ufkt.begin();
}


Parser::~Parser()
{
        kdDebug() << "Exiting......" << endl;
        for( QValueVector<Ufkt>::iterator it = ufkt.begin(); it != ufkt.end(); ++it)
        {
                kdDebug() << "Deleting something... :-)" << endl;
                delete [](*it).mem;
        }
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
        uint i = 0;
        bool found = false;
        while (1 )
        {
                found = false;
                for( QValueVector<Ufkt>::iterator it = ufkt.begin(); it != ufkt.end(); ++it)
                {
                        if (it->id == i && !it->fname.isEmpty())
                        {
                                found = true;
                                break;
                        }
                }
                if (!found)
                        return i;
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
		err=9;
                delete []stack;
		return 0;
	}
	for (uint i=0;i<str.length();i++ )
		if (str.at(i).category() == QChar::Letter_Uppercase)
		{
			err=14;
			delete []stack;
			return 0;
		}
	
	lptr=str.latin1();
	err=0;
	heir1();
	if(*lptr!=0 && err==0) err=1;
	evalflg=0;
	double const erg=*stkptr;
	delete [] stack;
	if(err==0)
	{
                errpos=0;
		return erg;
	}
	else
	{
                errpos=lptr-(str.latin1())+1;
		return 0.;
	}
}

int Parser::idValue(int const ix)
{
        if ( ix >=0 && ix<(int)ufkt.count() ) // range check
        {
                if ( !( ufkt.count()==1 && ufkt[0].fname.isEmpty() ) )
                        return ufkt[ix].id;
        }
        return -1;

}

int Parser::ixValue(uint const id)
{
        int ix=0;
        for( QValueVector<Ufkt>::iterator it = ufkt.begin(); it != ufkt.end(); ++it)
        {
                if ( it->id ==id)
                        return ix;
                ix++;
        }
        return -1;
}

double Parser::fkt(uint const id, double const x)
{
        for( QValueVector<Ufkt>::iterator it = ufkt.begin(); it != ufkt.end(); ++it)
        {
                if ( it->id == id)
                        return fkt(it,x);
        }
        err=13;
        return 0;
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
                                for( QValueVector<Ufkt>::iterator ite = ufkt.begin(); ite != ufkt.end(); ++ite)
                                {
                                        if ( ite->id == id)
                                        {
                                                *stkptr=fkt(ite, *stkptr);
                                                break;
                                        }
                                }
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
	err=0;
	errpos=1;
	const int p1=str.find('(');
	int p2=str.find(',');
	const int p3=str.find(")=");
        fix_expression(str,p1+4);
        
	if(p1==-1 || p3==-1 || p1>p3)
	{   err=4;
		return -1;
	}
	if ( p3+2 == (int) str.length()) //empty function
	{   err=11;
		return -1;
	}
	if(p2==-1 || p2>p3) p2=p3;
	
	if( fnameToId(str.left(p1))!=-1 )
	{
                err=8;
		return -1;
	}
	else
                err=0;
	
	if (str.mid(p1+1, p2-p1-1) == "e")
	{   err=4;
		return -1;
	}
        
        if ( ufkt.begin()->fname.isEmpty() )
        {
                ufkt.begin()->id = 0;
                //kdDebug() << "ufkt.begin()->id:" << ufkt.begin()->id << endl;
        }
        else
        {
                Ufkt temp;
				if ( !temp.fstr.isEmpty() && temp.fstr.at(0) == 'y')
			temp.id = ufkt.last().id; //the function belongs to the last inserted function
		else
                	temp.id = getNewId();
                temp.mem=new unsigned char [MEMSIZE];
                ufkt.append(temp );
        }
        QString const fname = str.left(p1);
        Ufkt *temp = &ufkt.last();
	temp->fstr=extstr;
        temp->mptr = 0;
        temp->fname=fname;
        temp->fvar=str.mid(p1+1, p2-p1-1);
        if(p2<p3) temp->fpar=str.mid(p2+1, p3-p2-1);
        else temp->fpar="";      //.resize(1);
        
        kdDebug() << "temp.id:" << temp->id << endl;
        
	if ( temp->fname != temp->fname.lower() ) //isn't allowed to contain capital letters
	{
		delfkt(temp);
		err=12;
		return -1;
	}
        current_item = temp;
	mem=mptr=temp->mem;
	lptr=(str.latin1())+p3+2;
	heir1();
	if(*lptr!=0 && err==0) err=1;		// Syntaxfehler
	addtoken(ENDE);
	if(err!=0)
	{
                errpos=lptr-(str.latin1())+1;
		delfkt(temp);
		return -1;
	}
	errpos=0;
	return temp->id; //return the unique ID-number for the function
}

void Parser::reparse(int ix)
{
        reparse( &ufkt[ix] );
}

void Parser::reparse(Ufkt *item)
{
	kdDebug() << "Reparsing: " << item->fstr << endl;
	QString str = item->fstr.latin1();
	err=0;
	errpos=1;

	const int p1=str.find('(');
	int p2=str.find(',');
	const int p3=str.find(")=");
	
        fix_expression(str,p1+4);
        
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
	
	item->fname=str.left(p1);
	item->fvar=str.mid(p1+1, p2-p1-1);
	if(p2<p3) item->fpar=str.mid(p2+1, p3-p2-1);
	else item->fpar="";
	
	if ( item->fname != item->fname.lower() ) //isn't allowed to contain capital letters
	{
		err=12;
		return;
	}
	
	//ixa=ix;
        current_item = item;
	mem=mptr=item->mem;
	lptr=(str.latin1())+p3+2;
	heir1();
	if(*lptr!=0 && err==0) err=1;		// Syntaxfehler
	addtoken(ENDE);
	errpos=0;
}

void Parser::fix_expression(QString &str, int const pos)
{
        str.remove(" " );
        
        //insert '*' when it is needed
        QChar ch;
        bool function = false;
        for(uint i=pos; i <  str.length();i++)
        {
                ch = str.at(i);
                if ( str.at(i+1)=='(' && ch.category()==QChar::Letter_Lowercase )
                {
                        QString str_function(ch);
                        int n=i-1;
                        while (n>0 && str.at(n).category() == QChar::Letter_Lowercase )
                        {
                                     str_function.prepend(str.at(n));
                                     --n;
                        }
                        if (str_function == "tanh" || str_function == "tan" || str_function =="sqrt" || str_function =="sqr" || str_function =="sin" || str_function =="sinh" || str_function =="sign" || str_function =="sech" || str_function =="sec" || str_function =="log" || str_function =="ln" || str_function =="exp" || str_function =="coth" || str_function =="cot" || str_function =="cosh" || str_function =="cosech" || str_function =="cosec" || str_function =="cos" || str_function =="artanh" || str_function =="arsinh" || str_function =="arsech" || str_function =="arctan" || str_function =="arcsin" || str_function =="arcsec" || str_function =="arcoth" || str_function =="arcosh" || str_function =="arcosech" || str_function =="arccot" || str_function =="arccosec" || str_function =="arccos" || str_function =="abs" || str_function=="arctanh" || str_function=="arcsinh" || str_function=="arccosh")
                                function = true;
                        else
                                for( QValueVector<Ufkt>::iterator it = ufkt.begin(); it != ufkt.end(); ++it)
                                {
                                        for ( int j=i; j>0 && (str.at(j).isLetter() || str.at(j).isNumber() ) ; --j)
                                        {
                                                if ( it->fname == str.mid(j,i-j+1) )
                                                        function = true;
                                        }
                                }
                }
                else  if (function)
                        function = false;
                
                if( (ch.isNumber() || ch.category()==QChar::Letter_Uppercase )&& ( str.at(i-1).isLetter() || str.at(i-1) == ')' ) || (ch.isLetter() && str.at(i-1)==')') )
                        str.insert(i,'*');
                else if( (ch.isNumber() || ch == ')' || ch.category()==QChar::Letter_Uppercase) && ( str.at(i+1).isLetter() || str.at(i+1) == '(' ) || (ch.isLetter() && str.at(i+1)=='(' && !function ) )
                {
                        str.insert(i+1,'*');
                        i++;
                }
        }
        QString str_end = str.mid(pos);
        str_end = str_end.replace(m_decimalsymbol, "."); //replace the locale decimal symbol with a '.'
        str.truncate(pos);
        str.append(str_end);
        //kdDebug() << "str:" << str << endl;
}

bool Parser::delfkt( Ufkt *item)
{
	kdDebug() << "Deleting id:" << item->id << endl;
	if (!item->dep.isEmpty())
	{
	  KMessageBox::error(0,i18n("This function is depending on an other function"));
	  return false;
	}
	for(QValueVector<Ufkt>::iterator it1=ufkt.begin(); it1!=ufkt.end(); ++it1)
	{
		if (it1==item)
			continue;
		for(QValueList<int>::iterator it2=it1->dep.begin(); it2!=it1->dep.end(); ++it2)
			if ( (uint)*it2 == item->id )
				it2 = it1->dep.erase(it2);
	}
	
        if ( ufkt.count()==1 )
        {
                //kdDebug() << "first item, don't delete" << endl;
		item->fname="";
        }
        else
        {
                //kdDebug() << "Deleting something" << endl;
		QChar const extstr_c = item->fstr.at(0);
		uint const id = item->id;
		delete []item->mem;
                ufkt.erase(item);
		if ( extstr_c == 'x')
		{
			int const ix = ixValue(id+1);
			if (ix!= -1 && ufkt[ix].fstr.at(0) == 'y')
				delfkt( &ufkt[ix]);
		}
		else if ( extstr_c == 'y')
		{
			int const ix = ixValue(id-1);
			if (ix!= -1 && ufkt[ix].fstr.at(0) == 'x')
				delfkt( &ufkt[ix]);
		}
        }
	return true;
}

bool Parser::delfkt(uint id)
{
	int ix = ixValue(id);
	if ( ix!=-1 && delfkt(&ufkt[ix]))
	  	return true;
	else
		return false;
}

uint Parser::countFunctions()
{
        uint const count = ufkt.count();
        if (count == 1 && ufkt.begin()->fname.isEmpty())
                return 0;
        else
                return count;
}

void Parser::heir1()
{
        char c;
	heir2();
	if(err!=0) return ;

	while(1)
	{
                switch(c=*lptr)
		{
                        default:
                                return ;

                        case ' ':
                                ++lptr;
                                continue;
                        case '+':
                        case '-':
                                ++lptr;
                                addtoken(PUSH);
                                heir2();
                                if(err!=0)
                                        return;
		}
		switch(c)
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
		if(err!=0)
                        return;
		addtoken(NEG);
	}
	else
                heir3();
}


void Parser::heir3()
{
        char c;
	heir4();
	if(err!=0)
                return;
	while(1)
	{
                switch(c=*lptr)
		{
                        default:
                                return;
                        case ' ':
                                ++lptr;
                                continue;
                        case '*':
                        case '/':
                                ++lptr;
                                addtoken(PUSH);
                                heir4();
                                if(err!=0)
                                        return ;
		      }
		switch(c)
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
	if(err!=0)
                return;
	while(match("^"))
	{
                addtoken(PUSH);
		primary();
		if(err!=0)
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
                        err=2;	// fehlende Klammer
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
        for( QValueVector<Ufkt>::iterator it = ufkt.begin(); it != ufkt.end(); ++it)
	{
                if(QString(lptr)=="pi" || QString(lptr)=="e") continue;

		if( match(it->fname.latin1()) )
		{
                        if (it == current_item)
                        {
                                err=9;
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
	if(lptr[0] >='A' && lptr[0]<='Z' )
	{
                char tmp[2];
                tmp[1] = '\0';
                for( i = 0; i< (int)constant.size();i++)
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
        if(match(current_item->fvar.latin1())) 
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
        if(match(current_item->fpar.latin1()))
	{
                addtoken(KWERT);
		return;
	}

        char *p;
	double const w=strtod(lptr, &p);
	if(lptr!=p)
	{
                lptr=p;
		addtoken(KONST);
		addwert(w);
	}
	else
                err=1;				// Syntax-Fehler
}


int Parser::match(const char *lit)
{

        const char *p;
	if(*lit==0)
                return 0;
	while(*lptr==' ')
                ++lptr;
	p=lptr;

	while(*lit)
	{
                if(*lit++!=*p++)
                        return 0;
	}
	lptr=p;
	return 1;
}


void Parser::addtoken(unsigned char token)
{
        if(stkptr>=stack+STACKSIZE-1)
	{
                err=7;
		return;
	}

	if(evalflg==0)
	{
                if(mptr>=&mem[MEMSIZE-10])
                        err=6;
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
                        err=6;
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
                err=6;
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
                if(mptr>=&mem[MEMSIZE-10]) err=6;
		else
		{
                        *p++=id;
			mptr=(unsigned char*)p;
		}
	}
	else
        {
                for( QValueVector<Ufkt>::iterator it = ufkt.begin(); it != ufkt.end(); ++it)
                        if ( it->id == id)
                        {
                                *stkptr=fkt(it, *stkptr);
                                break;
                        }
        }
}


int Parser::fnameToId(const QString &name)
{
        for( QValueVector<Ufkt>::iterator it = ufkt.begin(); it != ufkt.end(); ++it)
	{
                if(name==it->fname)
                        return it->id;
	}
	return -1;     // Name nicht bekannt
}


int Parser::parserError(bool showMessageBox)
{
        if (!showMessageBox)
                return err;
        switch(err)
	{
                case 1:  KMessageBox::error(0, i18n("Parser error at position %1:\n"
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
		                                      "Name of function not free.").arg(QString::number(errpos)), "KmPlot");
                        break;
                case 9:  KMessageBox::error(0, i18n("Parser error at position %1:\n"
		                                      "recursive function not allowed.").arg(QString::number(errpos)), "KmPlot");
                        break;
                case 10:  KMessageBox::error(0, i18n("Could not find a defined constant at position %1." ).arg(QString::number(errpos)),
                                                                "KmPlot");
                        break;
                case 11:  KMessageBox::error(0, i18n("Empty function"), "KmPlot");
                        break;
                case 12:  KMessageBox::error(0, i18n("The function name is not allowed to contain capital letters."), "KmPlot");
                        break;
                case 13:  KMessageBox::error(0, i18n("Function could not be found."), "KmPlot");
                        break;
		case 14:  KMessageBox::error(0, i18n("The expression must not contain user-defined constants."), "KmPlot");
			break;
	}
        return err;
}



// static
QString Parser::number( double value )
{
       QString str = QString::number( value, 'g', 6 );
       str.replace( 'e', "*10^" );
//     kDebug() << "returning str="<<str<<endl;
       return str;
}



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
