#ifndef KoordDlg_included
#define KoordDlg_included

#include "KoordDlgData.h"
#include "KOptDlg.h"
#include "misc.h"
#include "diagr.h"
#include <klocale.h>


class KoordDlg : public KoordDlgData
{
    Q_OBJECT

public:

    KoordDlg(QWidget* parent=NULL, const char* name=NULL, bool modal=TRUE);
    virtual ~KoordDlg();

private:

	void setachsen();
	int wertholen(double&, QLineEdit*);
	
	
	int errflg,
	    kdx,
	    kdy;
	    
	double mxmin,
	       mxmax,
	       mymin,
	       mymax;
	       
	QString mxminstr,
		mxmaxstr,
		myminstr,
		mymaxstr;


protected slots:
	
	void onok();
	void oncancel();
	void onoptions();
	void xclicked(int);
	void yclicked(int);
	void onXChanged();
	void onYChanged();
};

#endif // KoordDlg_included
