#include "RstDlg.h"
#include "RstDlg.moc"


#define Inherited RstDlgData

RstDlg::RstDlg(QWidget* parent,	const char* name, bool modal) : Inherited(parent, name, modal)
{	//setCaption( i18n("Raster") );
	switch(g_mode)
	{  case 0:	rb_r1->setChecked(TRUE); break;
	   case 1:	rb_r2->setChecked(TRUE); break;
	   case 2:	rb_r3->setChecked(TRUE); break;
	   case 3:	rb_r4->setChecked(TRUE);
	}
	farbe.setRgb(GitterFarbe);
}


RstDlg::~RstDlg()
{
}

// Slots

void RstDlg::onok()
{	QString str;

	if(rb_r1->isChecked()) g_mode=0;
	else if(rb_r2->isChecked()) g_mode=1;
	else if(rb_r3->isChecked()) g_mode=2;
	else if(rb_r4->isChecked()) g_mode=3;
	str=le_dicke->text();
	GitterDicke=str.toInt();
	GitterFarbe=farbe.rgb();
	
	if(cb_default->isChecked())
	{	kc->setGroup("Gitter");
		kc->writeEntry("Gitterstärke", GitterDicke);
		kc->writeEntry("Mode", g_mode);
		kc->writeEntry("Gitterfarbe", farbe);
		kc->sync();
	}
	
	done(1);
}

void RstDlg::oncancel()
{	done(0);
}

void RstDlg::onfarbe()
{	KColorDialog cdlg;

	cdlg.setColor(farbe);
	cdlg.getColor(farbe);
}

void RstDlg::show()
{	QString str;

	le_dicke->setText(str.setNum(GitterDicke));
	QDialog::show();
}
