/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter Möller
*               2000, 2002 kd.moeller@t-online.de
*               
* This file is part of the KDE Project.
* KmPlot is part of the KDE-EDU Project.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/


// Qt includes
#include <qvariant.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcolor.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qstring.h>
#include <qvbuttongroup.h>

// KDE includes
#include <kaction.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kfontcombo.h>
#include <kiconloader.h>
#include <klocale.h>

#include <kdebug.h>

// locale includes
#include "ksettingsdlg.h"
#include "misc.h"

//////////
// Dialog Page for Color Settings

KColorPage::KColorPage( QWidget *parent, const char *name )
		: QTabWidget ( parent, name )
{
	coords = new QWidget( this, "coords" );
	QGridLayout *layout_coords = new QGridLayout( coords, 3, 2, 5 );
	layout_coords->setAutoAdd( true );
	l_coords_axes = new QLabel( i18n( "Axes:" ), coords );
	cb_coords_axes = new KColorButton( coords );
	l_coords_grid = new QLabel( i18n( "Grid:" ), coords );
	cb_coords_grid = new KColorButton( coords );
	layout_coords->setRowStretch( 2, 1 );
	addTab( coords, i18n( "Coords" ) );

	functions = new QWidget( this, "functions" );
	QGridLayout *layout_functions = new QGridLayout( functions, 6, 4, 5 );
	layout_functions->setAutoAdd( true );
	l_functions0 = new QLabel( i18n( "Function 1:" ), functions );
	cb_functions0 = new KColorButton( functions );
	l_functions1 = new QLabel( i18n( "Function 2:" ), functions );
	cb_functions1 = new KColorButton( functions );
	l_functions2 = new QLabel( i18n( "Function 3:" ), functions );
	cb_functions2 = new KColorButton( functions );
	l_functions3 = new QLabel( i18n( "Function 4:" ), functions );
	cb_functions3 = new KColorButton( functions );
	l_functions4 = new QLabel( i18n( "Function 5:" ), functions );
	cb_functions4 = new KColorButton( functions );
	l_functions5 = new QLabel( i18n( "Function 6:" ), functions );
	cb_functions5 = new KColorButton( functions );
	l_functions6 = new QLabel( i18n( "Function 7:" ), functions );
	cb_functions6 = new KColorButton( functions );
	l_functions7 = new QLabel( i18n( "Function 8:" ), functions );
	cb_functions7 = new KColorButton( functions );
	l_functions8 = new QLabel( i18n( "Function 9:" ), functions );
	cb_functions8 = new KColorButton( functions );
	l_functions9 = new QLabel( i18n( "Function 10:" ), functions );
	cb_functions9 = new KColorButton( functions );
	addTab( functions, i18n( "Functions" ) );

	// Load actual defaults from ConfigFile
	loadSettings();
}

void KColorPage::loadSettings()
{
	QColor tmp_color;
	tmp_color.setRgb( 192, 192, 192 );
	if ( kc->hasGroup( "Grid" ) )
	{
		kc->setGroup( "Grid" );
		cb_coords_grid->setColor( kc->readColorEntry( "Color", &tmp_color ) );
	}
	else
		cb_coords_grid->setColor( tmp_color );

	tmp_color.setRgb( 0, 0, 0 );
	if ( kc->hasGroup( "Axes" ) )
	{
		kc->setGroup( "Axes" );
		cb_coords_axes->setColor( kc->readColorEntry( "Color", &tmp_color ) );
	}
	else
	{
		cb_coords_axes->setColor( tmp_color );
	}

	if ( kc->hasGroup( "Graphs" ) )
	{
		kc->setGroup( "Graphs" );
		tmp_color.setRgb( 255, 0, 0 );
		cb_functions0->setColor( kc->readColorEntry( "Color0", &tmp_color ) );
		tmp_color.setRgb( 0, 255, 0 );
		cb_functions1->setColor( kc->readColorEntry( "Color1", &tmp_color ) );
		tmp_color.setRgb( 0, 0, 255 );
		cb_functions2->setColor( kc->readColorEntry( "Color2", &tmp_color ) );
		tmp_color.setRgb( 255, 0, 255 );
		cb_functions3->setColor( kc->readColorEntry( "Color3", &tmp_color ) );
		tmp_color.setRgb( 255, 255, 0 );
		cb_functions4->setColor( kc->readColorEntry( "Color4", &tmp_color ) );
		tmp_color.setRgb( 0, 255, 255 );
		cb_functions5->setColor( kc->readColorEntry( "Color5", &tmp_color ) );
		tmp_color.setRgb( 0, 128, 0 );
		cb_functions6->setColor( kc->readColorEntry( "Color6", &tmp_color ) );
		tmp_color.setRgb( 0, 0, 128 );
		cb_functions7->setColor( kc->readColorEntry( "Color7", &tmp_color ) );
		tmp_color.setRgb( 0, 0, 0 );
		cb_functions8->setColor( kc->readColorEntry( "Color8", &tmp_color ) );
		cb_functions9->setColor( kc->readColorEntry( "Color9", &tmp_color ) );
	}
	else
	{
		tmp_color.setRgb( 255, 0, 0 );
		cb_functions0->setColor( tmp_color );
		tmp_color.setRgb( 0, 255, 0 );
		cb_functions1->setColor( tmp_color );
		tmp_color.setRgb( 0, 0, 255 );
		cb_functions2->setColor( tmp_color );
		tmp_color.setRgb( 255, 0, 255 );
		cb_functions3->setColor( tmp_color );
		tmp_color.setRgb( 255, 255, 0 );
		cb_functions4->setColor( tmp_color );
		tmp_color.setRgb( 0, 255, 255 );
		cb_functions5->setColor( tmp_color );
		tmp_color.setRgb( 0, 128, 0 );
		cb_functions6->setColor( tmp_color );
		tmp_color.setRgb( 0, 0, 128 );
		cb_functions7->setColor( tmp_color );
		tmp_color.setRgb( 0, 0, 0 );
		cb_functions8->setColor( tmp_color );
		cb_functions9->setColor( tmp_color );
	}
}

void KColorPage::applySettings()
{
	kc->setGroup( "Grid" );
	kc->writeEntry( "Color", cb_coords_grid->color() );

	kc->setGroup( "Axes" );
	kc->writeEntry( "Color", cb_coords_axes->color() );

	kc->setGroup( "Graphs" );
	kc->writeEntry( "Color0", cb_functions0->color() );
	kc->writeEntry( "Color1", cb_functions1->color() );
	kc->writeEntry( "Color2", cb_functions2->color() );
	kc->writeEntry( "Color3", cb_functions3->color() );
	kc->writeEntry( "Color4", cb_functions4->color() );
	kc->writeEntry( "Color5", cb_functions5->color() );
	kc->writeEntry( "Color6", cb_functions6->color() );
	kc->writeEntry( "Color7", cb_functions7->color() );
	kc->writeEntry( "Color8", cb_functions8->color() );
	kc->writeEntry( "Color9", cb_functions9->color() );

	kc->sync();
}

void KColorPage::defaults()
{
	QColor tmp_color;
	tmp_color.setRgb( 192, 192, 192 );
	cb_coords_grid->setColor( tmp_color );

	tmp_color.setRgb( 0, 0, 0 );
	cb_coords_axes->setColor( tmp_color );

	tmp_color.setRgb( 255, 0, 0 );
	cb_functions0->setColor( tmp_color );
	tmp_color.setRgb( 0, 255, 0 );
	cb_functions1->setColor( tmp_color );
	tmp_color.setRgb( 0, 0, 255 );
	cb_functions2->setColor( tmp_color );
	tmp_color.setRgb( 255, 0, 255 );
	cb_functions3->setColor( tmp_color );
	tmp_color.setRgb( 255, 255, 0 );
	cb_functions4->setColor( tmp_color );
	tmp_color.setRgb( 0, 255, 255 );
	cb_functions5->setColor( tmp_color );
	tmp_color.setRgb( 0, 128, 0 );
	cb_functions6->setColor( tmp_color );
	tmp_color.setRgb( 0, 0, 128 );
	cb_functions7->setColor( tmp_color );
	tmp_color.setRgb( 0, 0, 0 );
	cb_functions8->setColor( tmp_color );
	cb_functions9->setColor( tmp_color );
}


KAxesTab::KAxesTab( QWidget* parent, const char* name )
		: QWidget( parent, name )
{
	KAxesTabLayout = new QVBoxLayout( this, 11, 6, "KAxesTabLayout" );

	Layout17 = new QHBoxLayout( 0, 0, 6, "Layout17" );

	bg_xachse = new QButtonGroup( this, "bg_xachse" );
	bg_xachse->setTitle( i18n( "X-Axis" ) );
	bg_xachse->setColumnLayout( 0, Qt::Vertical );
	bg_xachse->layout() ->setSpacing( 6 );
	bg_xachse->layout() ->setMargin( 11 );
	bg_xachseLayout = new QVBoxLayout( bg_xachse->layout() );
	bg_xachseLayout->setAlignment( Qt::AlignTop );

	rb_x1 = new QRadioButton( bg_xachse, "rb_x1" );
	rb_x1->setText( i18n( "[ -8 | +8 ]" ) );
	bg_xachseLayout->addWidget( rb_x1 );

	rb_x2 = new QRadioButton( bg_xachse, "rb_x2" );
	rb_x2->setText( i18n( "[ -5 | +5 ]" ) );
	bg_xachse->insert( rb_x2, 1 );
	bg_xachseLayout->addWidget( rb_x2 );

	rb_x3 = new QRadioButton( bg_xachse, "rb_x3" );
	rb_x3->setText( i18n( "[ 0 | +16 ]" ) );
	bg_xachse->insert( rb_x3, 2 );
	bg_xachseLayout->addWidget( rb_x3 );

	rb_x4 = new QRadioButton( bg_xachse, "rb_x4" );
	rb_x4->setText( i18n( "[ 0 | +10 ]" ) );
	bg_xachse->insert( rb_x4, 3 );
	bg_xachseLayout->addWidget( rb_x4 );

	rb_x5 = new QRadioButton( bg_xachse, "rb_x5" );
	rb_x5->setText( i18n( "Custom" ) );
	bg_xachseLayout->addWidget( rb_x5 );

	Layout6 = new QVBoxLayout( 0, 0, 6, "Layout6" );

	Layout4 = new QHBoxLayout( 0, 0, 6, "Layout4" );

	TextLabel1 = new QLabel( bg_xachse, "TextLabel1" );
	TextLabel1->setText( i18n( "min:" ) );
	TextLabel1->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
	Layout4->addWidget( TextLabel1 );

	le_xmin = new QLineEdit( bg_xachse, "le_xmin" );
	le_xmin->setSizePolicy( QSizePolicy( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, le_xmin->sizePolicy().hasHeightForWidth() ) );
	Layout4->addWidget( le_xmin );
	Layout6->addLayout( Layout4 );

	Layout5 = new QHBoxLayout( 0, 0, 6, "Layout5" );

	TextLabel1_2 = new QLabel( bg_xachse, "TextLabel1_2" );
	TextLabel1_2->setText( i18n( "max:" ) );
	TextLabel1_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
	Layout5->addWidget( TextLabel1_2 );

	le_xmax = new QLineEdit( bg_xachse, "le_xmax" );
	le_xmax->setSizePolicy( QSizePolicy( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, le_xmax->sizePolicy().hasHeightForWidth() ) );
	Layout5->addWidget( le_xmax );
	Layout6->addLayout( Layout5 );
	bg_xachseLayout->addLayout( Layout6 );
	Layout17->addWidget( bg_xachse );

	bg_yachse = new QButtonGroup( this, "bg_yachse" );
	bg_yachse->setTitle( i18n( "Y-Axis" ) );
	bg_yachse->setColumnLayout( 0, Qt::Vertical );
	bg_yachse->layout() ->setSpacing( 6 );
	bg_yachse->layout() ->setMargin( 11 );
	bg_yachseLayout = new QVBoxLayout( bg_yachse->layout() );
	bg_yachseLayout->setAlignment( Qt::AlignTop );

	rb_y1 = new QRadioButton( bg_yachse, "rb_y1" );
	rb_y1->setText( i18n( "[ -8 | +8 ]" ) );
	bg_yachse->insert( rb_y1, 0 );
	bg_yachseLayout->addWidget( rb_y1 );

	rb_y2 = new QRadioButton( bg_yachse, "rb_y2" );
	rb_y2->setText( i18n( "[ -5 | +5 ]" ) );
	bg_yachse->insert( rb_y2, 1 );
	bg_yachseLayout->addWidget( rb_y2 );

	rb_y3 = new QRadioButton( bg_yachse, "rb_y3" );
	rb_y3->setText( i18n( "[ 0 | +16 ]" ) );
	bg_yachse->insert( rb_y3, 2 );
	bg_yachseLayout->addWidget( rb_y3 );

	rb_y4 = new QRadioButton( bg_yachse, "rb_y4" );
	rb_y4->setText( i18n( "[ 0 | +10 ]" ) );
	bg_yachse->insert( rb_y4, 3 );
	bg_yachseLayout->addWidget( rb_y4 );

	rb_y5 = new QRadioButton( bg_yachse, "rb_y5" );
	rb_y5->setText( i18n( "Custom" ) );
	bg_yachseLayout->addWidget( rb_y5 );

	Layout3 = new QVBoxLayout( 0, 0, 6, "Layout3" );

	Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1" );

	TextLabel1_3 = new QLabel( bg_yachse, "TextLabel1_3" );
	TextLabel1_3->setText( i18n( "min:" ) );
	TextLabel1_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
	Layout1->addWidget( TextLabel1_3 );

	le_ymin = new QLineEdit( bg_yachse, "le_ymin" );
	le_ymin->setSizePolicy( QSizePolicy( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, le_ymin->sizePolicy().hasHeightForWidth() ) );
	Layout1->addWidget( le_ymin );
	Layout3->addLayout( Layout1 );

	Layout2 = new QHBoxLayout( 0, 0, 6, "Layout2" );

	TextLabel1_2_2 = new QLabel( bg_yachse, "TextLabel1_2_2" );
	TextLabel1_2_2->setText( i18n( "max:" ) );
	TextLabel1_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
	Layout2->addWidget( TextLabel1_2_2 );

	le_ymax = new QLineEdit( bg_yachse, "le_ymax" );
	le_ymax->setSizePolicy( QSizePolicy( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, le_ymax->sizePolicy().hasHeightForWidth() ) );
	Layout2->addWidget( le_ymax );
	Layout3->addLayout( Layout2 );
	bg_yachseLayout->addLayout( Layout3 );
	Layout17->addWidget( bg_yachse );
	KAxesTabLayout->addLayout( Layout17 );

	Layout16 = new QGridLayout( 0, 1, 1, 0, 6, "Layout16" );

	TextLabel1_2_3 = new QLabel( this, "TextLabel1_2_3" );
	TextLabel1_2_3->setText( i18n( "&Tic width:" ) );

	Layout16->addWidget( TextLabel1_2_3, 2, 0 );

	Layout13 = new QHBoxLayout( 0, 0, 6, "Layout13" );

	le_td = new QLineEdit( this, "le_td" );
	le_td->setSizePolicy( QSizePolicy( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, le_td->sizePolicy().hasHeightForWidth() ) );
	le_td->setMinimumSize( QSize( 80, 0 ) );
	QToolTip::add
		( le_td, i18n( "Tic mark line thickness:" ) );
	Layout13->addWidget( le_td );

	TextLabel2_2 = new QLabel( this, "TextLabel2_2" );
	TextLabel2_2->setText( i18n( "0.1mm" ) );
	Layout13->addWidget( TextLabel2_2 );

	Layout16->addLayout( Layout13, 3, 0 );

	TextLabel1_2_2_2 = new QLabel( this, "TextLabel1_2_2_2" );
	TextLabel1_2_2_2->setText( i18n( "Tic &length:" ) );

	Layout16->addWidget( TextLabel1_2_2_2, 2, 1 );

	Layout14 = new QHBoxLayout( 0, 0, 6, "Layout14" );

	le_tl = new QLineEdit( this, "le_tl" );
	le_tl->setSizePolicy( QSizePolicy( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, le_tl->sizePolicy().hasHeightForWidth() ) );
	le_tl->setMinimumSize( QSize( 80, 0 ) );
	QToolTip::add
		( le_tl, i18n( "Tic mark line length" ) );
	Layout14->addWidget( le_tl );

	TextLabel2_2_2 = new QLabel( this, "TextLabel2_2_2" );
	TextLabel2_2_2->setText( i18n( "0.1mm" ) );
	Layout14->addWidget( TextLabel2_2_2 );

	Layout16->addLayout( Layout14, 3, 1 );

	TextLabel1_4 = new QLabel( this, "TextLabel1_4" );
	TextLabel1_4->setText( i18n( "&Axes line width:" ) );

	Layout16->addWidget( TextLabel1_4, 0, 0 );

	Layout12 = new QHBoxLayout( 0, 0, 6, "Layout12" );

	le_ad = new QLineEdit( this, "le_ad" );
	le_ad->setSizePolicy( QSizePolicy( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, le_ad->sizePolicy().hasHeightForWidth() ) );
	le_ad->setMinimumSize( QSize( 80, 0 ) );
	QToolTip::add
		( le_ad, i18n( "Thickness of the axes line" ) );
	Layout12->addWidget( le_ad );

	TextLabel2 = new QLabel( this, "TextLabel2" );
	TextLabel2->setText( i18n( "0.1mm" ) );
	Layout12->addWidget( TextLabel2 );

	Layout16->addLayout( Layout12, 1, 0 );

	cb_beschr = new QCheckBox( this, "cb_beschr" );
	cb_beschr->setText( i18n( "&Labels" ) );

	Layout16->addWidget( cb_beschr, 0, 1 );
	KAxesTabLayout->addLayout( Layout16 );
	QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
	KAxesTabLayout->addItem( spacer );

	// buddies
	TextLabel1_2_3->setBuddy( le_td );
	TextLabel1_2_2_2->setBuddy( le_tl );
	TextLabel1_4->setBuddy( le_ad );
}

KGridTab::KGridTab( QWidget* parent, const char* name )
		: QWidget( parent, name )
{
	KGridTabLayout = new QVBoxLayout( this, 11, 6, "KGridTabLayout" );

	bg_raster = new QButtonGroup( this, "bg_raster" );
	//    bg_raster->setFrameShape( QButtonGroup::Panel );
	bg_raster->setTitle( i18n( "Grid Style" ) );
	bg_raster->setColumnLayout( 0, Qt::Vertical );
	bg_raster->layout() ->setSpacing( 6 );
	bg_raster->layout() ->setMargin( 11 );
	bg_rasterLayout = new QVBoxLayout( bg_raster->layout() );
	bg_rasterLayout->setAlignment( Qt::AlignTop );

	rb_r1 = new QRadioButton( bg_raster, "rb_r1" );
	rb_r1->setText( i18n( "No grid" ) );
	bg_raster->insert( rb_r1, 0 );
	bg_rasterLayout->addWidget( rb_r1 );

	rb_r2 = new QRadioButton( bg_raster, "rb_r2" );
	rb_r2->setText( i18n( "Lines" ) );
	bg_raster->insert( rb_r2, 1 );
	bg_rasterLayout->addWidget( rb_r2 );

	rb_r3 = new QRadioButton( bg_raster, "rb_r3" );
	rb_r3->setText( i18n( "Crosses" ) );
	bg_raster->insert( rb_r3, 2 );
	bg_rasterLayout->addWidget( rb_r3 );

	rb_r4 = new QRadioButton( bg_raster, "rb_r4" );
	rb_r4->setText( i18n( "Polar grid" ) );
	bg_raster->insert( rb_r4, 3 );
	bg_rasterLayout->addWidget( rb_r4 );
	KGridTabLayout->addWidget( bg_raster );

	TextLabel1 = new QLabel( this, "TextLabel1" );
	TextLabel1->setText( i18n( "Line width:" ) );
	KGridTabLayout->addWidget( TextLabel1 );

	Layout20 = new QHBoxLayout( 0, 0, 6, "Layout20" );

	le_dicke = new QLineEdit( this, "le_dicke" );
	le_dicke->setSizePolicy( QSizePolicy( ( QSizePolicy::SizeType ) 0, ( QSizePolicy::SizeType ) 0, 0, 0, le_dicke->sizePolicy().hasHeightForWidth() ) );
	Layout20->addWidget( le_dicke );

	TextLabel2 = new QLabel( this, "TextLabel2" );
	TextLabel2->setText( "0.1mm" );
	Layout20->addWidget( TextLabel2 );
	QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	Layout20->addItem( spacer );
	KGridTabLayout->addLayout( Layout20 );
	QSpacerItem* spacer_2 = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
	KGridTabLayout->addItem( spacer_2 );
}

KFontTab::KFontTab( QWidget* parent, const char* name )
		: QWidget( parent, name )
{
	KFontTabLayout = new QGridLayout( this, 1, 1, 11, 6, "KFontTabLayout" );

	label_axes = new QLabel( this, "label_axes" );
	label_axes->setText( i18n( "&Axes labels:" ) );

	KFontTabLayout->addWidget( label_axes, 1, 0 );

	fc_axes = new KFontCombo( this, "fc_axes" );

	KFontTabLayout->addWidget( fc_axes, 1, 1 );

	fc_header = new KFontCombo( this, "fc_header" );

	KFontTabLayout->addWidget( fc_header, 0, 1 );

	label_header = new QLabel( this, "label_header" );
	label_header->setText( i18n( "Header &table:" ) );

	KFontTabLayout->addWidget( label_header, 0, 0 );
	QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
	KFontTabLayout->addItem( spacer, 2, 1 );

	// buddies
	label_axes->setBuddy( fc_axes );
	label_header->setBuddy( fc_header );
}


KCoordsPage::KCoordsPage( QWidget *parent, const char *name )
		: QTabWidget ( parent, name )
{
	axes_tab = new KAxesTab( this, "axes_tab" );
	addTab( axes_tab, i18n( "Axes" ) );

	grid_tab = new KGridTab( this, "grid_tab" );
	addTab( grid_tab, i18n( "Grid" ) );

	font_tab = new KFontTab( this, "font_tab" );
	addTab( font_tab, i18n( "Fonts" ) );

	// Load actual defaults from ConfigFile
	loadSettings();
}

void KCoordsPage::loadSettings()
{
	if ( kc->hasGroup( "Grid" ) )
	{
		kc->setGroup( "Grid" );
		grid_tab->le_dicke->setText( QString::number( kc->readNumEntry( "Line Width", 1 ) ) );
		grid_tab->bg_raster->setButton( kc->readNumEntry( "Mode", 1 ) );
	}
	else
	{
		grid_tab->le_dicke->setText( "1" );
		grid_tab->bg_raster->setButton( 1 );
	}

	if ( kc->hasGroup( "Axes" ) )
	{
		kc->setGroup( "Axes" );

		axes_tab->bg_xachse->setButton( kc->readNumEntry( "XCoord", 0 ) );
		axes_tab->bg_yachse->setButton( kc->readNumEntry( "YCoord", 0 ) );
		axes_tab->le_xmin->setText( kc->readEntry( "Xmin", "-2*pi" ) );
		axes_tab->le_xmax->setText( kc->readEntry( "Xmax", "2*pi" ) );
		axes_tab->le_ymin->setText( kc->readEntry( "Ymin", "-2*pi" ) );
		axes_tab->le_ymax->setText( kc->readEntry( "Ymax", "2*pi" ) );

		axes_tab->cb_beschr->setChecked( kc->readBoolEntry( "Labeled", true ) );

		axes_tab->le_ad->setText( QString::number( kc->readNumEntry( "Axes Width", 5 ) ) );
		axes_tab->le_td->setText( QString::number( kc->readNumEntry( "Tic Width", 3 ) ) );
		axes_tab->le_tl->setText( QString::number( kc->readNumEntry( "Tic Length", 10 ) ) );
	}
	else
	{
		axes_tab->bg_xachse->setButton( 0 );
		axes_tab->bg_yachse->setButton( 0 );
		axes_tab->le_xmin->setText( "-2*pi" );
		axes_tab->le_xmax->setText( "2*pi" );
		axes_tab->le_ymin->setText( "-2*pi" );
		axes_tab->le_ymax->setText( "2*pi" );

		axes_tab->cb_beschr->setChecked( true );

		axes_tab->le_ad->setText( "5" );
		axes_tab->le_td->setText( "3" );
		axes_tab->le_tl->setText( "10" );
	}

	if ( kc->hasGroup( "Fonts" ) )
	{
		kc->setGroup( "Fonts" );
		font_tab->fc_header->setCurrentFont( kc->readEntry( "Header Table", "Helvetica" ) );
		font_tab->fc_axes->setCurrentFont( kc->readEntry( "Axes", "Helvetica" ) );
	}
	else
	{
		font_tab->fc_header->setCurrentFont( "Helvetica" );
		font_tab->fc_axes->setCurrentFont( "Helvetica" );
	}

}

void KCoordsPage::applySettings()
{
	kc->setGroup( "Grid" );
	kc->writeEntry( "Line Width", grid_tab->le_dicke->text() );
	int m_id = 0;
	while ( m_id < grid_tab->bg_raster->count() && !grid_tab->bg_raster->find( m_id ) ->isOn() )
		m_id++;
	kc->writeEntry( "Mode", m_id );

	kc->setGroup( "Axes" );
	m_id = 0;
	while ( m_id < axes_tab->bg_xachse->count() && !axes_tab->bg_xachse->find( m_id ) ->isOn() )
		m_id++;
	kc->writeEntry( "XCoord", m_id );
	m_id = 0;
	while ( m_id < axes_tab->bg_yachse->count() && !axes_tab->bg_yachse->find( m_id ) ->isOn() )
		m_id++;
	kc->writeEntry( "YCoord", m_id );

	kc->writeEntry( "Xmin", axes_tab->le_xmin->text() );
	kc->writeEntry( "Xmax", axes_tab->le_xmax->text() );
	kc->writeEntry( "Ymin", axes_tab->le_ymin->text() );
	kc->writeEntry( "Ymax", axes_tab->le_ymax->text() );

	kc->writeEntry( "Labeled", axes_tab->cb_beschr->isChecked() );

	kc->writeEntry( "Axes Width", axes_tab->le_ad->text().toInt() );
	kc->writeEntry( "Tic Width", axes_tab->le_td->text().toInt() );
	kc->writeEntry( "Tic Length", axes_tab->le_tl->text().toInt() );

	kc->setGroup( "Fonts" );
	kc->writeEntry( "Header Table", font_tab->fc_header->currentText() );
	kc->writeEntry( "Axes", font_tab->fc_axes->currentText() );

	kc->sync();
}

void KCoordsPage::defaults()
{
	grid_tab->le_dicke->setText( "1" );
	grid_tab->bg_raster->setButton( 1 );

	axes_tab->bg_xachse->setButton( 0 );
	axes_tab->bg_yachse->setButton( 0 );
	axes_tab->le_xmin->setText( "-2*pi" );
	axes_tab->le_xmax->setText( "2*pi" );
	axes_tab->le_ymin->setText( "-2*pi" );
	axes_tab->le_ymax->setText( "2*pi" );

	axes_tab->cb_beschr->setChecked( true );

	axes_tab->le_ad->setText( "5" );
	axes_tab->le_td->setText( "3" );
	axes_tab->le_tl->setText( "10" );

	font_tab->fc_header->setCurrentFont( "Helvetica" );
	font_tab->fc_axes->setCurrentFont( "Helvetica" );
}

KScalingPage::KScalingPage( QWidget* parent, const char* name )
		: QWidget( parent, name )
{
	KScalingTabLayout = new QVBoxLayout( this, 11, 6, "KScalingTabLayout" );

	Layout11 = new QHBoxLayout( 0, 0, 6, "Layout11" );

	GroupBox1 = new QGroupBox( this, "GroupBox1" );
	GroupBox1->setTitle( i18n( "X-Axis" ) );
	GroupBox1->setColumnLayout( 0, Qt::Vertical );
	GroupBox1->layout() ->setSpacing( 6 );
	GroupBox1->layout() ->setMargin( 11 );
	GroupBox1Layout = new QVBoxLayout( GroupBox1->layout() );
	GroupBox1Layout->setAlignment( Qt::AlignTop );

	TextLabel3_3 = new QLabel( GroupBox1, "TextLabel3_3" );
	TextLabel3_3->setText( i18n( "Scaling:" ) );
	GroupBox1Layout->addWidget( TextLabel3_3 );

	Layout8 = new QHBoxLayout( 0, 0, 6, "Layout8" );

	TextLabel1 = new QLabel( GroupBox1, "TextLabel1" );
	TextLabel1->setSizePolicy( QSizePolicy( ( QSizePolicy::SizeType ) 5, ( QSizePolicy::SizeType ) 5, 0, 0, TextLabel1->sizePolicy().hasHeightForWidth() ) );
	TextLabel1->setText( i18n( "1 tic =" ) );
	TextLabel1->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
	Layout8->addWidget( TextLabel1 );

	cb_xtlg = new QComboBox( FALSE, GroupBox1, "cb_xtlg" );
	cb_xtlg->insertItem( "10" );
	cb_xtlg->insertItem( "5" );
	cb_xtlg->insertItem( "2" );
	cb_xtlg->insertItem( "1" );
	cb_xtlg->insertItem( "0.5" );
	cb_xtlg->insertItem( "pi/3" );
	cb_xtlg->insertItem( "pi/2" );
	cb_xtlg->insertItem( "pi/4" );
	cb_xtlg->setSizePolicy( QSizePolicy( ( QSizePolicy::SizeType ) 1, ( QSizePolicy::SizeType ) 0, 0, 0, cb_xtlg->sizePolicy().hasHeightForWidth() ) );
	cb_xtlg->setEditable( FALSE );
	cb_xtlg->setSizeLimit( 8 );
	Layout8->addWidget( cb_xtlg );
	QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	Layout8->addItem( spacer );
	GroupBox1Layout->addLayout( Layout8 );

	Line1 = new QFrame( GroupBox1, "Line1" );
	Line1->setFrameShape( QFrame::HLine );
	Line1->setFrameShadow( QFrame::Raised );
	Line1->setFrameShape( QFrame::HLine );
	GroupBox1Layout->addWidget( Line1 );

	TextLabel3 = new QLabel( GroupBox1, "TextLabel3" );
	TextLabel3->setText( i18n( "Printing:" ) );
	GroupBox1Layout->addWidget( TextLabel3 );

	Layout7 = new QHBoxLayout( 0, 0, 6, "Layout7" );

	TextLabel1_3 = new QLabel( GroupBox1, "TextLabel1_3" );
	TextLabel1_3->setText( i18n( "1 tic =" ) );
	TextLabel1_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
	Layout7->addWidget( TextLabel1_3 );

	cb_xdruck = new QComboBox( FALSE, GroupBox1, "cb_xdruck" );
	cb_xdruck->insertItem( "10" );
	cb_xdruck->insertItem( "5" );
	cb_xdruck->insertItem( "2" );
	cb_xdruck->insertItem( "1" );
	cb_xdruck->insertItem( "0.5" );
	cb_xdruck->insertItem( "pi/3" );
	cb_xdruck->insertItem( "pi/2" );
	cb_xdruck->insertItem( "pi/4" );
	cb_xdruck->setSizePolicy( QSizePolicy( ( QSizePolicy::SizeType ) 1, ( QSizePolicy::SizeType ) 0, 0, 0, cb_xdruck->sizePolicy().hasHeightForWidth() ) );
	cb_xdruck->setSizeLimit( 8 );
	Layout7->addWidget( cb_xdruck );

	TextLabel5 = new QLabel( GroupBox1, "TextLabel5" );
	TextLabel5->setText( "cm" );
	Layout7->addWidget( TextLabel5 );
	QSpacerItem* spacer_2 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	Layout7->addItem( spacer_2 );
	GroupBox1Layout->addLayout( Layout7 );
	Layout11->addWidget( GroupBox1 );

	GroupBox1_2 = new QGroupBox( this, "GroupBox1_2" );
	GroupBox1_2->setTitle( i18n( "Y-Axis" ) );
	GroupBox1_2->setColumnLayout( 0, Qt::Vertical );
	GroupBox1_2->layout() ->setSpacing( 6 );
	GroupBox1_2->layout() ->setMargin( 11 );
	GroupBox1_2Layout = new QVBoxLayout( GroupBox1_2->layout() );
	GroupBox1_2Layout->setAlignment( Qt::AlignTop );

	TextLabel3_3_2 = new QLabel( GroupBox1_2, "TextLabel3_3_2" );
	TextLabel3_3_2->setText( i18n( "Scaling:" ) );
	GroupBox1_2Layout->addWidget( TextLabel3_3_2 );

	Layout10 = new QHBoxLayout( 0, 0, 6, "Layout10" );

	TextLabel1_2 = new QLabel( GroupBox1_2, "TextLabel1_2" );
	TextLabel1_2->setText( i18n( "1 tic =" ) );
	TextLabel1_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
	Layout10->addWidget( TextLabel1_2 );

	cb_ytlg = new QComboBox( FALSE, GroupBox1_2, "cb_ytlg" );
	cb_ytlg->insertItem( "10" );
	cb_ytlg->insertItem( "5" );
	cb_ytlg->insertItem( "2" );
	cb_ytlg->insertItem( "1" );
	cb_ytlg->insertItem( "0.5" );
	cb_ytlg->insertItem( "pi/3" );
	cb_ytlg->insertItem( "pi/2" );
	cb_ytlg->insertItem( "pi/4" );
	cb_ytlg->setSizeLimit( 8 );
	Layout10->addWidget( cb_ytlg );
	QSpacerItem* spacer_3 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	Layout10->addItem( spacer_3 );
	GroupBox1_2Layout->addLayout( Layout10 );

	Line1_2 = new QFrame( GroupBox1_2, "Line1_2" );
	Line1_2->setFrameShape( QFrame::HLine );
	Line1_2->setFrameShadow( QFrame::Raised );
	Line1_2->setFrameShape( QFrame::HLine );
	GroupBox1_2Layout->addWidget( Line1_2 );

	TextLabel3_2 = new QLabel( GroupBox1_2, "TextLabel3_2" );
	TextLabel3_2->setText( i18n( "Printing:" ) );
	GroupBox1_2Layout->addWidget( TextLabel3_2 );

	Layout9 = new QHBoxLayout( 0, 0, 6, "Layout9" );

	TextLabel1_3_2 = new QLabel( GroupBox1_2, "TextLabel1_3_2" );
	TextLabel1_3_2->setText( i18n( "1 tic =" ) );
	TextLabel1_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
	Layout9->addWidget( TextLabel1_3_2 );

	cb_ydruck = new QComboBox( FALSE, GroupBox1_2, "cb_ydruck" );
	cb_ydruck->insertItem( "10" );
	cb_ydruck->insertItem( "5" );
	cb_ydruck->insertItem( "2" );
	cb_ydruck->insertItem( "1" );
	cb_ydruck->insertItem( "0.5" );
	cb_ydruck->insertItem( "pi/3" );
	cb_ydruck->insertItem( "pi/2" );
	cb_ydruck->insertItem( "pi/4" );
	cb_ydruck->setSizeLimit( 8 );
	Layout9->addWidget( cb_ydruck );

	TextLabel5_2 = new QLabel( GroupBox1_2, "TextLabel5_2" );
	TextLabel5_2->setText( "cm" );
	Layout9->addWidget( TextLabel5_2 );
	QSpacerItem* spacer_4 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	Layout9->addItem( spacer_4 );
	GroupBox1_2Layout->addLayout( Layout9 );
	Layout11->addWidget( GroupBox1_2 );
	KScalingTabLayout->addLayout( Layout11 );
	QSpacerItem* spacer_5 = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
	KScalingTabLayout->addItem( spacer_5 );

	// Load actual defaults from ConfigFile
	loadSettings();
}

void KScalingPage::loadSettings()
{
	if ( kc->hasGroup( "Axes" ) )
	{
		cb_xtlg->setCurrentText( kc->readEntry( "tlgx", "1" ) );
		cb_ytlg->setCurrentText( kc->readEntry( "tlgy", "1" ) );
		cb_xdruck->setCurrentText( kc->readEntry( "drskalx", "1" ) );
		cb_ydruck->setCurrentText( kc->readEntry( "drskaly", "1" ) );
	}
	else
	{
		cb_xtlg->setCurrentText( "1" );
		cb_ytlg->setCurrentText( "1" );
		cb_xdruck->setCurrentText( "1" );
		cb_ydruck->setCurrentText( "1" );
	}
}

void KScalingPage::applySettings()
{
	kc->setGroup( "Axes" );
	kc->writeEntry( "tlgx", cb_xtlg->currentText() );
	kc->writeEntry( "tlgy", cb_ytlg->currentText() );
	kc->writeEntry( "drskalx", cb_xdruck->currentText() );
	kc->writeEntry( "drskaly", cb_ydruck->currentText() );

	kc->sync();
}

void KScalingPage::defaults()
{
	cb_xtlg->setCurrentText( "1" );
	cb_ytlg->setCurrentText( "1" );
	cb_xdruck->setCurrentText( "1" );
	cb_ydruck->setCurrentText( "1" );
}

////////////////
// The Dialog

KSettingsDlg::KSettingsDlg( QWidget *parent, const char *name )
		: KDialogBase( IconList, i18n( "Default Settings" ), Help | Default | Ok | Apply | Cancel , Ok, parent, name, true, false )
{

	// Colors
	frame_color = addPage( i18n( "Colors" ), i18n( "Customize Color Settings" ), BarIcon( "colorize", KIcon::SizeMedium ) );
	color_page = new KColorPage( frame_color, "color_page" );
	QVBoxLayout *layout_color = new QVBoxLayout( frame_color );
	layout_color->addWidget( color_page );

	// Coords
	frame_coords = addPage( i18n( "Coords" ), i18n( "Customize Settings for Axes & Grid" ), BarIcon( "coords", KIcon::SizeMedium ) );
	coords_page = new KCoordsPage( frame_coords, "coords_page" );
	QVBoxLayout *layout_coords = new QVBoxLayout( frame_coords );
	layout_coords->addWidget( coords_page );

	// Scaling
	frame_scaling = addPage( i18n( "Scaling" ), i18n( "Customize Scaling Settings" ), BarIcon( "scaling", KIcon::SizeMedium ) );
	scaling_page = new KScalingPage( frame_scaling, "scaling_page" );
	QVBoxLayout *layout_scaling = new QVBoxLayout( frame_scaling );
	layout_scaling->addWidget( scaling_page );

	color_page->loadSettings();
	coords_page->loadSettings();
	scaling_page->loadSettings();
}

void KSettingsDlg::slotApply()
{
	color_page->applySettings();
	coords_page->applySettings();
	scaling_page->applySettings();
}


void KSettingsDlg::slotOk()
{
	slotApply();
	KDialogBase::slotOk();
}

void KSettingsDlg::slotDefault()
{
	color_page->defaults();
	coords_page->defaults();
	scaling_page->defaults();
}

#include "ksettingsdlg.moc"
