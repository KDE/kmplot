#ifndef SkalDlg_included
#define SkalDlg_included

#include "SkalDlgData.h"
#include <kapp.h>
#include <klocale.h>
#include "kmessagebox.h"
#include "misc.h"
#include <qcheckbox.h>
#include <qcombobox.h>


class SkalDlg : public SkalDlgData
{
    Q_OBJECT

public:

    SkalDlg(QWidget* parent=NULL, const char* name=NULL, bool modal=TRUE);
    virtual ~SkalDlg();

private:

	int wertholen(double&, QComboBox*);
	
	int errflg;
	
	
protected slots:
	
	void onok();
	void oncancel();
    void onxtlgactivated(int);
    void onytlgactivated(int);

};

#endif // SkalDlg_included
