// local includes
#include "KOptDlg.h"
#include "KOptDlg.moc"


#define Inherited KOptDlgData

KOptDlg::KOptDlg(QWidget* parent, const char* name, bool modal) : Inherited(parent, name, modal)
{	QString str;

	ad=AchsenDicke;
	td=TeilstrichDicke;
	tl=TeilstrichLaenge;
	le_ad->setText(str.setNum(ad));
	le_td->setText(str.setNum(td));
	le_tl->setText(str.setNum(tl));
	farbe.setRgb(AchsenFarbe);
}

KOptDlg::~KOptDlg()
{
}

// Slots

void KOptDlg::onok()
{	QString str;

	str=le_ad->text();
	ad=str.toInt();
	str=le_td->text();
	td=str.toInt();
	str=le_tl->text();
	tl=str.toInt();
	AchsenDicke=ad;
	TeilstrichDicke=td;
	TeilstrichLaenge=tl;
	AchsenFarbe=farbe.rgb();
	
	if(cb_default->isChecked())
	{	kc->setGroup("Axes");
		
		kc->writeEntry("Axes Width", ad);
		kc->writeEntry("Tic Width", td);
		kc->writeEntry("Tic Length", tl);
		kc->writeEntry("Color", farbe);
		kc->sync();
	}
	
	done(1);
}

void KOptDlg::oncancel()
{	done(0);
}

void KOptDlg::onfarbe()
{	KColorDialog cdlg;

	cdlg.setColor(farbe);
	cdlg.getColor(farbe);
}
