/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2004 Fredrik Edemar <f_edemar@linux.se>
    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "kconstanteditor.h"
#include "kmplotio.h"
#include "view.h"
#include "xparser.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QStandardPaths>
#include <QTimer>

#include <assert.h>
#include <KConfigGroup>
#include <KMessageBox>


#include "ui_constantseditor.h"
class ConstantsEditorWidget : public QWidget, public Ui::ConstantsEditor
{
	public:
		ConstantsEditorWidget( QWidget * parent = 0 )
	: QWidget( parent )
		{ setupUi(this); }
};



//BEGIN class KConstantEditor
KConstantEditor::KConstantEditor( QWidget * parent )
	: QDialog( parent )
{
	m_widget = new ConstantsEditorWidget( this );

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &KConstantEditor::reject);
	QVBoxLayout *dialogLayout = new QVBoxLayout (this);
	dialogLayout->addWidget(m_widget);
	dialogLayout->addWidget(buttonBox);

	m_widget->cmdNew->setIcon( QIcon::fromTheme("document-new") );
	m_widget->cmdDelete->setIcon( QIcon::fromTheme("edit-delete") );
	
	setWindowTitle( i18nc("@title:window", "Constants Editor") );
    
	connect( this, &KConstantEditor::finished, this, &KConstantEditor::dialogFinished );
	
	m_constantValidator = new ConstantValidator( this );
	m_widget->nameEdit->setValidator( m_constantValidator );
	
	updateConstantsList();
	
	connect( m_widget->nameEdit, &KLineEdit::textEdited, this, &KConstantEditor::constantNameEdited );
	connect( m_widget->valueEdit, &EquationEdit::textEdited, this, &KConstantEditor::saveCurrentConstant );
	
	connect( m_widget->nameEdit, &KLineEdit::textChanged, this, &KConstantEditor::checkValueValid );
	connect( m_widget->valueEdit, &EquationEdit::textChanged, this, &KConstantEditor::checkValueValid );
	
	connect( m_widget->cmdNew, &QPushButton::clicked, this, &KConstantEditor::cmdNew_clicked );
	connect( m_widget->cmdDelete, &QPushButton::clicked, this, &KConstantEditor::cmdDelete_clicked );
	
	connect( m_widget->constantList, &QTreeWidget::currentItemChanged, this, &KConstantEditor::selectedConstantChanged );
	connect( m_widget->constantList, &QTreeWidget::itemClicked, this, &KConstantEditor::itemClicked );
	
	connect( XParser::self()->constants(), &Constants::constantsChanged, this, &KConstantEditor::updateConstantsList );
	
	checkValueValid();
}


KConstantEditor::~KConstantEditor()
{
}


void KConstantEditor::dialogFinished()
{
    XParser::self()->reparseAllFunctions();
    View::self()->drawPlot();
}


void KConstantEditor::updateConstantsList( )
{
	m_widget->constantList->blockSignals( true );
	
	// This assumes that constants have only been added or their value changed.
	// (since constants can only be removed via this dialog)
	
	ConstantList constants = XParser::self()->constants()->list( Constant::All );
	for ( ConstantList::iterator it = constants.begin(); it != constants.end(); ++it )
	{
		QList<QTreeWidgetItem *> list = m_widget->constantList->findItems( it.key(), Qt::MatchExactly );
		if ( !list.isEmpty() )
			init( list.first(), it.key(), it.value() );
		else
		{
			QTreeWidgetItem * item = new QTreeWidgetItem( m_widget->constantList );
			init( item, it.key(), it.value() );
		}
	}
	
	m_widget->constantList->blockSignals( false );
}


void KConstantEditor::init( QTreeWidgetItem * item, const QString & name, const Constant & constant )
{
	item->setText( 0, name );
	item->setText( 1, constant.value.expression() );
	item->setData( 2, Qt::CheckStateRole, (constant.type & Constant::Document) ? Qt::Checked : Qt::Unchecked );
	item->setData( 2, Qt::ToolTipRole, i18n("Check this to have the constant exported when saving.") );
// 	item->setData( 2, Qt::WhatsThisRole, i18n("Document constants are saved with the documents, and will be loaded again when the document is opened.") );
	item->setData( 3, Qt::CheckStateRole, (constant.type & Constant::Global) ? Qt::Checked : Qt::Unchecked );
	item->setData( 3, Qt::ToolTipRole, i18n("Check this to have the constant permanently available between instances of KmPlot.") );
// 	item->setData( 3, Qt::WhatsThisRole, i18n("Global constants are stored in KmPlot's settings. They are not lost when KmPlot is closed.") );
}


void KConstantEditor::cmdNew_clicked()
{
	QTreeWidgetItem * item = new QTreeWidgetItem( m_widget->constantList );
	
	init( item, XParser::self()->constants()->generateUniqueName(), Constant() );
	
	m_widget->constantList->setCurrentItem( item );
	m_widget->nameEdit->setFocus();
}


void KConstantEditor::cmdDelete_clicked()
{
	QTreeWidgetItem * item = m_widget->constantList->currentItem();
	if ( !item )
		return;
	
	XParser::self()->constants()->remove( item->text(0) );
	
	m_widget->nameEdit->clear();
	m_widget->valueEdit->clear();
	m_widget->constantList->takeTopLevelItem( m_widget->constantList->indexOfTopLevelItem( item ) );
	delete item;
	
	m_widget->cmdDelete->setEnabled( m_widget->constantList->currentItem() != 0 );
}


void KConstantEditor::selectedConstantChanged( QTreeWidgetItem * current )
{
	m_widget->cmdDelete->setEnabled( current != 0 );
	
	QString name = current ? current->text(0) : QString();
	QString value = current ? current->text(1) : QString();
	
	m_previousConstantName = name;
	m_constantValidator->setWorkingName( m_previousConstantName );
	
	m_widget->nameEdit->setText( name );
	m_widget->valueEdit->setText( value );
}


void KConstantEditor::constantNameEdited( const QString & newName )
{
	QTreeWidgetItem * current = m_widget->constantList->currentItem();
	if ( !current )
	{
		Constant constant;
		constant.value.updateExpression( m_widget->valueEdit->text() );
		
		current = new QTreeWidgetItem( m_widget->constantList );
		init( current, newName, constant );
	}
	
	XParser::self()->constants()->remove( m_previousConstantName );
	
	current->setText( 0, newName );
	m_widget->constantList->setCurrentItem( current ); // make it the current item if no item was selected before
	
	m_previousConstantName = newName;
	
	m_constantValidator->setWorkingName( m_previousConstantName );
	
	saveCurrentConstant();
}


void KConstantEditor::saveCurrentConstant()
{
	if ( m_widget->nameEdit->text().isEmpty() )
		return;
	
	QTreeWidgetItem * current = m_widget->constantList->currentItem();
	assert( current );
	current->setText( 1, m_widget->valueEdit->text() );
	
	Constant constant;
	constant.value.updateExpression( m_widget->valueEdit->text() );
	
	// update type
	constant.type = 0;
	if ( current->data( 2, Qt::CheckStateRole ).toBool() )
		constant.type |= Constant::Document;
	if ( current->data( 3, Qt::CheckStateRole ).toBool() )
		constant.type |= Constant::Global;
	
	XParser::self()->constants()->add( m_widget->nameEdit->text(), constant );
}


bool KConstantEditor::checkValueValid()
{
	Parser::Error error;
	(double) XParser::self()->eval( m_widget->valueEdit->text(), & error );
	bool valid = (error == Parser::ParseSuccess) && m_constantValidator->isValid( m_widget->nameEdit->text() );
	m_widget->valueInvalidLabel->setVisible( !valid );
	return valid;
}


void KConstantEditor::itemClicked()
{
	QTimer::singleShot( 0, this, &KConstantEditor::saveCurrentConstant );
}
//END class KConstantEditor



//BEGIN class ConstantValidator
ConstantValidator::ConstantValidator( KConstantEditor * parent )
	: QValidator( parent )
{
}


bool ConstantValidator::isValid( const QString & name ) const
{
	bool correct = XParser::self()->constants()->isValidName( name );
	bool inUse = XParser::self()->constants()->have( name ) && (m_workingName != name);
	
	return correct && !inUse;
}


QValidator::State ConstantValidator::validate( QString & input, int & /*pos*/ ) const
{
	return isValid( input ) ? Acceptable : Intermediate;
}


void ConstantValidator::setWorkingName( const QString & name )
{
	m_workingName = name;
}
//END class ConstantValidator
