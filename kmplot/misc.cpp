// KDE includes
#include <kmessagebox.h>
#include <kurl.h>

// local includes
#include "misc.h"
#include "settings.h"

KApplication *ka;

XParser ps( 10, 200, 20 );

int mode,     // Diagrammodus
g_mode;  // grid style

int koordx,     // 0 => [-8|+8]
koordy,     // 1 => [-5|+5]
// 2 => [0|+16]
// 3 => [0|+10]
axesThickness,
GitterDicke,
gradThickness,
gradLength;

double xmin,                      // min. x-Wert
xmax,                      // max. x-Wert
ymin,                   // min. y-Wert
ymax,                   // max. y-Wert
sw,       // Schrittweite
rsw,       // rel. Schrittweite
tlgx,       // x-Achsenteilung
tlgy,       // y-Achsenteilung
drskalx,       // x-Ausdruckskalierung
drskaly;     // y-Ausdruckskalierung

QString xminstr,      // String für xmind
xmaxstr,      // String für xmaxd
yminstr,      // String für ymind
ymaxstr,      // String für ymaxd
tlgxstr,                  // String für tlgx
tlgystr,                  // String für tlgy
drskalxstr,               // String für drskalx
drskalystr;             // String für drskaly

QString font_header, font_axes; // Font family names

QRgb axesColor,
gridColor;

bool printtable;		// header table printing option


void getSettings()
{
	mode = ACHSEN | PFEILE | EXTRAHMEN;
	rsw = 1.;

	// axes settings
	
	koordx = Settings::xRange();
	koordy = Settings::yRange();
	xminstr = Settings::xMin();
	xmaxstr = Settings::xMax();
	yminstr = Settings::yMin();
	ymaxstr = Settings::yMax();
	
	if( xminstr.isEmpty() ) xminstr = "-2*pi";
	if( xmaxstr.isEmpty() ) xmaxstr = "2*pi";
	if( yminstr.isEmpty() ) yminstr = "-2*pi";
	if( ymaxstr.isEmpty() ) ymaxstr = "2*pi";

	if ( !coordToMinMax( koordx, xmin, xmax, xminstr, xmaxstr ) )
	{
		KMessageBox::error( 0, i18n( "Config file x-axis entry corrupt.\n"
		                             "Fall back to system defaults.\nCall Settings->Configure KmPlot..." ) );
		xminstr = "-2*pi";
		xmaxstr = "2*pi";
		koordx = 0;
		coordToMinMax( koordx, xmin, xmax, xminstr, xmaxstr );
	}
	if ( !coordToMinMax( koordy, ymin, ymax, yminstr, ymaxstr ) )
	{
		KMessageBox::error( 0, i18n( "Config file y-axis entry corrupt.\n"
		                             "Fall back to system defaults.\nCall Settings->Configure KmPlot..." ) );
		yminstr = "-2*pi";
		ymaxstr = "2*pi";
		koordy = 0;
		coordToMinMax( koordy, ymin, ymax, yminstr, ymaxstr );
	}
	
	const char* units[ 8 ] = { "10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4" };
	
	tlgxstr = units[ Settings::xScaling() ];
	tlgx = ps.eval( tlgxstr );
	tlgystr = units[ Settings::yScaling() ];
	tlgy = ps.eval( tlgystr );

	drskalxstr = units[ Settings::xPrinting() ];
	drskalx = ps.eval( drskalxstr );
	drskalystr = units[ Settings::yPrinting() ];
	drskaly = ps.eval( drskalystr );

	axesThickness = Settings::axesLineWidth();
	axesColor = Settings::axesColor().rgb();
	if ( Settings::showLabel() ) mode |= BESCHRIFTUNG;
	gradThickness = Settings::ticWidth();
	gradLength = Settings::ticLength();

	// grid settings

	GitterDicke = Settings::gridLineWidth();
	g_mode = Settings::gridStyle();
	gridColor = Settings::gridColor().rgb();

	// font settings
	font_header = Settings::headerTableFont().family();
	font_axes = Settings::axesFont().family();

	// graph settings

	ps.dicke0 = Settings::gridLineWidth();
	ps.fktext[ 0 ].farbe0 = Settings::color0().rgb();
	ps.fktext[ 1 ].farbe0 = Settings::color1().rgb();
	ps.fktext[ 2 ].farbe0 = Settings::color2().rgb();
	ps.fktext[ 3 ].farbe0 = Settings::color3().rgb();
	ps.fktext[ 4 ].farbe0 = Settings::color4().rgb();
	ps.fktext[ 5 ].farbe0 = Settings::color5().rgb();
	ps.fktext[ 6 ].farbe0 = Settings::color6().rgb();
	ps.fktext[ 7 ].farbe0 = Settings::color7().rgb();
	ps.fktext[ 8 ].farbe0 = Settings::color8().rgb();
	ps.fktext[ 9 ].farbe0 = Settings::color9().rgb();
	printtable = true;
	
	// precision settings
	rsw = Settings::stepWidth();
}

void init()
{
	getSettings();

	for ( int ix = 0; ix < ps.ufanz; ++ix )
		ps.delfkt( ix );
}

/*
 * evaluates the predefined axes settings (kkordx/y)
 */
bool coordToMinMax( const int koord, double &min, double &max, const QString minStr, const QString maxStr )
{
	bool ok = true;
	switch ( koord )
	{
	case 0:
		min = -8.0;
		max = 8.0;
		break;
	case 1:
		min = -5.0;
		max = 5.0;
		break;
	case 2:
		min = 0.0;
		max = 16.0;
		break;
	case 3:
		min = 0.0;
		max = 10.0;
		break;
	case 4:
		min = ps.eval( minStr );
		ok = ps.err == 0;
		max = ps.eval( maxStr );
		ok &= ps.err == 0;
	}
	ok &= min < max;
	return ok;
}

