//
// C++ Interface: keditconstant
//
// Description: 
//
//
// Author: Fredrik Edemar <f_edemar@linux.se>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KEDITCONSTANT_H
#define KEDITCONSTANT_H

#include "qeditconstant.h"
#include "xparser.h"

/**
@author Fredrik Edemar
*/
class KEditConstant : public QEditConstant
{
Q_OBJECT
public:
    KEditConstant(QWidget *parent = 0, const char *name = 0);
    KEditConstant(XParser *p, char &, QString &,QWidget *parent = 0, const char *name = 0);

    ~KEditConstant();
   
public slots:
    void cmdOK_clicked();
    void txtVariable_lostFocus();
signals:
    void finished();
        
private:
    char *constant;
    QString *value;
    XParser *m_parser;
    

};

#endif
