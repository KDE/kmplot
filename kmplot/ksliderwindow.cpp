/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2005  Fredrik Edemar <f_edemar@linux.se>
*               2007  David Saxton <david@bluehaze.org>
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

// local includes
#include "ksliderwindow.h"
#include "view.h"
#include "xparser.h"

#include <KConfigGroup>

#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>

//BEGIN class SliderWidget
SliderWidget::SliderWidget( QWidget *parent, int number )
	: QGroupBox( i18n( "Slider %1", number+1 ), parent )
{
	m_number = number;
	
	setupUi( this );
	
	slider->setToolTip( i18n( "Move slider to change the parameter of the function plot connected to this slider." ) );
	
	KConfig config( "kmplotrc" );
	KConfigGroup group = config.group( "slider" + QString::number(m_number) );
	
	min->setText( group.readEntry( "min", "0" ) );
	max->setText( group.readEntry( "max", "10" ) );
	slider->setValue( group.readEntry( "value", 500 ) );
	
	connect(slider, &QSlider::valueChanged, this, &SliderWidget::updateValue);
	connect(min, &EquationEdit::editingFinished, this, &SliderWidget::updateValue);
	connect(max, &EquationEdit::editingFinished, this, &SliderWidget::updateValue);
	
	updateValue();
}


SliderWidget::~SliderWidget()
{
	KConfig config( "kmplotrc" );
	KConfigGroup group = config.group( "slider" + QString::number(m_number) );
	
	group.writeEntry( "min", min->text() );
	group.writeEntry( "max", max->text() );
	group.writeEntry( "value", slider->value() );
}


void SliderWidget::updateValue()
{
	valueLabel->setText( View::self()->posToString( value(), 0.001*(max->value() - min->value()), View::DecimalFormat ) );
	emit valueChanged();
}


double SliderWidget::value()
{
	double prop = double(slider->value() - slider->minimum()) / double(slider->maximum() - slider->minimum());
	return prop * (max->value() - min->value()) + min->value();
}
//END class SliderWidget



//BEGIN class KSliderWindow
KSliderWindow::KSliderWindow( QWidget * parent ) :
	QDialog( parent )
{
	setModal( false );
	QWidget * widget = new QWidget( this );
	setWindowTitle( i18n("Sliders") );

	QVBoxLayout *layout = new QVBoxLayout( this );
	layout->addWidget(widget);

	Q_ASSERT( SLIDER_COUNT == 4 ); // safety check, in case SLIDER_COUNT is increased but not this code

	for ( int i = 0; i < SLIDER_COUNT; ++i )
	{
		m_sliders[i] = new SliderWidget( widget, i );
		connect( m_sliders[i], &SliderWidget::valueChanged, this, &KSliderWindow::valueChanged );
		layout->addWidget( m_sliders[i] );
	}
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &KSliderWindow::reject);
	layout->addWidget(buttonBox);

	resize( layout->minimumSize() );
}

KSliderWindow::~KSliderWindow()
{
}


double KSliderWindow::value( int slider )
{
	Q_ASSERT( (slider>=0) && (slider < SLIDER_COUNT) );
	return m_sliders[slider]->value();
}


void KSliderWindow::closeEvent( QCloseEvent * e)
{
	emit windowClosed();
	e->accept();
}
//END class KSliderWindow
