// Qt includes
#include <qlayout.h>

// KDE includes
#include <kdialog.h>
#include <klocale.h>

// local includes
#include "kprinterdlg.h"

KPrinterDlg::KPrinterDlg( QWidget *parent, const char *name )
        : KPrintDialogPage( parent, name )
{
    setTitle( i18n( "KmPlot Options" ) );
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( KDialog::marginHint() );
    layout->setSpacing( KDialog::spacingHint() );


	m_printtable = new QCheckBox( i18n( "print header table" ), this );
    m_printtable->setChecked( true );
    layout->addWidget( m_printtable );
	layout->addStretch( 1 );
}

void KPrinterDlg::getOptions( QMap<QString,QString>& opts, bool include_def )
{
    if ( m_printtable->isChecked() || include_def )
        opts[ "app-kmplot-printtable" ] = ( m_printtable->isChecked() ? "1" : "-1" );
}

void KPrinterDlg::setOptions( const QMap<QString,QString>& opts )
{
    m_printtable->setChecked( opts[ "app-kmplot-printtable" ] == "1" );
}

bool KPrinterDlg::isValid( QString& msg )
{
    return true;
}


