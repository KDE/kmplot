// local includes
#include "KoordDlg.h"
#include "KoordDlg.moc"

// Qt includes
#include <qbuttongroup.h>

// KDE includes
#include <kdebug.h>
#include <kmessagebox.h>

#define Inherited KoordDlgData

KoordDlg::KoordDlg( QWidget* parent, const char* name, bool modal ) : Inherited( parent, name, modal )
{
    le_xmin->setText( xminstr );
    le_xmax->setText( xmaxstr );
    le_ymin->setText( yminstr );
    le_ymax->setText( ymaxstr );
    kdx = koordx;
    kdy = koordy;
    setachsen();
    cb_beschr->setChecked( mode & BESCHRIFTUNG );
}

KoordDlg::~KoordDlg()
{
}

void KoordDlg::setachsen()
{
    bg_xachse->setButton( kdx );
    bg_yachse->setButton( kdy );
}

int KoordDlg::wertholen( double& w, QLineEdit *le )
{
    w = ps.eval( le->text() );
    if ( ps.err != 0 )
    {
        ps.errmsg();
        errflg = 1;
        return -1;
    }
    return 0;
}

// Slots

void KoordDlg::onok()
{
    int m;

    koordx = kdx;
    switch ( kdx )
    {
    case 0:
        mxmin = -8.0;
        mxmax = 8.0;
		break;
    case 1:
        mxmin = -5.0;
        mxmax = 5.0;
		break;
    case 2:
        mxmin = 0.0;
        mxmax = 16.0;
		break;
    case 3:
        mxmin = 0.0;
        mxmax = 10.0;
		break;
    case 4:
        if ( wertholen( mxmin, le_xmin ) != 0 )
            return ;
        if ( wertholen( mxmax, le_xmax ) != 0 )
            return ;
        if ( mxmin >= mxmax )
        {
            KMessageBox::error( this, i18n( "Wrong input:\nxmin > xmax" ), "KmPlot" );
            return ;
        }
    }
    koordy = kdy;
    switch ( kdy )
    {
    case 0:
        mymin = -8.0;
        mymax = 8.0;
		break;
    case 1:
        mymin = -5.0;
        mymax = 5.0;
		break;
    case 2:
        mymin = 0.0;
        mymax = 16.0;
		break;
    case 3:
        mymin = 0.0;
        mymax = 10.0;
		break;
    case 4:
        if ( wertholen( mymin, le_ymin ) != 0 )
            return ;
        if ( wertholen( mymax, le_ymax ) != 0 )
            return ;
        if ( mymin >= mymax )
        {
            KMessageBox::error( this, i18n( "Wrong input:\nymin > ymax" ), "KmPlot" );
            return ;
        }
    }

    xmin = mxmin;
    xmax = mxmax;
    ymin = mymin;
    ymax = mymax;
    xminstr = le_xmin->text();
    xmaxstr = le_xmax->text();
    yminstr = le_ymin->text();
    ymaxstr = le_ymax->text();
    errflg = 0;
    if ( cb_beschr->isChecked() )
    {
        m = 1;
        mode |= BESCHRIFTUNG;
    }
    else
    {
        m = 0;
        mode &= ~BESCHRIFTUNG;
    }

    if ( cb_default->isChecked() )
    {
        kc->setGroup( "Axes" );

		kc->writeEntry( "XCoord", koordx );
		kc->writeEntry( "YCoord", koordy );
        kc->writeEntry( "Xmin", xminstr );
        kc->writeEntry( "Xmax", xmaxstr );
        kc->writeEntry( "Ymin", yminstr );
        kc->writeEntry( "Ymax", ymaxstr );
        kc->writeEntry( "Labeled", m == 1 );
        kc->sync();
    }
    done( 1 );
}

void KoordDlg::oncancel()
{
    done( 0 );
}

void KoordDlg::onoptions()
{
    KOptDlg odlg;

    odlg.exec();
}

void KoordDlg::xclicked( int ix )
{
    kdx = ix;
}

void KoordDlg::yclicked( int iy )
{
    kdy = iy;
}

void KoordDlg::onXChanged()
{
    kdx = 4;
	setachsen();
}

void KoordDlg::onYChanged()
{
    kdy = 4;
	setachsen();
}
