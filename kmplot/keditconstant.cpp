//
// C++ Implementation: keditconstant
//
// Description: 
//
//
// Author: Fredrik Edemar <f_edemar@linux.se>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kdebug.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qpushbutton.h>


#include "keditconstant.h"

KEditConstant::KEditConstant(QWidget *parent, const char *name)
 : QEditConstant(parent, name)
{
}


KEditConstant::KEditConstant(XParser *p, char &c, QString &v, QWidget *parent, const char *name)
 : QEditConstant(parent, name)
{
	constant = &c;
	value = &v;
	m_parser = p;
	if ( *constant != '0' )
	{
		txtConstant->setText(QChar(*constant));
		txtValue->setText(*value);
	}
	connect( cmdCancel, SIGNAL( clicked() ), this, SLOT( deleteLater() ));
	connect( cmdOK, SIGNAL( clicked() ), this, SLOT( cmdOK_clicked() ));
}


KEditConstant::~KEditConstant()
{
}

void KEditConstant::cmdOK_clicked()
{
	*constant = txtConstant->text().at(0).latin1();
	*value = txtValue->text();
	
	if ( *constant<'A' || *constant>'Z')
	{
		KMessageBox::error(this, i18n("Please insert a valid constant name between A and Z"));
		return;
	}
	if (  *constant=='E')
	{
		KMessageBox::error(this, i18n("You are not allowed to define a constant named E"));
		return;
	}	
	if ( txtConstant->isEnabled() ) //creating, not edit a constant
	{
		bool found= false;
		QValueVector<Constant>::iterator it;
		for(it = m_parser->constant.begin(); it!= m_parser->constant.end() && !found;it++)
		{
			if ( it->constant == *constant)
					found = true;
		}
		if (found)
		{
			KMessageBox::error(this, i18n("The constant already exists"));
			return;
		}	
	}
	(double) m_parser->eval(*value);
	if ( m_parser->errmsg() )
		return;
	
	emit finished();
	QDialog::accept();
}

void KEditConstant::txtVariable_lostFocus()
{
    txtConstant->setText( txtConstant->text().upper() );
}

#include "keditconstant.moc"
