#ifndef diagr_included
#define diagr_included

#include <qnamespace.h>
#include <qpainter.h>
#include <qpaintdevice.h>
#include <qcolor.h>
#include <qpen.h>
#include <qpoint.h>
#include <qrect.h>


#include "math.h"
#include <stdio.h>


#define	Line				drawLine
#define	Lineh(x1, y, x2)	drawLine(x1, y, x2, y)
#define Linev(x, y1, y2)	drawLine(x, y1, x, y2)


#define	ACHSEN			1		// mit Achsen
#define PFEILE			2		// mit Pfeilen an den Achsen
#define RAHMEN			4		// mit Rahmen
#define	EXTRAHMEN		8		// mit vergrößertem Rahmen
#define BESCHRIFTUNG	16		// mit Beschriftung

#define	LINIENRASTER	1	
#define	KREUZRASTER     2
#define	KREISRASTER     3


class CDiagr
{	public:

	CDiagr();
	~CDiagr();
	
	void	Create(QPoint, int, int,
		       double, double,
		       double, double,
		       char, char);				// Parameter setzen
	void	Skal(double, double, char);	// Skalierung u. Gittertyp festlegen
	void	Plot(QPainter*);			// Diagramm zeichnen
	QRect	GetPlotArea() {return PlotArea;}
	QRect	GetRahmen() {return Rahmen;}
	int	Transx(double),				// transformiert reale Koordinaten
		Transy(double); 			// in Gerätekoordinaten
	double	Transx(int),			// transformiert Gerätekoordinaten
			Transy(int);			// in reale Koordinaten
	
	
	QRgb RahmenFarbe,
	     AchsenFarbe,
	     GitterFarbe; 
			    
	uint RahmenDicke,
	     AchsenDicke,
	     GitterDicke,
	     TeilstrichDicke,
	     TeilstrichLaenge,
	     RahmenOffset,
	     xclipflg,       	// clipflg wird auf 1 gesetzt, wenn die
	     yclipflg;			// Koordinaten außerhalb des Diagramms liegen
	                         	
	private:

	void Achsen(QPainter*);			// Achsen zeichnen
	void Raster(QPainter*);			// Raster zeichnen
	void Beschriftung(QPainter*);	// Achsen beschriften
	
	char mode,			// Kombination von ACHSEN, PFEILE,
						// RAHMEN u. BESCHRIFTUNG
         g_mode,		// 0 -> kein Raster
            			// 1 -> Linienraster
            			// 2 -> Kreuzraster
            			// 3 -> Kreisraster
         prt;           // 1 -> Druckerausgabe

	double  xmin, xmax, // x-Wertebereich
        	ymin, ymax, // y-Wertebereich
        	xmd,  ymd,	// x/y Begrenzung
        	ex,   ey,	// x/y-Achsenteilung
            tsx,  tsy,	// Positionen der ersten Teilstriche
        	ox,   oy,   // Bildschirm-Koordinaten des Ursprungs
        	skx,  sky;  // Skalierungsfaktoren
	QRect	PlotArea,	// Plotbereich
			Rahmen;     // Rahmen
};

#endif // diagr_included
