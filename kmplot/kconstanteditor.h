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

#ifndef KCONSTANTEDITOR_H
#define KCONSTANTEDITOR_H

#include "qconstanteditor.h"
#include "keditconstant.h"
#include "View.h"

/**
@author Fredrik Edemar
*/
/// Handles all the constants.
class KConstantEditor : public QConstantEditor
{
Q_OBJECT
public:
    KConstantEditor(View *, QWidget *parent = 0, const char *name = 0);
    ~KConstantEditor();
    
public slots:
    /// actions for the buttons
    void cmdNew_clicked();
    void cmdEdit_clicked();
    void cmdDelete_clicked();
    void cmdDuplicate_clicked();
    
    ///actions for the visible constant list
    void varlist_clicked( QListViewItem * item );
    void varlist_doubleClicked( QListViewItem * );
   
    /// called when a user pressed OK the the new-constant-dialog
    void newConstantSlot();
    /// called when a user pressed OK the the edit-constant-dialog
    void editConstantSlot();
  
    
private:   
   char constant;
   QString value;
   View *m_view;
    
};

#endif
