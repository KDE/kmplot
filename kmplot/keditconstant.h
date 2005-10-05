/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar
*                     f_edemar@linux.se
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

#ifndef KEDITCONSTANT_H
#define KEDITCONSTANT_H

#include "qeditconstant.h"
#include "xparser.h"

/**
@author Fredrik Edemar
*/
/// A small dialog for creating an editing constants
class KEditConstant : public QEditConstant
{
Q_OBJECT
public:
    KEditConstant(XParser *p, char &, QString &,QWidget *parent = 0, const char *name = 0);
    ~KEditConstant();
   
public slots:
    void cmdOK_clicked();
    void txtVariable_lostFocus();
    
signals:
    /// called when we are finished
    void finished();
        
private:
    char &constant;
    QString &value;
    XParser *m_parser;
    

};

#endif

