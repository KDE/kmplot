//
// C++ Interface: kconstanteditor
//
// Description: 
//
//
// Author: Fredrik Edemar <f_edemar@linux.se>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KCONSTANTEDITOR_H
#define KCONSTANTEDITOR_H

#include "qconstanteditor.h"
#include "keditconstant.h"
#include "View.h"

/**
@author Fredrik Edemar
*/
class KConstantEditor : public QConstantEditor
{
Q_OBJECT
public:
    KConstantEditor(QWidget *parent = 0, const char *name = 0);
    KConstantEditor(View *, QWidget *parent = 0, const char *name = 0);
    ~KConstantEditor();
    
public slots:
    void cmdNew_clicked();
    void cmdEdit_clicked();
    void cmdDelete_clicked();
    void varlist_clicked( QListViewItem * item );
    void varlist_doubleClicked( QListViewItem * );
    void cmdDuplicate_clicked();
    void newConstantSlot();
    void editConstantSlot();
  
    
private:   
   char constant;
   QString value;
   View *m_view;
    
};

#endif
