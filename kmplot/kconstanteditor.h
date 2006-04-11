/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar <f_edemar@linux.se>
*               2006 David Saxton <david@bluehaze.org>
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

#include "settingspageconstants.h"
#include "View.h"

#include <QValidator>

class ConstantValidator;

/**
@author Fredrik Edemar, David Saxton
*/
/// Handles all the constants.
class KConstantEditor : public QWidget, public Ui::SettingsPageConstants
{
Q_OBJECT
public:
    KConstantEditor(QWidget *parent = 0);
    ~KConstantEditor();
    
public slots:
    /// actions for the buttons
    void cmdNew_clicked();
    void cmdDelete_clicked();
	
	/// called when the user changes the text in the Constant name edit box
	void constantNameEdited( const QString & newName );
    
    ///actions for the visible constant list
	void selectedConstantChanged( QTreeWidgetItem * current );
	
	/// saves the value in the edit boxes of the constant currently being edited
	void saveCurrentConstant();
	
	/// updates whether or not the "value is invalid" label is shown, (and returns the validity of the current value)
	bool checkValueValid();
	
private:
	QChar m_previousConstantName;
	ConstantValidator * m_constantValidator;
};


/**
Validates the constant; ensuring that Roman letters are alphabetical and only
proper constant letters are used.
@author David Saxton
*/
class ConstantValidator : public QValidator
{
	public:
		ConstantValidator( KConstantEditor * parent );
		
		virtual State validate( QString & input, int & pos ) const;
		
		/**
		 * There cannot be more than one constant with the same name. So
		 * this validator checks that the input does not conflict with any
		 * existing names - except, of course, the name of the constant being
		 * edited.
		 */
		void setWorkingName( QChar name );
		
	protected:
		/// @see setWorkingName
		QChar m_workingName;
};

#endif
