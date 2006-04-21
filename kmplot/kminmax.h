/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar <f_edemar@linux.se>
*               2006  David Saxton <david@bluehaze.org>
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

#ifndef KMINMAX_H
#define KMINMAX_H

#include "ui_qminmax.h"
#include "View.h"

class QMinMax;

/**
@author Fredrik Edemar
*/
/// KMinMax handles all the dialogs for the items in the tool-menu.
class KMinMax : public KDialog
{
	Q_OBJECT
	public:
		KMinMax(QWidget *parent = 0 );
		
		enum Mode
		{
			FindMinimum,
			FindMaximum,
			CalculateY,
			CalculateArea
		};
		/// called every time the dialog is opened
		void init( Mode mode );
		
    /// update the list with functions
    void updateFunctions();
    /// select the right function when using the popup menu to show the dialog
    void selectItem();

    ~KMinMax();
   
public slots:
    /// the user has pressen the find/caluclate/draw button
    void cmdFind_clicked();
    /// the selecting a function that uses parameter function from a list the user can choose which paramater value he/she wants to use
    void cmdParameter_clicked();
    /// the button for changing the selected parameter value
	void list_currentChanged(QListWidgetItem*);
    /// call cmdParameter_clicked() if parameter values is enabled for that function
	void list_doubleClicked(QListWidgetItem *);
        
	private:
		Mode m_mode;
		Parameter parameter;
		QMinMax * m_mainWidget;
};

class QMinMax : public QWidget, public Ui::QMinMax
{
	public:
		QMinMax( QWidget * parent = 0 )
	: QWidget( parent )
		{ setupUi(this); }
};

#endif
