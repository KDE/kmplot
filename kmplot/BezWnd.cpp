//local includes
#include "BezWnd.h"
#include "BezWnd.moc"
#include "MainDlg.h"

#define Inherited BezWndData

BezWnd::BezWnd( QWidget* parent, const char* name ) : Inherited( 0, name )
{
    main_dlg = ( MainDlg * ) parent;
}

BezWnd::~BezWnd()
{
}

void BezWnd::hideEvent( QHideEvent * )
{
    main_dlg->view_bezeichnungen->setChecked( false );
}

