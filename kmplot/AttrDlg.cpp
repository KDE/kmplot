#include "AttrDlg.h"
#include "AttrDlg.moc"

#define Inherited AttrDlgData

AttrDlg::AttrDlg(QWidget* parent, const char* name, bool modal) : Inherited(parent, name, modal)
{
}

AttrDlg::~AttrDlg()
{
}

// Slots

void AttrDlg::onok()
{	QString str;

	str=le_dicke->text();
	ps.fktext[ix].dicke=str.toInt();
	ps.fktext[ix].farbe=farbe.rgb();
	
	if(cb_default->isChecked())
	{	char s[8];
	
		kc->setGroup("Graphen");
		kc->writeEntry("Linienstärke", ps.fktext[ix].dicke);
		sprintf(s, "farbe%c", '0'+ix);
		kc->writeEntry(s, farbe);
		kc->sync();
	}
	
	done(1);
}

void AttrDlg::oncancel()
{	done(0);
}

void AttrDlg::onfarbe()
{	KColorDialog cdlg;

	cdlg.setColor(farbe);
	cdlg.getColor(farbe);
}

void AttrDlg::show()
{	QString str;

	le_dicke->setText(str.setNum(ps.fktext[ix].dicke));
	farbe.setRgb(ps.fktext[ix].farbe);
	QDialog::show();
}
