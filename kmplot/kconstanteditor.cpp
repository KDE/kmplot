/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar
*                     f_edemar@linux.se
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

#include <kapplication.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include <qpushbutton.h>
#include <qstringlist.h>
#include <QVector>
#include <qdom.h>
#include <qtreewidget.h>

#include "kmplotio.h"
#include "kconstanteditor.h"

#include <assert.h>


//BEGIN class KConstantEditor
KConstantEditor::KConstantEditor( QWidget * parent )
	: QWidget( parent )
{
	m_previousConstantName = 0;
	setupUi( this );
	m_constantValidator = new ConstantValidator( this );
	nameEdit->setValidator( m_constantValidator );
	
	QVector<Constant> constants = XParser::self()->constants()->all();
	foreach ( Constant c, constants )
	{
		QString valueAsString;
		valueAsString.setNum( c.value, 'g', 8 );
		QTreeWidgetItem * item = new QTreeWidgetItem( constantList );
		item->setText( 0, QString( c.constant ) );
		item->setText( 1, valueAsString );
	}
	
	connect( nameEdit, SIGNAL( textEdited( const QString & ) ), this, SLOT( constantNameEdited( const QString & ) ) );
	connect( valueEdit, SIGNAL( textEdited( const QString & ) ), this, SLOT( saveCurrentConstant() ) );
	connect( valueEdit, SIGNAL( textChanged( const QString & ) ), this, SLOT( checkValueValid() ) );
	
	connect( cmdNew, SIGNAL( clicked() ), this, SLOT( cmdNew_clicked() ) );
	connect( cmdDelete, SIGNAL( clicked() ), this, SLOT( cmdDelete_clicked() ) );
	
	connect( constantList, SIGNAL(currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * )), this, SLOT(selectedConstantChanged( QTreeWidgetItem * )) );
	
	checkValueValid();
}


KConstantEditor::~KConstantEditor()
{
}


void KConstantEditor::cmdNew_clicked()
{
	QTreeWidgetItem * item = new QTreeWidgetItem( constantList );
	constantList->setCurrentItem( item );
	nameEdit->setFocus();
}


void KConstantEditor::cmdDelete_clicked()
{
	QTreeWidgetItem * item = constantList->currentItem();
	if ( !item )
		return;
	
	QString constantText = item->text(0);
	if ( !constantText.isEmpty() )
	{
		QChar currentConstant = constantText[0];
	
// 		for( QVector<Function>::iterator it = XParser::self()->ufkt.begin(); it !=  XParser::self()->ufkt.end(); ++it)
		foreach ( Function * it, XParser::self()->m_ufkt )
		{
			QString str = it->eq[0]->fstr();
			if ( str.indexOf( currentConstant, str.indexOf(')') ) != -1 )
			{
				KMessageBox::sorry(this, i18n("A function uses this constant; therefore, it cannot be removed."));
				return;
			}
		}
	
		XParser::self()->constants()->remove( currentConstant );
	}
	
	nameEdit->clear();
	valueEdit->clear();
	constantList->takeTopLevelItem( constantList->indexOfTopLevelItem( item ) );
	delete item;
	
	cmdDelete->setEnabled( constantList->currentItem() != 0 );
}


void KConstantEditor::selectedConstantChanged( QTreeWidgetItem * current )
{
	cmdDelete->setEnabled( current != 0 );
	
	QString name = current ? current->text(0) : QString::null;
	QString value = current ? current->text(1) : QString::null;
	
	m_previousConstantName = name.isEmpty() ? QChar(0) : name[0];
	m_constantValidator->setWorkingName( m_previousConstantName );
	
	nameEdit->setText( name );
	valueEdit->setText( value );
}


void KConstantEditor::constantNameEdited( const QString & newName )
{
	QTreeWidgetItem * current = constantList->currentItem();
	if ( !current )
		current = new QTreeWidgetItem( constantList );
	
	XParser::self()->constants()->remove( m_previousConstantName );
	
	current->setText( 0, newName );
	constantList->setCurrentItem( current ); // make it the current item if no item was selected before
	
	if ( newName.isEmpty() )
		m_previousConstantName = 0;
	else
		m_previousConstantName = newName[0];
	
	m_constantValidator->setWorkingName( m_previousConstantName );
	
	saveCurrentConstant();
}


void KConstantEditor::saveCurrentConstant()
{
	if ( nameEdit->text().isEmpty() )
		return;
	
	QTreeWidgetItem * current = constantList->currentItem();
	assert( current );
	current->setText( 1, valueEdit->text() );
	
	QChar currentName = nameEdit->text()[0];
	double value = XParser::self()->eval( valueEdit->text() );
	XParser::self()->constants()->add( Constant( currentName, value ) );
}


bool KConstantEditor::checkValueValid()
{
	(double) XParser::self()->eval( valueEdit->text() );
	bool valid = (XParser::self()->parserError( false ) == 0);
	valueInvalidLabel->setVisible( !nameEdit->text().isEmpty() && !valid );
	return valid;
}
//END class KConstantEditor



//BEGIN class ConstantValidator
ConstantValidator::ConstantValidator( KConstantEditor * parent )
	: QValidator( parent )
{
	m_workingName = 0;
}


QValidator::State ConstantValidator::validate( QString & input, int & pos ) const
{
	(void)pos;
	
	// The input should already be limited to a maximum length of 1
	assert( input.length() < 2 );
	if ( input.isEmpty() )
		return Intermediate;
	
	kDebug() << "input[0]="<<input[0]<<"input[0].category()="<<input[0].category()<<endl;
	
	// note: don't want to make greek letters uppercase
	if ( (input[0].unicode() >= 'a') && (input[0].unicode() <= 'z') )
		input = input.toUpper();
	
	// Is the constant name already in use?
	if ( (input[0] != m_workingName) && XParser::self()->constants()->have( input[0] ) )
		return Invalid;
	
	return XParser::self()->constants()->isValidName( input[0] ) ? Acceptable : Invalid;
}


void ConstantValidator::setWorkingName( QChar name )
{
	m_workingName = name;
}
//END class ConstantValidator

#include "kconstanteditor.moc"
