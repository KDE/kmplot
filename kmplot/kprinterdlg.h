#ifndef kprintdlg_included
#define kprintdlg_included

// Qt includes
#include <qcheckbox.h>

// KDE includes
#include <kdeprint/kprintdialogpage.h>

class KPrinterDlg : public KPrintDialogPage
{
public:
    KPrinterDlg( QWidget *parent = 0, const char *name = 0 );

    //reimplement virtual functions
    void getOptions( QMap<QString,QString>& opts, bool include_def = false );
    void setOptions( const QMap<QString,QString>& opts );
    bool isValid( QString& msg );

private:
    QCheckBox *m_printtable;
};

#endif //kprinterdlg_included
