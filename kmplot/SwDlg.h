#ifndef SwDlg_included
#define SwDlg_included

#include "SwDlgData.h"
#include <kapp.h>
#include <klocale.h>
#include "misc.h"
#include <qlineedit.h>


class SwDlg : public SwDlgData
{
    Q_OBJECT

public:

    SwDlg(QWidget* parent=NULL, const char* name=NULL, bool modal=TRUE);
    virtual ~SwDlg();

	
protected slots:
	
	void onok();
	void oncancel();
	
};

#endif // SwDlg_included
