/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar <f_edemar@linux.se>
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

#include <klocale.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <qradiobutton.h>

#include "settings.h"
#include "xparser.h"

#include "coordsconfigdialog.h"
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
	configAxesDialog->layout()->setMargin( 0 );
	addPage(configAxesDialog , i18n( "Coords" ), "coords", i18n( "Edit Coordinate System" ) );
	setHelp("axes-config");
	setFaceType( Plain );
}

CoordsConfigDialog::~CoordsConfigDialog()
{
}

bool CoordsConfigDialog::evalX()
{
	Parser::Error error;
	
	double const min = XParser::self()->eval( configAxesDialog->kcfg_XMin->text(), & error );
	if ( error != Parser::ParseSuccess )
	{
		XParser::self()->displayErrorDialog( error );
		return false;
	}
	
	double const max = XParser::self()->eval( configAxesDialog->kcfg_XMax->text(), & error );
	if ( error != Parser::ParseSuccess )
	{
		XParser::self()->displayErrorDialog( error );
		return false;
	}
	
	if ( min >= max )
	{
		KMessageBox::sorry(this,i18n("The minimum range value must be lower than the maximum range value"));
		return false;
	}
	return true;
}

bool CoordsConfigDialog::evalY()
{
	Parser::Error error;
	
	double const min = XParser::self()->eval( configAxesDialog->kcfg_YMin->text(), & error );
	if ( error != Parser::ParseSuccess )
	{
		XParser::self()->displayErrorDialog( error );
		return false;
	}
	
	double const max = XParser::self()->eval( configAxesDialog->kcfg_YMax->text(), & error );
	if ( error != Parser::ParseSuccess )
	{
		XParser::self()->displayErrorDialog( error );
		return false;
	}
	
	if ( min >= max )
	{
		KMessageBox::sorry(this,i18n("The minimum range value must be lower than the maximum range value"));
		return false;
	}
	return true;
}

void CoordsConfigDialog::slotOk()
{
	// I've left this here as an example of poor naming (which radio button is which?!) :p
// 	if ( !(configAxesDialog->radioButton1_4->isChecked() && !evalX()) && !(configAxesDialog->radioButton1_4_2->isChecked() && !evalY()))
	
	if ( evalX() && evalY() )
        slotButtonClicked( KDialog::Ok );
}

void CoordsConfigDialog::slotApply()
{
	if ( evalX() && evalY() )
        slotButtonClicked( KDialog::Apply );
}

void CoordsConfigDialog::updateXYRange( )
{
	configAxesDialog->kcfg_XMin->setText( Settings::xMin() );
	configAxesDialog->kcfg_XMax->setText( Settings::xMax() );
	configAxesDialog->kcfg_YMin->setText( Settings::yMin() );
	configAxesDialog->kcfg_YMax->setText( Settings::yMax() );
}

#include "coordsconfigdialog.moc"
