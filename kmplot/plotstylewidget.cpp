/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>
*               
* This file is part of the KDE Project.
* KmPlot is part of the KDE-EDU Project.
*
* SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "plotstylewidget.h"
#include "kgradientdialog.h"
#include "ui_plotstylewidget.h"

#include <KColorButton>

#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>


class PlotStyleDialogWidget : public QWidget, public Ui::PlotStyleWidget
{
	public:
		PlotStyleDialogWidget( QWidget * parent = 0 )
	: QWidget( parent )
		{
			setupUi(this);
			lineStyle->addItem( i18n("Solid"), int(Qt::SolidLine) );
			lineStyle->addItem( i18n("Dash"), int(Qt::DashLine) );
			lineStyle->addItem( i18n("Dot"), int(Qt::DotLine) );
			lineStyle->addItem( i18n("Dash Dot"), int(Qt::DashDotLine) );
			lineStyle->addItem( i18n("Dash Dot Dot"), int(Qt::DashDotDotLine) );
		}
};


//BEGIN class PlotStyleWidget
PlotStyleWidget::PlotStyleWidget( QWidget * parent )
	: QGroupBox( parent )
{
	m_color = new KColorButton( this );
	QPushButton *advancedButton = new QPushButton( this );
	advancedButton->setText( i18n("Advanced...") );
	connect(advancedButton, &QPushButton::clicked, this, &PlotStyleWidget::advancedOptions);
	
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget( new QLabel( i18n("Color:"), this ) );
	layout->addWidget( m_color );
	layout->addStretch( 1 );
	layout->addWidget( advancedButton );
	setLayout(layout);
	
	m_dialog = new QDialog( this );
	QVBoxLayout *mainLayout = new QVBoxLayout;
	m_dialog->setLayout(mainLayout);
	m_dialog->setWindowTitle( i18nc("@title:window", "Plot Appearance") );

	m_dialogWidget = new PlotStyleDialogWidget( m_dialog );
	m_dialogWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
	mainLayout->addWidget(m_dialogWidget);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
	QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setDefault(true);
	okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
	connect(buttonBox, &QDialogButtonBox::accepted, m_dialog, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, m_dialog, &QDialog::reject);
	mainLayout->addWidget(buttonBox);
}


void PlotStyleWidget::init( const PlotAppearance & plot, Function::Type type )
{
	m_dialogWidget->gradientButton->setGradient( plot.gradient );
	m_dialogWidget->lineWidth->setValue( plot.lineWidth );
	m_color->setColor( plot.color );
	m_dialogWidget->useGradient->setChecked( plot.useGradient );
	setStyle( plot.style );
	m_dialogWidget->showExtrema->setChecked( plot.showExtrema );
	m_dialogWidget->showPlotName->setChecked( plot.showPlotName );
	
	// Show/hide stuff as appropriate
	m_dialogWidget->showExtrema->setVisible( type == Function::Cartesian );
	m_dialogWidget->showTangentField->setVisible( type == Function::Differential );
	
	layout()->invalidate();
	resize( layout()->minimumSize() );
}


PlotAppearance PlotStyleWidget::plot( bool visible )
{
	PlotAppearance p;
	p.lineWidth =  m_dialogWidget->lineWidth->value();
	p.color = m_color->color();
	p.useGradient = m_dialogWidget->useGradient->isChecked();
	p.showTangentField = m_dialogWidget->showTangentField->isChecked();
	p.gradient = m_dialogWidget->gradientButton->gradient();
	p.visible = visible;
	p.style = style();
	p.showExtrema = m_dialogWidget->showExtrema->isChecked();
	p.showPlotName = m_dialogWidget->showPlotName->isChecked();
	return p;
}


Qt::PenStyle PlotStyleWidget::style( ) const
{
	return (Qt::PenStyle)m_dialogWidget->lineStyle->itemData( m_dialogWidget->lineStyle->currentIndex() ).toInt();
}


void PlotStyleWidget::setStyle( Qt::PenStyle style )
{
	m_dialogWidget->lineStyle->setCurrentIndex( m_dialogWidget->lineStyle->findData( int(style) ) );
}


void PlotStyleWidget::advancedOptions( )
{
	m_dialog->show();
}
//END class PlotStyleWidget
