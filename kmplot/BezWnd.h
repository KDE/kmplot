#ifndef BezWnd_included
#define BezWnd_included

#include "BezWndData.h"

class MainDlg;

class BezWnd : public BezWndData
{
    Q_OBJECT

public:
    BezWnd( QWidget* parent = NULL, const char* name = NULL );
    virtual ~BezWnd();

protected slots:
    void hideEvent( QHideEvent * );
private:
    MainDlg *main_dlg;
};
#endif // BezWnd_included
