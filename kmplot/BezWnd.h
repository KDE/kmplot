#ifndef BezWnd_included
#define BezWnd_included

#include "BezWndData.h"
#include <kapp.h>
#include <klocale.h>


class BezWnd : public BezWndData
{
    Q_OBJECT

public:

    BezWnd(QWidget* parent=NULL, const char* name=NULL);
    virtual ~BezWnd();

};
#endif // BezWnd_included
