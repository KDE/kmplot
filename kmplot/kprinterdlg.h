/***************************************************************************

    kmplot/kprinterdlg.h  -  PrintDialogPage for kmplot priter pptions
                             -------------------
    begin                : 2002-06-21
    email                : bmlmessmer@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef kprintdlg_included
#define kprintdlg_included

// Qt includes
#include <qcheckbox.h>

// KDE includes
#include <kdeprint/kprintdialogpage.h>

class KPrinterDlg : public KPrintDialogPage
{
	Q_OBJECT
public:
    KPrinterDlg( QWidget *parent = 0, const char *name = 0 );

    //reimplement virtual functions
    void getOptions( QMap<QString,QString>& opts, bool include_def = false );
    void setOptions( const QMap<QString,QString>& opts );
    bool isValid( QString& msg );

private:
    QCheckBox *m_printtable;
};

#endif //kprinterdlg_included
