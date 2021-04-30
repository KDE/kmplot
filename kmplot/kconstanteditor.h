/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2004 Fredrik Edemar <f_edemar@linux.se>
    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef KCONSTANTEDITOR_H
#define KCONSTANTEDITOR_H

#include <QDialog>
#include <QValidator>

class Constant;
class ConstantValidator;
class ConstantsEditorWidget;
class QTreeWidgetItem;

/**
 * Handles all the constants.
 * @author Fredrik Edemar, David Saxton
*/
class KConstantEditor : public QDialog
{
Q_OBJECT
public:
    explicit KConstantEditor(QWidget *parent = 0);
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
    
    void dialogFinished();
	
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
		explicit ConstantValidator( KConstantEditor * parent );
		
		State validate( QString & input, int & pos ) const Q_DECL_OVERRIDE;
		
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
