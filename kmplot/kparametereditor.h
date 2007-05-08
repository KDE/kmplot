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

#ifndef KPARAMETEREDITOR_H
#define KPARAMETEREDITOR_H

#include "ui_qparametereditor.h"
#include <QList>
#include "xparser.h"

#include <kdialog.h>

class ParameterValueList;
class QParameterEditor;

/**
@author Fredrik Edemar
*/
/// This class handles the parameter values: it can create, remove, edit and import values.
class KParameterEditor : public KDialog
{
Q_OBJECT
public:
	KParameterEditor( QList<Value> *, QWidget *parent );
    ~KParameterEditor();
    
public slots:
	void moveUp();
	void moveDown();
	void prev();
	void next();
    void cmdNew_clicked();
    void cmdDelete_clicked();
    void cmdImport_clicked();
    void cmdExport_clicked();
    
    ///actions for the visible constant list
	void selectedConstantChanged( QListWidgetItem * current );
	
	/// updates whether or not the "value is invalid" label is shown, (and returns the validity of the current value)
	bool checkValueValid();
	
	/// saves the value being edited
	void saveCurrentValue();
	
	virtual void accept();
  
    
private:
    /// Check so that it doesn't exist two equal values
    bool checkTwoOfIt( const QString & text);
    QList<Value> *m_parameter;
	QParameterEditor * m_mainWidget;
};

class QParameterEditor : public QWidget, public Ui::QParameterEditor
{
	public:
		QParameterEditor( QWidget * parent = 0 )
	: QWidget( parent )
		{ setupUi(this); }
};

#endif
