#include "KoordDlg.h"
#include "KoordDlg.moc"
#include "kmessagebox.h"

#define Inherited KoordDlgData

KoordDlg::KoordDlg(QWidget* parent, const char* name, bool modal) : Inherited(parent, name, modal)
{	kdx=koordx; kdy=koordy;
	setachsen();
	le_xmin->setText(xminstr);
	le_xmax->setText(xmaxstr);
	le_ymin->setText(yminstr);
	le_ymax->setText(ymaxstr);
	cb_beschr->setChecked(mode&BESCHRIFTUNG);
}

KoordDlg::~KoordDlg()
{
}

void KoordDlg::setachsen()
{	switch(kdx)
	{ case 0:	mxminstr="-8"; mxmaxstr="8";
				rb_x1->setChecked(TRUE); break;
      case 1:	mxminstr="-5"; mxmaxstr="5";
      	  		rb_x2->setChecked(TRUE); break;
      case 2:	mxminstr="0";  mxmaxstr="16";
     	  		rb_x3->setChecked(TRUE); break;
      case 3:	mxminstr="0";  mxmaxstr="10";
      	  		rb_x4->setChecked(TRUE);
    }
    switch(kdy)
	{ case 0:	myminstr="-8"; mymaxstr="8";
				rb_y1->setChecked(TRUE); break;
      case 1:	myminstr="-5"; mymaxstr="5";
      	  		rb_y2->setChecked(TRUE); break;
      case 2:	myminstr="0";  mymaxstr="16";
      	  		rb_y3->setChecked(TRUE); break;
      case 3:	myminstr="0";  mymaxstr="10";
      	  		rb_y4->setChecked(TRUE); 
    }
	le_xmin->setText(mxminstr);
	le_xmax->setText(mxmaxstr);
	le_ymin->setText(myminstr);
	le_ymax->setText(mymaxstr);
}

int KoordDlg::wertholen(double& w, QLineEdit *le)
{	w=ps.eval(le->text());
	if(ps.err!=0)
	{	ps.errmsg();
		errflg=1;
		return -1;
	}
	return 0;
}

// Slots

void KoordDlg::onok()
{	int m;

	koordx=kdx; koordy=kdy;
	if(wertholen(mxmin, le_xmin)!=0) return;
    if(wertholen(mxmax, le_xmax)!=0) return;
    if(mxmin>=mxmax)
    {	KMessageBox::error( this, i18n( "Wrong input:\nxmin > xmax"), "KmPlot" );
    	return;
    }

	if(wertholen(mymin, le_ymin)!=0) return;
    if(wertholen(mymax, le_ymax)!=0) return;
    if(mymin>=mymax)
    {	KMessageBox::error( this, i18n( "Wrong input:\nymin > ymax"), "KmPlot" );
    	return;
    }

	xmin=mxmin; xmax=mxmax;
	ymin=mymin; ymax=mymax;
	xminstr=le_xmin->text(); xmaxstr=le_xmax->text();
	yminstr=le_ymin->text(); ymaxstr=le_ymax->text();
	errflg=0;
	if(cb_beschr->isChecked())
	{	m=1;
		mode|=BESCHRIFTUNG;
	}
	else
	{	m=0;
		mode&=~BESCHRIFTUNG;
	}
	
	if(cb_default->isChecked())
	{	kc->setGroup("Achsen");
		kc->writeEntry("xmin", xminstr);
		kc->writeEntry("xmax", xmaxstr);
		kc->writeEntry("ymin", yminstr);
		kc->writeEntry("ymax", ymaxstr);
		kc->writeEntry("Beschriftung", m);
		kc->sync();
	}

	done(1);
}

void KoordDlg::oncancel()
{	done(0);
}

void KoordDlg::onoptions()
{	KOptDlg odlg;

	odlg.exec();
}

void KoordDlg::xclicked(int ix)
{	kdx=ix;
	setachsen();
}

void KoordDlg::yclicked(int iy)
{	kdy=iy;
	setachsen();
}

