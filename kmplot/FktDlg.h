#ifndef FktDlg_included
#define FktDlg_included

// locale includes
#include "FktDlgData.h"
#include "AttrDlg.h"
#include "misc.h"
// Qt includes
#include <qvaluelist.h>
// KDE includes
#include <kapplication.h>
#include <klocale.h>

class FktDlg : public FktDlgData
{
    Q_OBJECT

public:
    FktDlg( QWidget* parent = NULL, const char* name = NULL );
    virtual ~FktDlg();

protected slots:
    void onok();
    void onclose();
    void ondelete();
    void onedit();
    void ondblclick( int );
    void onattr();
    void onapply();

private:
    int getIx( const QString f_str );
    void updateView();
    int chflg;
    int errflg;
};

#endif // FktDlg_included
