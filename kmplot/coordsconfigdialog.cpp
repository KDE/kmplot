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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include <klocale.h>
#include <klineedit.h>
#include <kmessagebox.h>

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

bool CoordsConfigDialog::eval()
{
	m_parser->eval( configAxesDialog->kcfg_XMin->text() );
	if ( m_parser->parserError()!=0 )
		return false;
	m_parser->eval( configAxesDialog->kcfg_XMax->text() );
	if ( m_parser->parserError()!=0 )
		return false;
	m_parser->eval( configAxesDialog->kcfg_YMin->text() );
	if ( m_parser->parserError()!=0 )
		return false;
	m_parser->eval( configAxesDialog->kcfg_YMax->text() );
	if ( m_parser->parserError()!=0 )
		return false;
	return true;
}

void CoordsConfigDialog::slotOk()
{
	if ( eval() )
		KConfigDialog::slotOk();
}

void CoordsConfigDialog::slotApply()
{
	if ( eval() )
		KConfigDialog::slotApply();
}

#include "coordsconfigdialog.moc"
