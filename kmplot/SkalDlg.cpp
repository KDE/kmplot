#include "SkalDlg.h"
#include "SkalDlg.moc"


#define Inherited SkalDlgData

SkalDlg::SkalDlg(QWidget* parent, const char* name, bool modal) : Inherited(parent, name, modal)
{	cb_xtlg->insertItem(tlgxstr, 0);
	cb_ytlg->insertItem(tlgystr, 0);
	cb_xdruck->insertItem(drskalxstr, 0);
	cb_ydruck->insertItem(drskalystr, 0);
}

SkalDlg::~SkalDlg()
{
}

int SkalDlg::wertholen(double& w, QComboBox *cb)
{	w=ps.eval(cb->currentText());
	if(w<=0.)
	{	if(ps.err!=0) ps.errmsg();
		else KMessageBox::error( this, i18n( "Wrong input:\n All terms must result values greater than 0."), "KmPlot" );
/*
		QMessageBox::warning(0, "kplot",
    		 i18n("Fehler in der Eingabe\nAlle Terme müssen Werte > 0 ergeben."));
*/
		errflg=1;
		return -1;
	}
	return 0;
}

// Slots

void SkalDlg::onok()
{	double dx, dy, tx, ty, mxmax, mymax;

	if(wertholen(tx, cb_xtlg)!=0) return;
    if(wertholen(ty, cb_ytlg)!=0) return;
	if(wertholen(dx, cb_xdruck)!=0) return;
    if(wertholen(dy, cb_ydruck)!=0) return;

	mxmax=(xmax-xmin)*dx/tx;
	mymax=(ymax-ymin)*dy/ty;
	if(mxmax>16. || mymax>16.)
	{	KMessageBox::error( this, i18n( "The plot is too big for paper size Din A4.\n"
			"Please correct the printing format."), "KmPlot" );
/*
	QMessageBox::warning(0, "kplot",
    		i18n("Das Diagramm ist zu groß für eine A4-Seite.\nDas Druckformat muß angepaßt werden."));
*/
		return;
	}
	
	tlgx=tx;    tlgxstr=cb_xtlg->currentText();
	tlgy=ty;    tlgystr=cb_ytlg->currentText();
    drskalx=dx; drskalxstr=cb_xdruck->currentText();
    drskaly=dy; drskalystr=cb_ydruck->currentText();
	
	if(cb_default->isChecked())
	{	kc->setGroup("Achsen");
		kc->writeEntry("tlgx", tlgxstr);
		kc->writeEntry("tlgy", tlgystr);
		kc->writeEntry("drskalx", drskalxstr);
		kc->writeEntry("drskaly", drskalystr);
		kc->sync();
	}

	errflg=0;
	done(1);
}

void SkalDlg::oncancel()
{	done(0);
}

void SkalDlg::onxtlgactivated(int ix)
{	cb_xdruck->setCurrentItem(ix);
}

void SkalDlg::onytlgactivated(int ix)
{	cb_ydruck->setCurrentItem(ix);
}
