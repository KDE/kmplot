#ifndef FktDlg_included
#define FktDlg_included

#include "FktDlgData.h"
#include "AttrDlg.h"
#include <kapplication.h>
#include "misc.h"
#include <qlist.h>
#include <klocale.h>


class FktDlg : public FktDlgData
{
    Q_OBJECT

protected slots:
	
	void onok();
	void oncancel();
	void ondelete();
	void onedit();
	void ondblclick(int);
	void onattr();

public:

    FktDlg(QWidget* parent=NULL, const char* name=NULL, bool modal=TRUE);
    virtual ~FktDlg();

private:

	int chflg, errflg, ix;
	QList <int>fktidx;

};
#endif // FktDlg_included
