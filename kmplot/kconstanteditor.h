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

#include "kdialog.h"
#include <QValidator>

class Constant;
class ConstantValidator;
class ConstantsEditorWidget;
class QTreeWidgetItem;

/**
 * Handles all the constants.
 * @author Fredrik Edemar, David Saxton
*/
class KConstantEditor : public KDialog
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
	
	/**
	 * Updates the list of constants.
	 */
	void updateConstantsList();
	
protected slots:
	/**
	 * Called when an item is clicked on. This might mean that the check state
	 * has changed, so will save the constants list.
	 */
	void itemClicked();
	
protected:
	/**
	 * Initializes the values, checkstates, tooltips, etc.
	 */
	void init( QTreeWidgetItem * item, const QString & name, const Constant & constant );
	
private:
	QString m_previousConstantName;
	ConstantValidator * m_constantValidator;
	ConstantsEditorWidget * m_widget;
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
		
		bool isValid( const QString & name ) const;
		
		/**
		 * There cannot be more than one constant with the same name. So
		 * this validator checks that the input does not conflict with any
		 * existing names - except, of course, the name of the constant being
		 * edited.
		 */
		void setWorkingName( const QString & name );
		
	protected:
		/// @see setWorkingName
		QString m_workingName;
};

#endif
