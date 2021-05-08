/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2004 Fredrik Edemar <f_edemar@linux.se>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "coordsconfigdialog.h"

#include <KMessageBox>

#include <QPushButton>

#include "settings.h"
#include "xparser.h"

#include "ui_editcoords.h"

class EditCoords : public QWidget, public Ui::EditCoords
{
 public:
     EditCoords( QWidget * parent = 0 )
 : QWidget( parent )
     { setupUi(this); }
};

CoordsConfigDialog::CoordsConfigDialog(QWidget *parent)
	: KConfigDialog(parent, "coords", Settings::self())
{
	configAxesDialog = new EditCoords( 0 );
	configAxesDialog->kcfg_XMin->setTabChain( configAxesDialog->kcfg_XMax->focusProxy() );
	configAxesDialog->kcfg_XMax->setTabChain( configAxesDialog->kcfg_YMin->focusProxy() );
	configAxesDialog->kcfg_YMin->setTabChain( configAxesDialog->kcfg_YMax->focusProxy() );
	configAxesDialog->layout()->setContentsMargins( 0, 0, 0, 0 );
	addPage(configAxesDialog , i18n( "Coordinates" ), "coords", i18n( "Coordinate System" ) );
	setWindowTitle( i18nc("@title:window", "Coordinate System") );
	setHelp(QString::fromLatin1("axes-config"));
	setFaceType( Plain );
	connect( configAxesDialog->kcfg_XMin, &EquationEdit::textEdited, this, &CoordsConfigDialog::updateButtons );
	connect( configAxesDialog->kcfg_XMax, &EquationEdit::textEdited, this, &CoordsConfigDialog::updateButtons );
	connect( configAxesDialog->kcfg_YMin, &EquationEdit::textEdited, this, &CoordsConfigDialog::updateButtons );
	connect( configAxesDialog->kcfg_YMax, &EquationEdit::textEdited, this, &CoordsConfigDialog::updateButtons );
}

CoordsConfigDialog::~CoordsConfigDialog()
{
}

bool CoordsConfigDialog::evalX(bool showError)
{
	Parser::Error error;
	
	double const min = XParser::self()->eval( configAxesDialog->kcfg_XMin->text(), & error );
	if ( error != Parser::ParseSuccess )
	{
		if ( showError ) XParser::self()->displayErrorDialog( error );
		return false;
	}
	
	double const max = XParser::self()->eval( configAxesDialog->kcfg_XMax->text(), & error );
	if ( error != Parser::ParseSuccess )
	{
		if ( showError ) XParser::self()->displayErrorDialog( error );
		return false;
	}
	
	if ( min >= max )
	{
		if ( showError ) KMessageBox::sorry(this,i18n("The minimum range value must be lower than the maximum range value"));
		return false;
	}
	return true;
}

bool CoordsConfigDialog::evalY(bool showError)
{
	Parser::Error error;
	
	double const min = XParser::self()->eval( configAxesDialog->kcfg_YMin->text(), & error );
	if ( error != Parser::ParseSuccess )
	{
		if ( showError ) XParser::self()->displayErrorDialog( error );
		return false;
	}
	
	double const max = XParser::self()->eval( configAxesDialog->kcfg_YMax->text(), & error );
	if ( error != Parser::ParseSuccess )
	{
		if ( showError ) XParser::self()->displayErrorDialog( error );
		return false;
	}
	
	if ( min >= max )
	{
		if ( showError ) KMessageBox::sorry(this,i18n("The minimum range value must be lower than the maximum range value"));
		return false;
	}
	return true;
}

void CoordsConfigDialog::updateButtons()
{
	buttonBox()->button( QDialogButtonBox::Apply )->setEnabled( evalX(false) && evalY(false) );
}

void CoordsConfigDialog::done(int result)
{
	// https://www.qtcentre.org/threads/8048-Validate-Data-in-QDialog
	if ( result == QDialog::Accepted ) {
		if ( !evalX() || !evalY() ) {
			return;
		}
	}
	KConfigDialog::done( result );
}

void CoordsConfigDialog::updateXYRange( )
{
	configAxesDialog->kcfg_XMin->setText( Settings::xMin() );
	configAxesDialog->kcfg_XMax->setText( Settings::xMax() );
	configAxesDialog->kcfg_YMin->setText( Settings::yMin() );
	configAxesDialog->kcfg_YMax->setText( Settings::yMax() );
}
