#include "SwDlg.h"
#include "SwDlg.moc"
#include "kmessagebox.h"

#define Inherited SwDlgData


SwDlg::SwDlg(QWidget* parent, const char* name, bool modal) : Inherited(parent, name, modal)
{	QString str;

	//setCaption(i18n("Schrittweite einstellen"));
	le_sw->setText(str.setNum(rsw));
	le_sw->setFocus();
}

SwDlg::~SwDlg()
{
}

// Slots

void SwDlg::onok()
{	double w;
	QString str;

	str=le_sw->text();
	w=str.toDouble(); printf("%g\n", w);
	if(w>0. && w<=20.) {rsw=w; done(1);}
	else KMessageBox::error( this, i18n("Wrong input:\n"
		"Allowed range is ] 0; 20 ]."), "KmPlot");
}

void SwDlg::oncancel()
{	done(0);
}
