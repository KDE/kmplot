#ifndef AttrDlg_included
#define AttrDlg_included

#include "AttrDlgData.h"
#include <kapp.h>
#include "misc.h"
#include <kcolordlg.h>
#include <klocale.h>
#include <qcheckbox.h>
#include <qlineedit.h>


class AttrDlg : public AttrDlgData
{
    Q_OBJECT


public:

    AttrDlg(QWidget* parent=NULL, const char* name=NULL,  bool modal=TRUE);
    virtual ~AttrDlg();

	int ix;
	

private:

	QColor farbe;


protected slots:
	
	void onok();
	void oncancel();
	void onfarbe();
virtual	void show();
	

};
#endif // AttrDlg_included
