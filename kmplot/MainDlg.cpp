// local includes
#include "MainDlg.h"
#include "MainDlg.moc"


MainDlg::MainDlg(const char* name) : KMainWindow(0, name)
{	init();
    view=new View(this);
    setCentralWidget(view);
    setupActions();
    setupStatusBar();
}

MainDlg::~MainDlg()
{
}

void MainDlg::setupActions()
{	KStdAction::openNew(this, SLOT(neu()), actionCollection());
    KStdAction::open(this, SLOT(load()), actionCollection());
    KStdAction::print(this, SLOT(print()), actionCollection());
    KStdAction::save(this, SLOT(save()), actionCollection());
    KStdAction::saveAs(this, SLOT(saveas()), actionCollection());
    KStdAction::quit(kapp, SLOT(closeAllWindows()), actionCollection());
    connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));
    KStdAction::helpContents(this, SLOT(hilfe()), actionCollection(), "helpcontents");

    (void) new KAction(i18n("&Axes..."), 0, this, SLOT(achsen()), actionCollection(), "axes");
    (void) new KAction(i18n("&Scale..."), 0, this, SLOT(skalierung()), actionCollection(), "scale");
    (void) new KAction(i18n("&Grid..."), 0, this, SLOT(raster()), actionCollection(), "grid");
    (void) new KAction(i18n("&Step..."), 0, this, SLOT(schrittw()), actionCollection(), "step");
    (void) new KAction(i18n("&Names"), 0, this, SLOT(bezeichnungen()), actionCollection(), "names");
    viewToolBar = KStdAction::showToolbar(this, SLOT(tbmode()), actionCollection());
    viewStatusBar = KStdAction::showStatusbar(this, SLOT(stbmode()), actionCollection());

    (void) new KAction(i18n("Functions" ), "kfkt.png", 0, this, SLOT(funktionen()), actionCollection(), "functions");
    (void) new KAction(i18n("Coordinate System I" ), "ksys1.png", 0, this, SLOT(onachsen1()), actionCollection(), "coord_i");
    (void) new KAction(i18n("Coordinate System II" ), "ksys2.png", 0, this, SLOT(onachsen2()), actionCollection(), "coord_ii");
    (void) new KAction(i18n("Coordinate System III" ), "ksys3.png", 0, this, SLOT(onachsen3()), actionCollection(), "coord_iii");

    createGUI(locate("data", "kmplot/kmplotui.rc"));
}

void MainDlg::setupStatusBar()
{   stbar=statusBar();
    stbar->insertFixedItem("1234567890", 1);
    stbar->insertFixedItem("1234567890", 2);
    stbar->insertItem("", 3, 1);
    stbar->changeItem("", 1);
    stbar->changeItem("", 2);
    view->stbar=stbar;
}


// Slots

void MainDlg::neu()
{   init();
    datei="";
    setCaption( datei );
    view->update();
}

void MainDlg::doSave()
{   if(datei.isEmpty()) return;

    if(datei.find(".svg")!=-1)
    {   QPicture pic;

        view->draw(&pic);
        pic.save(datei, "svg");
        return;
    }

    KConfig file(datei);
    file.setGroup("Axes");
    file.writeEntry("Mode", mode);
    file.writeEntry("Xmin", xmin);
    file.writeEntry("Xmax", xmax);
    file.writeEntry("Ymin", ymin);
    file.writeEntry("Ymax", ymax);
    file.writeEntry("Xmin String", xminstr);
    file.writeEntry("Xmax String", xmaxstr);
    file.writeEntry("Ymin String", yminstr);
    file.writeEntry("Ymax String", ymaxstr);
    file.writeEntry("Coord X", koordx);
    file.writeEntry("Coord Y", koordy);
    file.writeEntry("Axes Width", AchsenDicke);
    file.writeEntry("Color", QColor(AchsenFarbe));
    file.writeEntry("Tic Width", TeilstrichDicke);
    file.writeEntry("Tic Length", TeilstrichLaenge);
    
    file.setGroup("Grid");
    file.writeEntry("Mode", g_mode);
    file.writeEntry("Line Width", GitterDicke);
    file.writeEntry("Color", QColor(GitterFarbe));

    file.setGroup("Scale");
    file.writeEntry("tlgx", tlgx);
    file.writeEntry("tlgy", tlgy);
    file.writeEntry("tlgxstr", tlgxstr);
    file.writeEntry("tlgystr", tlgystr);
    file.writeEntry("drskalx", drskalx);
    file.writeEntry("drskaly", drskaly);
    file.writeEntry("drskalxstr", drskalxstr);
    file.writeEntry("drskalystr", drskalystr);

    file.setGroup("Step");
    file.writeEntry("rsw", rsw);

    for(int ix=0; ix<ps.ufanz; ix++)
    {   if( !ps.fktext[ix].extstr.isEmpty() )
        {
            QString groupname = QString("Function%1").arg(ix);
        file.setGroup(groupname);
        file.writeEntry("Equation", ps.fktext[ix].extstr);
        file.writeEntry("Visible", ps.fktext[ix].f_mode);
        file.writeEntry("Visible Derivative", ps.fktext[ix].f1_mode);
        file.writeEntry("Visible 2nd Derivative", ps.fktext[ix].f2_mode);
        file.writeEntry("Line Width", ps.fktext[ix].dicke);
        file.writeEntry("Color", QColor(ps.fktext[ix].farbe));
        }
    }
    file.sync();
}

void MainDlg::save()
{   if(datei.isEmpty()) saveas();
    else doSave();
}

void MainDlg::saveas()
{   datei=KFileDialog::getSaveFileName(QDir::currentDirPath(), i18n( "*.fkt|KmPlot files (*.fkt)\n*.svg|Scalable Vector Graphics (*.svg)\n*|All files" ), this, i18n( "Save As..." ));
    if(!datei.isEmpty())
    {   if(datei.find(".")==-1) datei=datei+".fkt";
        doSave();
        setCaption(datei);
    }
}

void MainDlg::load()
{   QString d;

    d=KFileDialog::getOpenFileName(QDir::currentDirPath(), i18n("*.fkt|KmPlot files (*.fkt)\n*|All files"), this, i18n("Open..."));
    if(d.isEmpty()) return;

    init();
    datei=d;
    KConfig file(datei);
    if(file.hasGroup("Axes"))
    {   file.setGroup("Axes");
        mode=file.readUnsignedNumEntry("Mode");
        xmin=file.readDoubleNumEntry("Xmin", -8.0);
        xmax=file.readDoubleNumEntry("Xmax", 8.0);
        ymin=file.readDoubleNumEntry("Ymin", -8.0);
        ymax=file.readDoubleNumEntry("Ymax", 8.0);
        xminstr=file.readEntry("Xmin String");
        xmaxstr=file.readEntry("Xmax String");
        yminstr=file.readEntry("Ymin String");
        ymaxstr=file.readEntry("Ymax String");
        koordx=file.readNumEntry("Coord X");
        koordy=file.readNumEntry("Coord Y");
        AchsenDicke=file.readNumEntry("Axes Width");
        AchsenFarbe=file.readColorEntry("Color").rgb();
        TeilstrichDicke=file.readNumEntry("Tic Width");
        TeilstrichLaenge=file.readNumEntry("Tic Length");
    }

    if(file.hasGroup( "Grid" ) )
    {
        file.setGroup( "Grid" );
        GitterFarbe=file.readColorEntry("Color").rgb();
        g_mode=file.readUnsignedNumEntry("Mode");
        GitterDicke=file.readNumEntry("Line Width");
    }
    
    if(file.hasGroup("Scale"))
    {   file.setGroup("Scale");
        tlgx=file.readDoubleNumEntry("tlgx");
        tlgy=file.readDoubleNumEntry("tlgy");
        tlgxstr=file.readEntry("tlgxstr");
        tlgystr=file.readEntry("tlgystr");
        drskalx=file.readDoubleNumEntry("drskalx");
        drskaly=file.readDoubleNumEntry("drskaly");
        drskalxstr=file.readEntry("drskalxstr");
        drskalystr=file.readEntry("drskalystr");
    }

    if(file.hasGroup("Step"))
    {   file.setGroup("Step");
        rsw=file.readDoubleNumEntry("rsw");

        for(int ix=0; ix<ps.ufanz; ix++)
        {   QString groupname = QString("Function%1").arg(ix);
            if(file.hasGroup(groupname))
            {   file.setGroup(groupname);
                ps.fktext[ix].extstr=file.readEntry("Equation");
                ps.fktext[ix].f_mode=file.readUnsignedNumEntry("Visible");
                ps.fktext[ix].f1_mode=file.readUnsignedNumEntry("Visible Derivative");
                ps.fktext[ix].f2_mode=file.readUnsignedNumEntry("Visible 2nd Derivative");
                ps.fktext[ix].dicke=file.readNumEntry("Line Width");
                ps.fktext[ix].farbe=file.readColorEntry("Color").rgb();

                QCString fstr=ps.fktext[ix].extstr.utf8();
                if( !fstr.isEmpty() )
                {   int i=fstr.find(';');
                    QCString str;
                    if(i==-1) str=fstr;
                    else str=fstr.left(i);
                    ix=ps.addfkt(str);
                    ps.getext(ix);
                }
            }
        }
    }
    setCaption(datei);
    view->update();
}

void MainDlg::print()
{   KPrinter prt(QPrinter::PrinterResolution);

    if(prt.setup(this))
    {	prt.setFullPage(true);
    	view->draw(&prt);
    }
}

void MainDlg::bezeichnungen()
{	static BezWnd bez;
    bez.move(200, 200);
    bez.show();
}

void MainDlg::stbmode()
{	if(!viewStatusBar->isChecked()) statusBar()->hide();
    else statusBar()->show();
}

void MainDlg::tbmode()
{	if(!viewToolBar->isChecked()) toolBar()->hide();
    else toolBar()->show();
}

void MainDlg::funktionen()
{
    if (!fdlg) fdlg=new FktDlg( this );
    fdlg->show();
//    fdlg.exec();
//    view->update();
}

void MainDlg::skalierung()
{	SkalDlg skdlg;

    skdlg.exec();
    view->update();
}

void MainDlg::schrittw()
{	SwDlg sdlg;

    sdlg.exec();
    view->update();
}

void MainDlg::raster()
{	RstDlg rdlg;

    rdlg.exec();
    view->update();
}

void MainDlg::achsen()
{	KoordDlg kdlg;

    kdlg.exec();
    view->update();
}

void MainDlg::onachsen1()
{ 	koordx=koordy=0;
    xminstr=yminstr="-8";
    xmaxstr=ymaxstr="8";
    xmin=ymin=-8.;
    xmax=ymax=8.;
    /*tlgx=tlgy=1.;
    tlgxstr="1"; tlgystr="1";
    drskalx=drskaly=1.;
    drskalxstr="1"; drskalystr="1";*/
    view->update();
}

void MainDlg::onachsen2()
{ 	koordx=2; koordy=0;
    xminstr="0";  yminstr="-8";
    xmaxstr="16"; ymaxstr="8";
    xmin=0.;  ymin=-8.;
    xmax=16.; ymax=8.;
    /*tlgx=tlgy=1.;
    tlgxstr="1"; tlgystr="1";
    drskalx=drskaly=1.;
    drskalxstr="1"; drskalystr="1";*/
    view->update();
}

void MainDlg::onachsen3()
{	koordx=koordy=2;
    xminstr=yminstr="0";
    xmaxstr=ymaxstr="16";
    xmin=ymin=0.;
    xmax=ymax=16.;
    /*tlgx=tlgy=1.;
    tlgxstr="1"; tlgystr="1";
    drskalx=drskaly=1.;
    drskalxstr="1"; drskalystr="1";*/
    view->update();
}

void MainDlg::hilfe()
{	kapp->invokeHelp("", "kmplot");
}

