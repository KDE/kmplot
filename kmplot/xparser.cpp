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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

// KDE includes
#include <dcopclient.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

// local includes
#include "xparser.h"

XParser::XParser(bool &mo) : DCOPObject("Parser"), Parser(), m_modified(mo)
{
        // setup slider support
	setDecimalSymbol( KGlobal::locale()->decimalSymbol() );
}

XParser::~XParser()
{
}

bool XParser::getext( Ufkt *item, const QString fstr )
{
  	bool errflg = false;
   	int p1, p2, p3, pe;
	QString tstr;
	pe = fstr.length();
	if ( fstr.find( 'N' ) != -1 )
		item->f_mode = false;
	else
	{
		if ( fstr.find( "A1" ) != -1 )
			item->f1_mode = true;
		if ( fstr.find( "A2" ) != -1 )
			item->f2_mode = true;
	}
	switch ( fstr[0].latin1() )
	{
	  case 'x':
	  case 'y':
	  case 'r':
	    item->f1_mode = item->f2_mode = false;
	}

	p1 = fstr.find( "D[" );
	if ( p1 != -1 )
	{
		p1 += 2;
		const QString str = fstr.mid( p1, pe - p1);
		p2 = str.find(',');
		p3 = str.find(']');
		if ( p2 > 0 && p2 < p3 )
		{
			tstr = str.left( p2 );
			item->dmin = eval( tstr );
			if ( parserError(false) )
				errflg = true;
			tstr = str.mid( p2 + 1, p3 - p2 - 1 );
			item->dmax = eval( tstr );
			if ( parserError(false) )
				errflg = true;
			if ( item->dmin > item->dmax )
				errflg = true;
		}
		else
			errflg = true;
	}
	p1 = fstr.find( "P[" );
	if ( p1 != -1 )
	{
		int i = 0;
		p1 += 2;
		QString str = fstr.mid( p1, 1000);
		p3 = str.find( ']' );
		do
		{
			p2 = str.find( ',' );
			if ( p2 == -1 || p2 > p3 )
				p2 = p3;
			tstr = str.left( p2++ );
			str = str.mid( p2, 1000 );
			item->parameters.append( ParameterValueItem(tstr, eval( tstr )) );
			if ( parserError(false) )
			{
				errflg = true;
				break;
			}
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

double XParser::a1fkt( Ufkt *u_item, double x, double h )
{
	return ( fkt(u_item, x + h ) - fkt( u_item, x ) ) / h;
}

double XParser::a2fkt( Ufkt *u_item, double x, double h )
{
	return ( fkt( u_item, x + h + h ) - 2 * fkt( u_item, x + h ) + fkt( u_item, x ) ) / h / h;
}

void XParser::findFunctionName(QString &function_name, int const id, int const type)
{
  char last_character;
  int pos;
  if ( function_name.length()==2/*type == XParser::Polar*/ || type == XParser::ParametricX || type == XParser::ParametricY)
    pos=1;
  else
    pos=0;
  for ( ; ; ++pos)
  {
    last_character = 'f';
    for (bool ok=true; last_character<'x'; ++last_character)
    {
      if ( pos==0 && last_character == 'r') continue;
      function_name.at(pos)=last_character;
      for( QValueVector<Ufkt>::iterator it = ufkt.begin(); it != ufkt.end(); ++it)
      {
        if (it == ufkt.begin() && it->fname.isEmpty() ) continue;
        if ( it->fstr.startsWith(function_name+'(') && (int)it->id!=id) //check if the name is free
          ok = false;
      }
      if ( ok) //a free name was found
      {
        //kdDebug() << "function_name:" << function_name << endl;
        return;
      }
      ok = true;
    }
    function_name.at(pos)='f';
    function_name.append('f');
  }
  function_name = "e"; //this should never happen
}

void XParser::fixFunctionName( QString &str, int const type, int const id)
{
  if ( str.startsWith( "y=" ) ) //we find a free function name
  {
    str.remove( 0, 2 );
    str.prepend("(x)=");
    QString function_name;
    findFunctionName(function_name, id, type);
    str.prepend( function_name );
  }

  int p1=str.find('(');
  int p2=str.find(')');
  if( p1>=0 && str.at(p2+1)=='=')
  {
    if ( type == XParser::Polar && str.at(0)!='r' )
    {
      if (str.at(0)=='(')
      {
        str.prepend('f');
        p1++;
        p2++;
      }
      str.prepend('r');
      p1++;
      p2++;
    }
    QString const fname = str.left(p1);
    for ( QValueVector<Ufkt>::iterator it = ufkt.begin(); it!=ufkt.end(); ++it )
    {
      if (it->fname == fname)
      {
        str = str.mid(p1,str.length()-1);
        QString function_name;
        if ( type == XParser::Polar )
          function_name = "rf";
        else if ( type == XParser::ParametricX )
          function_name = "x";
        else if ( type == XParser::ParametricY )
          function_name = "y";
        else
          function_name = "f";
        findFunctionName(function_name, id, type);
        str.prepend( function_name );
        return;
      }
    }
  }
  else if ( p1==-1 || !str.at(p1+1).isLetter() ||  p2==-1 || str.at(p2+1 )!= '=')
  {
    QString function_name;
    if ( type == XParser::Polar )
      function_name = "rf";
    else if ( type == XParser::ParametricX )
      function_name = "xf";
    else if ( type == XParser::ParametricY )
      function_name = "yf";
    else
      function_name = "f";
    str.prepend("(x)=");
    findFunctionName(function_name, id, type);
    str.prepend( function_name );
  }
}

double XParser::euler_method(const double x, const QValueVector<Ufkt>::iterator it)
{
	double const y = it->oldy + ((x-it->oldx) * it->oldyprim);
	it->oldy = y;
	it->oldx = x;
	it->oldyprim = fkt( it, x ); //yprim;
	return y;
}

QRgb XParser::defaultColor(int function)
{
	switch ( function%10 )
        {
                case 0:
                        return Settings::color0().rgb();
                        break;
                case 1:
                        return Settings::color1().rgb();
                        break;
                case 2:
                        return Settings::color2().rgb();
                        break;
                case 3:
                        return Settings::color3().rgb();
                        break;
                case 4:
                        return Settings::color4().rgb();
                        break;
                case 5:
                        return Settings::color5().rgb();
                        break;
                case 6:
                        return Settings::color6().rgb();
                        break;
                case 7:
                        return Settings::color7().rgb();
                        break;
                case 8:
                        return Settings::color8().rgb();
                        break;
                case 9:
                        return Settings::color9().rgb();
                        break;
                default:
                        return Settings::color0().rgb();
                        break;
        }
}

void XParser::prepareAddingFunction(Ufkt *temp)
{
        temp->color = temp->f1_color = temp->f2_color = temp->integral_color = defaultColor(getNextIndex() );
        temp->linewidth = temp->f1_linewidth = temp->f2_linewidth = temp->integral_linewidth = linewidth0;
        temp->f_mode = true;
        temp->f1_mode = false;
        temp->f2_mode = false;
        temp->integral_mode = false;
        temp->integral_precision = Settings::stepWidth();
        temp->usecustomxmin = false;
	temp->usecustomxmax = false;
        temp->use_slider = -1;
        //TODO temp->slider_min = 0; temp->slider_max = 50;

}
int XParser::getNextIndex()
{
        //return ufkt.count();
        return getNewId();
}

QStringList XParser::listFunctionNames()
{
	QStringList list;
	for( QValueVector<Ufkt>::iterator it = ufkt.begin(); it != ufkt.end(); ++it)
	{
		list.append(it->fname);
	}
	return list;	
}

bool XParser::functionFVisible(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	return ufkt[ix].f_mode;
}
bool XParser::functionF1Visible(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	return ufkt[ix].f1_mode;
}
bool XParser::functionF2Visible(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	return ufkt[ix].f2_mode;
}
bool XParser::functionIntVisible(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	return ufkt[ix].integral_mode;
}

bool XParser::setFunctionFVisible(bool visible, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	ufkt[ix].f_mode = visible;
	m_modified = true;
	return true;
}
bool XParser::setFunctionF1Visible(bool visible, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	ufkt[ix].f1_mode = visible;
	m_modified = true;
	return true;
}
bool XParser::setFunctionF2Visible(bool visible, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	ufkt[ix].f2_mode = visible;
	m_modified = true;
	return true;
}
bool XParser::setFunctionIntVisible(bool visible, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	ufkt[ix].integral_mode = visible;
	m_modified = true;
	return true;
}

QString XParser::functionStr(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return "";
	return ufkt[ix].fstr;
}

QColor XParser::functionFColor(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return QColor();
	return QColor(ufkt[ix].color);
}
QColor XParser::functionF1Color(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return QColor();
	return QColor(ufkt[ix].f1_color);
}
QColor XParser::functionF2Color(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return QColor();
	return QColor(ufkt[ix].f2_color);
}
QColor XParser::functionIntColor(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return QColor();
	return QColor(ufkt[ix].integral_color);
}
bool XParser::setFunctionFColor(const QColor &color, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	ufkt[ix].color = color.rgb();
	m_modified = true;
	return true;
}
bool XParser::setFunctionF1Color(const QColor &color, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	ufkt[ix].color = color.rgb();
	m_modified = true;
	return true;
}		
bool XParser::setFunctionF2Color(const QColor &color, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	ufkt[ix].color = color.rgb();
	m_modified = true;
	return true;
}
bool XParser::setFunctionIntColor(const QColor &color, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	ufkt[ix].color = color.rgb();
	m_modified = true;
	return true;
}

int XParser::functionFLineWidth(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return 0;
	return ufkt[ix].linewidth;
}
int XParser::functionF1LineWidth(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return int();
	return ufkt[ix].f1_linewidth;
}
int XParser::functionF2LineWidth(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return int();
	return ufkt[ix].f2_linewidth;
}
int XParser::functionIntLineWidth(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return int();
	return ufkt[ix].integral_linewidth;
}
bool XParser::setFunctionFLineWidth(int linewidth, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	ufkt[ix].linewidth = linewidth;
	m_modified = true;
	return true;
}
bool XParser::setFunctionF1LineWidth(int linewidth, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	ufkt[ix].f1_linewidth = linewidth;
	m_modified = true;
	return true;
}		
bool XParser::setFunctionF2LineWidth(int linewidth, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	ufkt[ix].f2_linewidth = linewidth;
	m_modified = true;
	return true;
}
bool XParser::setFunctionIntLineWidth(int linewidth, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	ufkt[ix].integral_linewidth = linewidth;
	m_modified = true;
	return true;
}

QString XParser::functionMinValue(uint id)
{
  int const ix = ixValue(id);
  if (ix==-1)
    return int();
  return ufkt[ix].str_dmin;
}

bool XParser::setFunctionMinValue(const QString &min, uint id)
{
  int const ix = ixValue(id);
  if (ix==-1)
    return false;
  ufkt[ix].str_dmin = min;
  m_modified = true;
  return true;
}

QString XParser::functionMaxValue(uint id)
{
  int const ix = ixValue(id);
  if (ix==-1)
    return int();
  return ufkt[ix].str_dmax;
}

bool XParser::setFunctionMaxValue(const QString &max, uint id)
{
  int const ix = ixValue(id);
  if (ix==-1)
    return false;
  ufkt[ix].str_dmax = max;
  m_modified = true;
  return true;
}

QString XParser::functionStartXValue(uint id)
{
  int const ix = ixValue(id);
  if (ix==-1)
    return int();
  return ufkt[ix].str_startx;
}

bool XParser::setFunctionStartXValue(const QString &x, uint id)
{
  int const ix = ixValue(id);
  if (ix==-1)
    return false;
  ufkt[ix].str_startx = x;
  m_modified = true;
  return true;
}

QString XParser::functionStartYValue(uint id)
{
  int const ix = ixValue(id);
  if (ix==-1)
    return int();
  return ufkt[ix].str_starty;
}

bool XParser::setFunctionStartYValue(const QString &y, uint id)
{
  int const ix = ixValue(id);
  if (ix==-1)
    return false;
  ufkt[ix].str_starty = y;
  m_modified = true;
  return true;
}

QStringList XParser::functionParameterList(uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return QStringList();
	Ufkt *item = &ufkt[ix];
	QStringList str_parameter;
	for ( QValueList<ParameterValueItem>::iterator it = item->parameters.begin(); it != item->parameters.end(); ++it)
		str_parameter.append( (*it).expression);
	return str_parameter;
}
bool XParser::functionAddParameter(const QString &new_parameter, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	Ufkt *tmp_ufkt = &ufkt[ix];
	for ( QValueList<ParameterValueItem>::iterator it = tmp_ufkt->parameters.begin(); it != tmp_ufkt->parameters.end(); ++it)
		if ( (*it).expression == new_parameter) //check if the parameter already exists
			return false;

	double const result = eval(new_parameter);
	if ( parserError(false) != 0)
		return false;
	tmp_ufkt->parameters.append( ParameterValueItem(new_parameter,result) );
	m_modified = true;
	return true;
}
bool XParser::functionRemoveParameter(const QString &remove_parameter, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	Ufkt *tmp_ufkt = &ufkt[ix];
	
	bool found = false;
	QValueList<ParameterValueItem>::iterator it;
	for ( it = tmp_ufkt->parameters.begin(); it != tmp_ufkt->parameters.end(); ++it)
		if ( (*it).expression == remove_parameter) //check if the parameter already exists
		{
			found = true;
			break;
		}
	if (!found)
		return false;
	tmp_ufkt->parameters.remove(it);
	m_modified = true;
	return true;
}
int XParser::addFunction(const QString &f_str)
{
	QString added_function(f_str);
	int const pos = added_function.find(';');
	if (pos!=-1)
	  added_function = added_function.left(pos);
	
	fixFunctionName(added_function);
	if ( added_function.at(0)== 'x' || added_function.at(0)== 'y') //TODO: Make it possible to define parametric functions
		return -1;
	if  ( added_function.contains('y') != 0)
		return -1;
	int const id = addfkt( added_function );
	if (id==-1)
		return -1;
	Ufkt *tmp_ufkt = &ufkt.last();
	prepareAddingFunction(tmp_ufkt);
	if ( pos!=-1 && !getext( tmp_ufkt, f_str ) )
	{
		Parser::delfkt( tmp_ufkt );
		return -1;
	}
	m_modified = true;
	return id;
}

bool XParser::addFunction(const QString &fstr_const, bool f_mode, bool f1_mode, bool f2_mode, bool integral_mode, bool integral_use_precision, int linewidth, int f1_linewidth, int f2_linewidth, int integral_linewidth, const QString &str_dmin, const QString &str_dmax, const QString &str_startx, const QString &str_starty, double integral_precision, QRgb color, QRgb f1_color, QRgb f2_color, QRgb integral_color, QStringList str_parameter, int use_slider)
{
	QString fstr(fstr_const);
	switch ( fstr.at(0).latin1() )
	{
	  case 'r':
	  {
	    fixFunctionName(fstr, XParser::Polar);
	    break;
	  }
	  case 'x':
	    fixFunctionName(fstr, XParser::ParametricX);
	    break;
	  case 'y':
	    fixFunctionName(fstr, XParser::ParametricY);
	    break;
	  default:
	    fixFunctionName(fstr, XParser::Function);
	    break;
	}
	int const id = addfkt( fstr );
	if ( id==-1 )
		return false;
	Ufkt *added_function = &ufkt.last();
	added_function->f_mode = f_mode;
	added_function->f1_mode = f1_mode;
	added_function->f2_mode = f2_mode;
	added_function->integral_mode = integral_mode;
	added_function->integral_use_precision = integral_use_precision;
	added_function->linewidth = linewidth;
	added_function->f1_linewidth = f1_linewidth;
	added_function->f2_linewidth = f2_linewidth;
	added_function->integral_linewidth = integral_linewidth;
	
  if ( str_dmin.isEmpty() )
    added_function->usecustomxmin = false;
  else //custom minimum range
  {
    added_function->usecustomxmin = true;
    added_function->str_dmin = str_dmin;
    added_function->dmin = eval(str_dmin);
  }
  if ( str_dmax.isEmpty() )
    added_function->usecustomxmax = false;
  else //custom maximum range
  {
    added_function->usecustomxmax = true;
    added_function->str_dmax = str_dmax;
    added_function->dmax = eval(str_dmax);
  }
	added_function->str_startx = str_startx;
	added_function->str_starty = str_starty;
	if ( !str_starty.isEmpty() )
		added_function->starty = eval(str_starty);
	if ( !str_startx.isEmpty() )
		added_function->startx = eval(str_startx);
	added_function->oldx = 0;
	added_function->integral_precision = integral_precision;
	added_function->color = color;
	added_function->f1_color = f1_color;
	added_function->f2_color = f2_color;
	added_function->integral_color = integral_color;
	added_function->use_slider = use_slider;
	for( QStringList::Iterator it = str_parameter.begin(); it != str_parameter.end(); ++it )
	{
		double result = eval(*it);
		if ( parserError(false) != 0)
			continue;
		added_function->parameters.append( ParameterValueItem(*it, result ) );
	}
	m_modified = true;
	return true;
}

bool XParser::setFunctionExpression(const QString &f_str, uint id)
{
	int const ix = ixValue(id);
	if (ix==-1)
		return false;
	Ufkt *tmp_ufkt = &ufkt[ix];
	QString const old_fstr = tmp_ufkt->fstr;
	QString const fstr_begin = tmp_ufkt->fstr.left(tmp_ufkt->fstr.find('=')+1);
	tmp_ufkt->fstr = fstr_begin+f_str;
	reparse(tmp_ufkt);
	if ( parserError(false) != 0)
	{
		tmp_ufkt->fstr = old_fstr;
		reparse(tmp_ufkt);
		return false;
	}
	return true;
}

bool XParser::sendFunction(int id, const QString &dcopclient_target)
{
	QCStringList cstr_list = kapp->dcopClient()->registeredApplications();
	QStringList str_list;
	for ( QCStringList::iterator it = cstr_list.begin(); it!=cstr_list.end();++it )
		if ( QString(*it).startsWith("kmplot") && *it!=kapp->dcopClient()->appId() )
			str_list.append(*it);
	if ( str_list.isEmpty() )
	{
		KMessageBox::error(0, i18n("There are no other Kmplot instances running"));
		return false;
	}
	
	Ufkt *item = &ufkt[ixValue(id)];
	kdDebug() << "Transferring " << item->fname.latin1() << endl;
	QString str_result;
	if ( dcopclient_target.isEmpty() && item->fname.at(0) == 'y' )
	  	return false;
	else if ( dcopclient_target.isEmpty() )
	{
		bool ok;
		str_result = KInputDialog::getItem(i18n("kmplot"), i18n("Choose which KmPlot instance\nyou want to copy the function to:"), str_list, 0, false, &ok);
		if (!ok)
		  return false;
	}
	else
	  str_result = dcopclient_target;
	
	QByteArray parameters;
	QDataStream arg( parameters, IO_WriteOnly);
 
  QString str_dmin;
  if (!item->usecustomxmin)
    str_dmin = item->str_dmin;
  QString str_dmax;
  if (!item->usecustomxmax)
    str_dmax = item->str_dmax;
  
	QStringList str_parameters;
	for ( QValueList<ParameterValueItem>::Iterator it = item->parameters.begin(); it != item->parameters.end(); ++it )
	  	str_parameters.append( (*it).expression);
	arg << item->fstr << item->f_mode << item->f1_mode << item->f2_mode << item->integral_mode << item->integral_use_precision << item->linewidth << item->f1_linewidth << item->f2_linewidth << item->integral_linewidth << str_dmin << str_dmax << item->str_startx << item->str_starty << item->integral_precision << item->color << item->f1_color << item->f2_color << item->integral_color << str_parameters << item->use_slider;
	QByteArray replay_data;
	QCString replay_type;
	bool ok = kapp->dcopClient()->call( str_result.utf8(), "Parser", "addFunction(QString,bool,bool,bool,bool,bool,int,int,int,int,QString,QString,QString,QString,double,QRgb,QRgb,QRgb,QRgb,QStringList,int)", parameters, replay_type, replay_data, false);
	if (!ok)
	{
		KMessageBox::error(0, i18n("An error appeared during the transfer"));
		return false;
	}

	QDataStream replay_arg(replay_data, IO_ReadOnly);
	bool result;
	replay_arg >> result;
	if (!result)
	{
		KMessageBox::error(0, i18n("An error appeared during the transfer"));
		return false;
	}
	
	kapp->dcopClient()->send(str_result.utf8(), "View","drawPlot()",QByteArray() ); //update the other window
	
	if (item->fname.at(0) == 'x') // a parametric function
		return sendFunction(id+1, str_result);
	else
		return true;
}
