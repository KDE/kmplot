/***************************************************************************
       kmplot/ksettingsdlg.h  -  manipulating global KmPlot defaults
                            -------------------
   begin                : 2002-06-23
   email                : bmlmessmer@web.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSETTINGSDIALOG_H
#define KSETTINGSDIALOG_H

#include <qtabwidget.h>

#include <kdialogbase.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QButtonGroup;
class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QVButtonGroup;
class QComboBox;
class QString;

class KColorButton;
class KFontCombo;

class KColorPage : public QTabWidget
{
    Q_OBJECT
		
public:
    KColorPage( QWidget *parent = 0, const char *name = 0 );

	void loadSettings();
	void applySettings();
	void defaults();
private:
    QWidget
    *coords, *functions;
    QLabel
    *l_coords_axes, *l_coords_grid,
    *l_functions0, *l_functions1, *l_functions2, *l_functions3, *l_functions4,
    *l_functions5, *l_functions6, *l_functions7, *l_functions8, *l_functions9;
    KColorButton
    *cb_coords_axes, *cb_coords_grid,
    *cb_functions0, *cb_functions1, *cb_functions2, *cb_functions3, *cb_functions4,
    *cb_functions5, *cb_functions6, *cb_functions7, *cb_functions8, *cb_functions9;
};

class KAxesTab : public QWidget
{
    Q_OBJECT

public:
    KAxesTab( QWidget* parent = 0, const char* name = 0 );

    QButtonGroup* bg_xachse;
    QRadioButton* rb_x1;
    QRadioButton* rb_x2;
    QRadioButton* rb_x3;
    QRadioButton* rb_x4;
    QRadioButton* rb_x5;
    QLabel* TextLabel1;
    QLineEdit* le_xmin;
    QLabel* TextLabel1_2;
    QLineEdit* le_xmax;
    QButtonGroup* bg_yachse;
    QRadioButton* rb_y1;
    QRadioButton* rb_y2;
    QRadioButton* rb_y3;
    QRadioButton* rb_y4;
    QRadioButton* rb_y5;
    QLabel* TextLabel1_3;
    QLineEdit* le_ymin;
    QLabel* TextLabel1_2_2;
    QLineEdit* le_ymax;
    QCheckBox* cb_beschr;
    QLabel* TextLabel1_2_3;
    QLineEdit* le_tl;
    QLabel* TextLabel2_2_2;
    QLineEdit* le_td;
    QLabel* TextLabel2_2;
    QLineEdit* le_ad;
    QLabel* TextLabel2;
    QLabel* TextLabel1_2_2_2;
    QLabel* TextLabel1_4;


protected:
    QVBoxLayout* KAxesTabLayout;
    QHBoxLayout* Layout17;
    QVBoxLayout* bg_xachseLayout;
    QVBoxLayout* Layout6;
    QHBoxLayout* Layout4;
    QHBoxLayout* Layout5;
    QVBoxLayout* bg_yachseLayout;
    QVBoxLayout* Layout3;
    QHBoxLayout* Layout1;
    QHBoxLayout* Layout2;
    QGridLayout* Layout16;
    QHBoxLayout* Layout13;
    QHBoxLayout* Layout14;
    QHBoxLayout* Layout12;
};

class KGridTab : public QWidget
{
    Q_OBJECT

public:
    KGridTab( QWidget* parent = 0, const char* name = 0 );

    QButtonGroup* bg_raster;
    QRadioButton* rb_r1;
    QRadioButton* rb_r2;
    QRadioButton* rb_r3;
    QRadioButton* rb_r4;
    QLabel* TextLabel1;
    QLineEdit* le_dicke;
    QLabel* TextLabel2;

protected:
    QVBoxLayout* KGridTabLayout;
    QVBoxLayout* bg_rasterLayout;
    QHBoxLayout* Layout20;
};

class KFontTab : public QWidget
{ 
    Q_OBJECT

public:
    KFontTab( QWidget* parent = 0, const char* name = 0 );

    QLabel* label_axes;
    KFontCombo* fc_axes;
    KFontCombo* fc_header;
    QLabel* label_header;


protected:
    QGridLayout* KFontTabLayout;
};

class KCoordsPage : public QTabWidget
{
    Q_OBJECT
		
public:
    KCoordsPage( QWidget *parent = 0, const char *name = 0 );

	void loadSettings();
	void applySettings();
	void defaults();
	
private:
    KAxesTab *axes_tab;
    KGridTab *grid_tab;
	KFontTab *font_tab;
};

class KScalingPage : public QWidget
{
    Q_OBJECT
		
public:
    KScalingPage( QWidget* parent = 0, const char* name = 0 );

	void loadSettings();
	void applySettings();
	void defaults();
	
    QGroupBox* GroupBox1;
    QLabel* TextLabel3_3;
    QLabel* TextLabel1;
    QComboBox* cb_xtlg;
    QFrame* Line1;
    QLabel* TextLabel3;
    QLabel* TextLabel1_3;
    QComboBox* cb_xdruck;
    QLabel* TextLabel5;
    QGroupBox* GroupBox1_2;
    QLabel* TextLabel3_3_2;
    QLabel* TextLabel1_2;
    QComboBox* cb_ytlg;
    QFrame* Line1_2;
    QLabel* TextLabel3_2;
    QLabel* TextLabel1_3_2;
    QComboBox* cb_ydruck;
    QLabel* TextLabel5_2;

protected:
    QVBoxLayout* KScalingTabLayout;
    QHBoxLayout* Layout11;
    QVBoxLayout* GroupBox1Layout;
    QHBoxLayout* Layout8;
    QHBoxLayout* Layout7;
    QVBoxLayout* GroupBox1_2Layout;
    QHBoxLayout* Layout10;
    QHBoxLayout* Layout9;
};

class KSettingsDlg : public KDialogBase
{
    Q_OBJECT
		
public:
    KSettingsDlg( QWidget *parent = 0, const char *name = 0 );

private:
    QFrame
    *frame_color, *frame_coords, *frame_scaling;
    KColorPage *color_page;
    KCoordsPage *coords_page;
    KScalingPage *scaling_page;

protected slots:
    void slotApply();
    void slotDefault();
    void slotOk();
};

#endif
