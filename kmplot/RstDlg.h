#ifndef RstDlg_included
#define RstDlg_included

#include "RstDlgData.h"
#include "misc.h"
#include <kcolordlg.h>
#include <klocale.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>


class RstDlg : public RstDlgData
{
    Q_OBJECT

public:

    RstDlg(QWidget* parent=NULL, const char* name=NULL, bool modal=TRUE);
    virtual ~RstDlg();

	char mode;
	QColor farbe;
	

protected slots:
	
	void onok();
	void oncancel();
	void onfarbe();
virtual	void show();

};
#endif // RstDlg_included
