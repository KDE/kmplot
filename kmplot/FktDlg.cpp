#include "FktDlg.h"
#include "FktDlg.moc"

#define Inherited FktDlgData

FktDlg::FktDlg(QWidget* parent,	const char* name, bool modal) : Inherited(parent, name, modal)
{	int ix;
	QString fname, fstr;

	lb_fktliste->clear();
	fktidx.clear();
	
	for(ix=0; ix<ps.ufanz; ++ix)
	{	if(ps.getfkt(ix, fname, fstr)==-1) continue;

		lb_fktliste->insertItem(ps.fktext[ix].extstr);
		fktidx.append((int *)ix);
	}
	
    le_fktstr->setText("f(x)=");
    le_fktstr->selectAll();
	le_fktstr->setFocus();
}

FktDlg::~FktDlg()
{
}


// Slots

void FktDlg::onok()
{	char c0;
	QString fname, fstr, str;

	fstr=le_fktstr->text();
	if(fstr!="")
	{	int i=fstr.find(';');
		
		if(i==-1) str=fstr;
		else str=fstr.left(i);
		ix=ps.addfkt(str);
		if(ix==-1) {ps.errmsg(); errflg=1; return;}
		
		ps.fktext[ix].extstr=fstr;
		if(ps.getext(ix)==-1) {errflg=1; return;}
		
		if((c0=fstr[0].latin1())=='x')
		{	ps.getfkt(ix, fname, str);
		    fname[0]='y';
			if(ps.getfix(fname)==-1)
			{	int p;
			
				lb_fktliste->insertItem(fstr);
	    		fktidx.append((int *)(lb_fktliste->currentItem()));
	    		p=fstr.find('=');
	    		fstr=fstr.left(p+1);
		        fstr[0]='y';
		        le_fktstr->setText(fstr);
		        le_fktstr->setFocus();
				le_fktstr->deselect();
		        return;
			}
		}
		else if(c0=='y')
		{	if(ps.getfkt(ix, fname, str)!=-1)
		    {	fname[0]='x';
				ix=ps.getfix(fname);
			}
		}
	}
	errflg=0;
	done(1);
}

void FktDlg::oncancel()
{	done(0);
}

void FktDlg::ondelete()
{	int ix, num;

	if((num=lb_fktliste->currentItem())==-1) return;
	
	ix=(int)fktidx.at(num);
	chflg=1;
	ps.delfkt(ix);
	lb_fktliste->removeItem(num);
	fktidx.remove(num);
}

void FktDlg::onedit()
{	int ix, num;

	if((num=lb_fktliste->currentItem())==-1) return;
	
	ix=(int)fktidx.at(num);
	chflg=1;
	ps.delfkt(ix);
	le_fktstr->setText(lb_fktliste->text(num));
	lb_fktliste->removeItem(num);
	fktidx.remove(num);
	le_fktstr->setFocus();
	le_fktstr->deselect();
}

void FktDlg::ondblclick(int)
{	onedit();
}

void FktDlg::onattr()
{	int num;
	AttrDlg attr;

	if((num=lb_fktliste->currentItem())==-1) return;
	
    le_fktstr->clear();
	attr.ix=(int)fktidx.at(num);
	chflg=1;
	attr.exec();
}

