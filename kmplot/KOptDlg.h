#ifndef KOptDlg_included
#define KOptDlg_included

#include "KOptDlgData.h"
#include "misc.h"
#include <kcolordlg.h>
#include <qcheckbox.h>
#include <qlineedit.h>


class KOptDlg : public KOptDlgData
{
    Q_OBJECT

public:

    KOptDlg(QWidget* parent=NULL, const char* name=NULL, bool modal=TRUE);
    virtual ~KOptDlg();

private:

	int ad,
	    td,
	    tl;
	QColor farbe;
	
protected slots:
	
	void onok();
	void oncancel();
	void onfarbe();
};

#endif // KOptDlg_included
