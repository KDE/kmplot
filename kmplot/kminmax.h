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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#ifndef KMINMAX_H
#define KMINMAX_H

#include "qminmax.h"
#include "View.h"

/**
@author Fredrik Edemar
*/
class KMinMax : public QMinMax
{
Q_OBJECT
public:
    KMinMax(QWidget *parent = 0, const char *name = 0);
    KMinMax(View *, char, QWidget *parent = 0, const char *name = 0);

    ~KMinMax();
   
public slots:
    void cmdFind_clicked();
        
private:
    View *m_view;
    char m_mode; //0 = minimum, 1 = maximum, 2 = y-point
    

};

#endif
