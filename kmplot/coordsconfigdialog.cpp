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

#include <klocale.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <qradiobutton.h>

#include "settings.h"
#include "settingspagecoords.h"
#include "coordsconfigdialog.h"

CoordsConfigDialog::CoordsConfigDialog(XParser *p, QWidget *parent)
	: KConfigDialog(parent, "coords", Settings::self()), m_parser(p)
{
	configAxesDialog = new SettingsPageCoords( 0, "coordsSettings" );
	addPage(configAxesDialog , i18n( "Coords" ), "coords", i18n( "Edit Coordinate System" ) );
	setHelp("axes-config");
}

CoordsConfigDialog::~CoordsConfigDialog()
{
}

bool CoordsConfigDialog::evalX()
{
	double const min = m_parser->eval( configAxesDialog->kcfg_XMin->text() );
	if ( m_parser->parserError()!=0 )
		return false;
	double const max = m_parser->eval( configAxesDialog->kcfg_XMax->text() );
	if ( m_parser->parserError()!=0 )
		return false;
	if ( min >= max )
	{
		KMessageBox::error(this,i18n("The minimum range value must be lower than the maximum range value"));
		return false;
	}
	return true;
}
bool CoordsConfigDialog::evalY()
{
	double const min = m_parser->eval( configAxesDialog->kcfg_YMin->text() );
	if ( m_parser->parserError()!=0 )
		return false;
	double const max = m_parser->eval( configAxesDialog->kcfg_YMax->text() );
	if ( m_parser->parserError()!=0 )
		return false;
	if ( min >= max )
	{
		KMessageBox::error(this,i18n("The minimum range value must be lower than the maximum range value"));
		return false;
	}
	return true;
}

void CoordsConfigDialog::slotOk()
{
	if ( !(configAxesDialog->radioButton1_4->isChecked() && !evalX()) && !(configAxesDialog->radioButton1_4_2->isChecked() && !evalY()))
		KConfigDialog::slotOk();
}

void CoordsConfigDialog::slotApply()
{
	if ( !(configAxesDialog->radioButton1_4->isChecked() && !evalX()) && !(configAxesDialog->radioButton1_4_2->isChecked() && !evalY()))
		KConfigDialog::slotApply();
}

#include "coordsconfigdialog.moc"
