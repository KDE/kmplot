// locale includes
#include "FktDlg.h"
#include "FktDlg.moc"
#include "MainDlg.h"

#define Inherited FktDlgData

FktDlg::FktDlg( QWidget* parent, const char* name ) : Inherited( parent, name )
{
    int ix;
    QString fname, fstr;

    lb_fktliste->clear();
    // fktidx.clear();

    for ( ix = 0; ix < ps.ufanz; ++ix )
    {
        if ( ps.getfkt( ix, fname, fstr ) == -1 )
            continue;

        lb_fktliste->insertItem( ps.fktext[ ix ].extstr );
        // fktidx.append( ix );
    }

    le_fktstr->setText( "f(x)=" );
    le_fktstr->selectAll();
    le_fktstr->setFocus();
}

FktDlg::~FktDlg()
{
}


// Slots

void FktDlg::onapply()
{
    int ix;
    char c0;
    QString fname, fstr, str;

    fstr = le_fktstr->text();
    if ( fstr != "" )
    {
        // left from semicolon is function equation
        int i = fstr.find( ';' );

        if ( i == -1 )
            str = fstr;
        else
            str = fstr.left( i );

        // test the function equation syntax
        ix = ps.addfkt( str );
        if ( ix == -1 )
        {
            ps.errmsg();
            errflg = 1;
            return ;
        }

        // handle the extensions
        ps.fktext[ ix ].extstr = fstr;
        // test the extension syntax
        if ( ps.getext( ix ) == -1 )
        {
            errflg = 1;
            return ;
        }

        // handle parametric functions: xf(t), yf(t)
        if ( ( c0 = fstr[ 0 ].latin1() ) == 'x' )
        {
            ps.getfkt( ix, fname, str );
            fname[ 0 ] = 'y';
            if ( ps.getfix( fname ) == -1 )
            {
                int p;

                lb_fktliste->insertItem( fstr );
                // fktidx.append( lb_fktliste->currentItem() );
                p = fstr.find( '=' );
                fstr = fstr.left( p + 1 );
                fstr[ 0 ] = 'y';
                le_fktstr->setText( fstr );
                le_fktstr->setFocus();
                le_fktstr->deselect();
                return ;
            }
        }
        else if ( c0 == 'y' )
        {
            if ( ps.getfkt( ix, fname, str ) != -1 )
            {
                fname[ 0 ] = 'x';
                ix = ps.getfix( fname );
            }
        }
        lb_fktliste->insertItem( fstr );
        le_fktstr->clear();
    }
    errflg = 0;
    updateView();
}

void FktDlg::onok()
{
    onapply();
    if ( errflg == 0 ) onclose();
}

void FktDlg::onclose()
{
    ( ( MainDlg* ) parentWidget() )->fdlg = 0;
    close( TRUE );
}

void FktDlg::ondelete()
{
    int ix, num;

    if ( ( num = lb_fktliste->currentItem() ) == -1 )
        return ;

    ix = getIx( lb_fktliste->text( num ) );
    chflg = 1;
    ps.delfkt( ix );
    lb_fktliste->removeItem( num );
    updateView();
}

void FktDlg::onedit()
{
    int ix, num;

    if ( ( num = lb_fktliste->currentItem() ) == -1 )
        return ;

    ix = getIx( lb_fktliste->text( num ) );
    chflg = 1;
    ps.delfkt( ix );
    le_fktstr->setText( lb_fktliste->text( num ) );
    lb_fktliste->removeItem( num );
    le_fktstr->setFocus();
    le_fktstr->deselect();
    updateView();
}

void FktDlg::ondblclick( int )
{
    onedit();
}

void FktDlg::onattr()
{
    int num;
    AttrDlg attr;

    if ( ( num = lb_fktliste->currentItem() ) == -1 )
        return ;

    le_fktstr->clear();
    // attr.ix = ( int ) fktidx[ num ];
    attr.ix = getIx( lb_fktliste->text( num ) );
    chflg = 1;
    attr.exec();
    updateView();
}

int FktDlg::getIx( const QString f_str )
{
    QString fname;
    QString fstr;
    for ( int ix = 0; ix < ps.ufanz; ++ix )
    {
        if ( ps.getfkt( ix, fname, fstr ) == -1 )
            continue;

        if ( ps.fktext[ ix ].extstr == f_str )
            return ix;
    }
    return -1;
}

void FktDlg::updateView()
{
    ( ( MainDlg* ) parentWidget() ) ->view->update();
}

