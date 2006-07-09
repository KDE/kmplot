/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998-2002  Klaus-Dieter M�ler <kd.moeller@t-online.de>
*                    2004  Fredrik Edemar <f_edemar@linux.se>
*                    2006  David Saxton <david@bluehaze.org>
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

#include "equationedit.h"
#include "functioneditor.h"
#include "kmplotio.h"
#include "maindlg.h"
#include "parameterswidget.h"
#include "ui_functioneditorwidget.h"
#include "view.h"
#include "xparser.h"

#include <kaction.h>
#include <kcolorbutton.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QMenu>
#include <QRadioButton>
#include <QTimer>

#include <assert.h>

class FunctionEditorWidget : public QWidget, public Ui::FunctionEditorWidget
{
	public:
		FunctionEditorWidget(QWidget *parent = 0)
	: QWidget(parent)
		{ setupUi(this); }
};



//BEGIN class FunctionEditor
FunctionEditor::FunctionEditor( KMenu * createNewPlotsMenu, QWidget * parent )
	: QDockWidget( i18n("Function Editor"), parent )
{
	m_functionID = -1;
	m_createNewPlotsMenu = createNewPlotsMenu;
	
	// need a name for saving and restoring the position of this dock widget
	setObjectName( "FunctionEditor" );
	
	setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
	setFeatures( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable );
	
	m_saveCartesianTimer = new QTimer( this );
	m_savePolarTimer = new QTimer( this );
	m_saveParametricTimer = new QTimer( this );
	m_saveImplicitTimer = new QTimer( this );
	m_saveDifferentialTimer = new QTimer( this );
	m_syncFunctionListTimer = new QTimer( this );
	
	m_saveCartesianTimer->setSingleShot( true );
	m_savePolarTimer->setSingleShot( true );
	m_saveParametricTimer->setSingleShot( true );
	m_saveImplicitTimer->setSingleShot( true );
	m_saveDifferentialTimer->setSingleShot( true );
	m_syncFunctionListTimer->setSingleShot( true );
	
	connect( m_saveCartesianTimer, SIGNAL(timeout()), this, SLOT( saveCartesian() ) );
	connect( m_savePolarTimer, SIGNAL(timeout()), this, SLOT( savePolar() ) );
	connect( m_saveParametricTimer, SIGNAL(timeout()), this, SLOT( saveParametric() ) );
	connect( m_saveImplicitTimer, SIGNAL(timeout()), this, SLOT( saveImplicit() ) );
	connect( m_saveDifferentialTimer, SIGNAL(timeout()), this, SLOT( saveDifferential() ) );
	connect( m_syncFunctionListTimer, SIGNAL(timeout()), this, SLOT( syncFunctionList() ) );
	
	m_editor = new FunctionEditorWidget;
	m_functionList = m_editor->functionList;
	
	m_editor->cartesianEquation->setInputType( EquationEdit::Function );
	m_editor->polarEquation->setInputType( EquationEdit::Function );
	m_editor->parametricX->setInputType( EquationEdit::Function );
	m_editor->parametricY->setInputType( EquationEdit::Function );
	m_editor->implicitEquation->setInputType( EquationEdit::Function );
	m_editor->differentialEquation->setInputType( EquationEdit::Function );
	m_editor->differentialEquation->setEquationType( Equation::Differential );
	
	for ( unsigned i = 0; i < 5; ++i )
		m_editor->stackedWidget->widget(i)->layout()->setMargin( 0 );
	
	connect( m_editor->deleteButton, SIGNAL(clicked()), this, SLOT(deleteCurrent()) );
	connect( m_functionList, SIGNAL(currentItemChanged( QListWidgetItem *, QListWidgetItem * )), this, SLOT(functionSelected( QListWidgetItem* )) );
	connect( m_functionList, SIGNAL(itemClicked( QListWidgetItem * )), this, SLOT(save()) ); // user might have checked or unchecked the item
	
	//BEGIN connect up all editing widgets
	QList<QLineEdit *> lineEdits = m_editor->findChildren<QLineEdit *>();
	foreach ( QLineEdit * w, lineEdits )
		connect( w, SIGNAL(editingFinished()), this, SLOT(save()) );
	
	QList<EquationEdit *> equationEdits = m_editor->findChildren<EquationEdit *>();
	foreach ( EquationEdit * w, equationEdits )
		connect( w, SIGNAL(editingFinished()), this, SLOT(save()) );
	
	QList<QCheckBox *> checkBoxes = m_editor->findChildren<QCheckBox *>();
	foreach ( QCheckBox * w, checkBoxes )
		connect( w, SIGNAL(stateChanged(int)), this, SLOT(save()) );
	
	QList<KColorButton *> colorButtons = m_editor->findChildren<KColorButton *>();
	foreach ( KColorButton * w, colorButtons )
		connect( w, SIGNAL(changed(const QColor &)), this, SLOT(save()) );
	
	QList<QRadioButton *> radioButtons = m_editor->findChildren<QRadioButton *>();
	foreach ( QRadioButton * w, radioButtons )
		connect( w, SIGNAL(toggled(bool)), this, SLOT(save()) );
	
	QList<QComboBox *> comboBoxes = m_editor->findChildren<QComboBox *>();
	foreach ( QComboBox * w, comboBoxes )
		connect( w, SIGNAL(currentIndexChanged(int)), this, SLOT(save()) );
	
	QList<ParametersWidget *> parameters = m_editor->findChildren<ParametersWidget *>();
	foreach ( ParametersWidget * w, parameters )
		connect( w, SIGNAL( parameterListChanged() ), this, SLOT(save()) );
	
	connect( m_editor->initialConditions, SIGNAL(dataChanged()), this, SLOT(save()) );
	//END connect up all editing widgets
	
	connect( XParser::self(), SIGNAL(functionAdded(int)), this, SLOT(functionsChanged()) );
	connect( XParser::self(), SIGNAL(functionRemoved(int)), this, SLOT(functionsChanged()) );
	
	m_createNewPlotsMenu->installEventFilter( this );
	connect( m_editor->createNewPlot, SIGNAL(pressed()), this, SLOT( createNewPlot() ) );
	
	resetFunctionEditing();
	setWidget( m_editor );
}


FunctionEditor::~ FunctionEditor()
{
}


void FunctionEditor::deleteCurrent()
{
	FunctionListItem * functionItem = static_cast<FunctionListItem*>(m_functionList->currentItem());
	if ( !functionItem )
	{
		kDebug() << "Nothing currently selected!\n";
		return;
	}
	
	if ( !XParser::self()->removeFunction( functionItem->function() ) )
	{
		kDebug() << "Couldn't delete function.\n";
		// couldn't delete it, as e.g. another function depends on it
		return;
	}
	
	MainDlg::self()->requestSaveCurrentState();
	View::self()->drawPlot();
}


void FunctionEditor::functionsChanged()
{
	m_syncFunctionListTimer->start( 0 );
}


void FunctionEditor::syncFunctionList()
{
	int oldFunctionCount = m_functionList->count();
	
	QListWidgetItem * currentItem = m_functionList->currentItem();
	QString currentText = currentItem ? currentItem->text() : QString::null;
	
	// build up a list of IDs that we have
	QMap< int, FunctionListItem * > currentIDs;
	QList< FunctionListItem * > currentFunctionItems;
	for ( int row = 0; row < m_functionList->count(); ++row )
	{
		FunctionListItem * item = static_cast<FunctionListItem*>(m_functionList->item( row ));
		currentFunctionItems << item;
		currentIDs[ item->function() ] = item;
		
		// also update what is displayed
		item->update();
	}
	
	FunctionListItem * toSelect = 0l;
	int newFunctionCount = 0;
	
	for ( QMap<int, Function*>::iterator it = XParser::self()->m_ufkt.begin(); it != XParser::self()->m_ufkt.end(); ++it)
	{
		Function * function = *it;
		
		if ( currentIDs.contains( function->id ) )
		{
			// already have the function
			currentFunctionItems.removeAll( currentIDs[ function->id ] );
			currentIDs.remove( function->id );
			continue;
		}
		
		toSelect = new FunctionListItem( m_functionList, function->id );
		newFunctionCount++;
	}
	
	if ( newFunctionCount != 1 )
	{
		// only select a new functionlistitem if there was precisely one added
		toSelect = 0l;
	}
	
	
	// Now, any IDs left in currentIDs are of functions that have been deleted
	foreach ( FunctionListItem * item, currentFunctionItems )
	{
		if ( m_functionID == item->function() )
			m_functionID = -1;
		
		delete m_functionList->takeItem( m_functionList->row( item ) );
	}
	
	m_functionList->sortItems();
	
	// Try and see if there is an item with the same text as was initially selected, if we have
	// the same number of cuntions
	if ( (oldFunctionCount == m_functionList->count()) && !currentText.isEmpty() )
	{
		QList<QListWidgetItem *> matchedItems = m_functionList->findItems( currentText, Qt::MatchExactly );
		if ( matchedItems.count() == 1 )
			toSelect = static_cast<FunctionListItem*>(matchedItems.first());
	}
	
	if ( toSelect )
		m_functionList->setCurrentItem( toSelect );
	
	if ( m_functionList->count() == 0 )
		resetFunctionEditing();
}


void FunctionEditor::setCurrentFunction( int functionID )
{
	for ( int row = 0; row < m_functionList->count(); ++row )
	{
		FunctionListItem * item = static_cast<FunctionListItem*>(m_functionList->item( row ));
		if ( item->function() != functionID )
			continue;
		
		m_functionList->setCurrentRow( row );
		return;
	}
}


void FunctionEditor::functionSelected( QListWidgetItem * item )
{
	m_editor->deleteButton->setEnabled( item != 0 );
	if ( !item )
		return;
	
	// If there are any pending save events, then cancel them
	m_saveCartesianTimer->stop();
	m_savePolarTimer->stop();
	m_saveParametricTimer->stop();
	m_saveImplicitTimer->stop();
	
	FunctionListItem * functionItem = static_cast<FunctionListItem*>(item);
	
	m_functionID = functionItem->function();
	Function * f = XParser::self()->functionWithID( m_functionID );
	if ( !f )
		return;
	
	switch ( f->type() )
	{
		case Function::Cartesian:
			initFromCartesian();
			break;
			
		case Function::Polar:
			initFromPolar();
			break;
			
		case Function::Parametric:
			initFromParametric();
			break;
			
		case Function::Implicit:
			initFromImplicit();
			break;
			
		case Function::Differential:
			initFromDifferential();
	}
	
	functionItem->update();
}


void FunctionEditor::initFromCartesian()
{
	Function * f = XParser::self()->functionWithID(m_functionID);
	
	if ( !f )
	{
		kWarning() << k_funcinfo << "No f! (id="<<m_functionID<<")\n";
		return;
	}
	
	m_editor->cartesianEquation->setText( f->eq[0]->fstr() );
	m_editor->cartesian_f0->init( f->plotAppearance( Function::Derivative0 ), Function::Cartesian );
	m_editor->cartesian_f1->init( f->plotAppearance( Function::Derivative1 ), Function::Cartesian );
	m_editor->cartesian_f2->init( f->plotAppearance( Function::Derivative2 ), Function::Cartesian );
	m_editor->cartesian_integral->init( f->plotAppearance( Function::Integral ), Function::Cartesian );
	
	m_editor->showDerivative1->setChecked( f->plotAppearance( Function::Derivative1 ).visible );
	m_editor->showDerivative2->setChecked( f->plotAppearance( Function::Derivative2 ).visible );
	
	m_editor->cartesianCustomMin->setChecked( f->usecustomxmin );
	m_editor->cartesianMin->setText( f->dmin.expression() );
	
	m_editor->cartesianCustomMax->setChecked( f->usecustomxmax );
	m_editor->cartesianMax->setText( f->dmax.expression() );
	
	m_editor->cartesianParameters->init( f->m_parameters );
	
	m_editor->showIntegral->setChecked( f->plotAppearance( Function::Integral ).visible );
	m_editor->integralStep->setText( f->eq[0]->differentialStates.step().expression() );
	
	DifferentialState state = f->eq[0]->differentialStates[0];
	m_editor->txtInitX->setText( state.x0.expression() );
	m_editor->txtInitY->setText( state.y0[0].expression() );
	
	m_editor->stackedWidget->setCurrentIndex( 0 );
	m_editor->tabWidget->setCurrentIndex( 0 );
	m_editor->cartesianEquation->setFocus();
}


void FunctionEditor::initFromPolar()
{
	Function * f = XParser::self()->functionWithID(m_functionID);
	
	if ( !f )
		return;
	
	QString function = f->eq[0]->fstr();
	m_editor->polarEquation->setText( function );
	m_editor->polarMin->setText( f->dmin.expression() );
	m_editor->polarMax->setText( f->dmax.expression() );
	m_editor->polar_f0->init( f->plotAppearance( Function::Derivative0 ), Function::Polar );
	
	m_editor->polarParameters->init( f->m_parameters );
	
	m_editor->stackedWidget->setCurrentIndex( 2 );
	m_editor->polarEquation->setFocus();
}


void FunctionEditor::initFromParametric()
{
	Function * f = XParser::self()->functionWithID(m_functionID);
	
	if ( !f )
		return;
	
	m_editor->parametricX->setText( f->eq[0]->fstr() );
	m_editor->parametricY->setText( f->eq[1]->fstr() );

	m_editor->parametricMin->setText( f->dmin.expression() );
	m_editor->parametricMax->setText( f->dmax.expression() );
	
	m_editor->parametricParameters->init( f->m_parameters );
	
	m_editor->parametric_f0->init( f->plotAppearance( Function::Derivative0 ), Function::Parametric );
	
	m_editor->stackedWidget->setCurrentIndex( 1 );
	m_editor->parametricX->setFocus();
}


void FunctionEditor::initFromImplicit()
{
	Function * f = XParser::self()->functionWithID(m_functionID);
	
	if ( !f )
		return;
	
	QString name, expression;
	splitImplicitEquation( f->eq[0]->fstr(), & name, & expression );
	
	m_editor->implicitEquation->setValidatePrefix( name + '=' );
	
	m_editor->implicitName->setText( name );
	m_editor->implicitEquation->setText( expression );
	m_editor->implicit_f0->init( f->plotAppearance( Function::Derivative0 ), Function::Implicit );
	
	m_editor->implicitParameters->init( f->m_parameters );
	
	m_editor->stackedWidget->setCurrentIndex( 3 );
	m_editor->implicitEquation->setFocus();
}


void FunctionEditor::initFromDifferential()
{
	Function * f = XParser::self()->functionWithID(m_functionID);
	
	if ( !f )
		return;
	
	m_editor->differentialEquation->setText( f->eq[0]->fstr());
	m_editor->differentialStep->setText( f->eq[0]->differentialStates.step().expression() );
	
	m_editor->differential_f0->init( f->plotAppearance( Function::Derivative0 ), Function::Differential );
	m_editor->differentialParameters->init( f->m_parameters );
	m_editor->initialConditions->init( f );
	
	m_editor->differentialTabWidget->setCurrentIndex( 0 );
	m_editor->stackedWidget->setCurrentIndex( 4 );
	m_editor->differentialEquation->setFocus();
}


void FunctionEditor::splitImplicitEquation( const QString equation, QString * name, QString * expression )
{
	int equalsPos = equation.indexOf( '=' );
	assert( equalsPos >= 0 );
	*name = equation.left( equalsPos ).trimmed();
	*expression = equation.right( equation.length() - equalsPos - 1 ).trimmed();
}


void FunctionEditor::resetFunctionEditing()
{
	m_functionID = -1;
	
	// page 5 is an empty page
	m_editor->stackedWidget->setCurrentIndex( 5 );
	
	// assume that if there are functions in the list, then one will be selected
	m_editor->deleteButton->setEnabled( m_functionList->count() != 0 );
}


bool FunctionEditor::eventFilter( QObject * obj, QEvent * ev )
{
	if ( (obj != m_createNewPlotsMenu) || (ev->type() != QEvent::MouseButtonRelease) || !m_createNewPlotsMenu->isVisible() )
		return QDockWidget::eventFilter( obj, ev );
	
	m_editor->createNewPlot->setDown( false );
	return false;
}


void FunctionEditor::createNewPlot()
{
	QPoint popupPos = m_editor->createNewPlot->mapToGlobal( QPoint( 0, m_editor->createNewPlot->height() ) );
	m_createNewPlotsMenu->exec( popupPos );
}


void FunctionEditor::createCartesian()
{
	QString fname;
	fname = QString( "%1(x) = 0" ).arg( XParser::self()->findFunctionName( "f", -1 ) );
	m_functionID = XParser::self()->Parser::addFunction( fname, QString(), Function::Cartesian );
	assert( m_functionID != -1 );
	
	MainDlg::self()->requestSaveCurrentState();
	View::self()->drawPlot();
}


void FunctionEditor::createParametric()
{
	QString name = XParser::self()->findFunctionName( "f", -1 );
	m_functionID = XParser::self()->Parser::addFunction( QString("%1_x(t) = 0").arg( name ), QString("%1_y(t) = 0").arg( name ), Function::Parametric ); 
	assert( m_functionID != -1 );
	
	MainDlg::self()->requestSaveCurrentState();
}


void FunctionEditor::createPolar()
{
	QString fname;
	fname = QString( "%1(x) = 0" ).arg( XParser::self()->findFunctionName( "f", -1 ) );
	m_functionID = XParser::self()->Parser::addFunction( fname, QString(), Function::Polar );
	assert( m_functionID != -1 );
	
	MainDlg::self()->requestSaveCurrentState();
}


void FunctionEditor::createImplicit()
{
	QString fname;
	fname = QString( "%1(x,y) = y*sinx + x*cosy = 1" ).arg( XParser::self()->findFunctionName( "f", -1 ) );
	m_functionID = XParser::self()->Parser::addFunction( fname, QString(), Function::Implicit );
	assert( m_functionID != -1 );
	
	MainDlg::self()->requestSaveCurrentState();
}


void FunctionEditor::createDifferential()
{
	QString fname;
	fname = QString( "%1''(x) = -%1" ).arg( XParser::self()->findFunctionName( "f", -1 ) );
	m_functionID = XParser::self()->Parser::addFunction( fname, QString(), Function::Differential );
	assert( m_functionID != -1 );
	
	MainDlg::self()->requestSaveCurrentState();
	View::self()->drawPlot();
}


void FunctionEditor::save()
{
	Function * f = XParser::self()->functionWithID( m_functionID );
	if ( !f )
		return;
	
	switch ( f->type() )
	{
		case Function::Cartesian:
			m_saveCartesianTimer->start( 0 );
			break;
			
		case Function::Polar:
			m_savePolarTimer->start( 0 );
			break;
			
		case Function::Parametric:
			m_saveParametricTimer->start( 0 );
			break;
			
		case Function::Implicit:
			m_saveImplicitTimer->start( 0 );
			break;
			
		case Function::Differential:
			m_saveDifferentialTimer->start( 0 );
			break;
	}
}


void FunctionEditor::saveCartesian()
{
	Function * f = XParser::self()->functionWithID( m_functionID );
	if ( !f )
		return;
	
	FunctionListItem * functionListItem = static_cast<FunctionListItem*>(m_functionList->currentItem());
	
	QString f_str( m_editor->cartesianEquation->text() );
	XParser::self()->fixFunctionName(f_str, Equation::Cartesian, f->id );
	
	//all settings are saved here until we know that no errors have appeared
	Function tempFunction( Function::Cartesian );
	
	tempFunction.usecustomxmin = m_editor->cartesianCustomMin->isChecked();
	bool ok = tempFunction.dmin.updateExpression( m_editor->cartesianMin->text() );
	if ( tempFunction.usecustomxmin && !ok )
		return;
	
	tempFunction.usecustomxmax = m_editor->cartesianCustomMax->isChecked();
	ok = tempFunction.dmax.updateExpression( m_editor->cartesianMax->text() );
	if ( tempFunction.usecustomxmax && !ok )
		return;
	
	if (functionListItem)
		tempFunction.plotAppearance( Function::Derivative0 ) = m_editor->cartesian_f0->plot( (functionListItem->checkState() == Qt::Checked) );
	tempFunction.plotAppearance( Function::Derivative1 ) = m_editor->cartesian_f1->plot( m_editor->showDerivative1->isChecked() );
	tempFunction.plotAppearance( Function::Derivative2 ) = m_editor->cartesian_f2->plot( m_editor->showDerivative2->isChecked() );
	tempFunction.plotAppearance( Function::Integral ) = m_editor->cartesian_integral->plot( m_editor->showIntegral->isChecked() );
	
	DifferentialState * state = & tempFunction.eq[0]->differentialStates[0];
	state->setOrder( 1 );
	state->x0.updateExpression( m_editor->txtInitX->text() );
	state->y0[0].updateExpression( m_editor->txtInitY->text() );

	tempFunction.eq[0]->differentialStates.setStep( m_editor->integralStep->text() );
	tempFunction.m_parameters = m_editor->cartesianParameters->parameterSettings();
	
	if ( !tempFunction.eq[0]->setFstr( f_str ) )
		return;
	
	//save all settings in the function now when we know no errors have appeared
	bool changed = f->copyFrom( tempFunction );
	if ( !changed )
		return;

		
	MainDlg::self()->requestSaveCurrentState();
	if ( functionListItem )
		functionListItem->update();
	View::self()->drawPlot();
}


void FunctionEditor::savePolar()
{
// 	kDebug() << k_funcinfo << endl;
	
	Function * f = XParser::self()->functionWithID( m_functionID );
	if ( !f )
		return;
	
	FunctionListItem * functionListItem = static_cast<FunctionListItem*>(m_functionList->currentItem());
	
	QString f_str = m_editor->polarEquation->text();

	XParser::self()->fixFunctionName( f_str, Equation::Polar, f->id );
	Function tempFunction( Function::Polar );  // all settings are saved here until we know that no errors have appeared
	
	bool ok = tempFunction.dmin.updateExpression( m_editor->polarMin->text() );
	if ( !ok )
		return;
	
	ok = tempFunction.dmax.updateExpression( m_editor->polarMax->text() );
	if ( !ok )
		return;
	
	if ( tempFunction.usecustomxmin && tempFunction.usecustomxmax && tempFunction.dmin.value() >= tempFunction.dmax.value() )
	{
		kWarning() << "min > max\n";
// 		KMessageBox::sorry(this,i18n("The minimum range value must be lower than the maximum range value"));
// 		m_editor->min->setFocus();
// 		m_editor->min->selectAll();
		return;
	}
	
	tempFunction.m_parameters = m_editor->polarParameters->parameterSettings();
	if (functionListItem)
		tempFunction.plotAppearance( Function::Derivative0 ) = m_editor->polar_f0->plot( (functionListItem->checkState() == Qt::Checked) );
	
	if ( !tempFunction.eq[0]->setFstr( f_str ) )
		return;
	
	//save all settings in the function now when we know no errors have appeared
	bool changed = f->copyFrom( tempFunction );
	if ( !changed )
		return;

// 	kDebug() << "Polar changed, so requesting state save.\n";	
	MainDlg::self()->requestSaveCurrentState();
	if ( functionListItem )
		functionListItem->update();
	View::self()->drawPlot();
}


void FunctionEditor::saveParametric()
{
// 	kDebug() << k_funcinfo << endl;
	
	FunctionListItem * functionListItem = static_cast<FunctionListItem*>(m_functionList->currentItem());
	
	Function * f = XParser::self()->functionWithID( m_functionID );
	
	if ( !f )
	{
		kWarning() << k_funcinfo << "No f!\n";
		return;
	}
	if ( f->type() != Function::Parametric )
	{
		kWarning() << k_funcinfo << "f is not parametric!\n";
		return;
	}
	      
	Function tempFunction( Function::Parametric );
	
	QString f_str = m_editor->parametricX->text();
	XParser::self()->fixFunctionName( f_str, Equation::ParametricX, f->id );
	if ( !tempFunction.eq[0]->setFstr( f_str ) )
		return;
	
	f_str = m_editor->parametricY->text();
	XParser::self()->fixFunctionName( f_str, Equation::ParametricY, f->id );
	if ( !tempFunction.eq[1]->setFstr( f_str ) )
		return;
	
	tempFunction.usecustomxmin = true;
	bool ok = tempFunction.dmin.updateExpression( m_editor->parametricMin->text() );
	if ( tempFunction.usecustomxmin && !ok )
		return;
	
	tempFunction.usecustomxmax = true;
	ok = tempFunction.dmax.updateExpression( m_editor->parametricMax->text() );
	if ( tempFunction.usecustomxmax && !ok )
		return;
	
	if ( tempFunction.usecustomxmin && tempFunction.usecustomxmax && tempFunction.dmin.value() >= tempFunction.dmax.value() )
	{
// 		KMessageBox::sorry(this,i18n("The minimum range value must be lower than the maximum range value"));
// 		m_editor->min->setFocus();
// 		m_editor->min->selectAll();
		return;
	}
	
	tempFunction.m_parameters = m_editor->parametricParameters->parameterSettings();
	if (functionListItem)
		tempFunction.plotAppearance( Function::Derivative0 ) = m_editor->parametric_f0->plot( (functionListItem->checkState() == Qt::Checked) );
	
	//save all settings in the function now when we know no errors have appeared
	bool changed = f->copyFrom( tempFunction );
	if ( !changed )
		return;
	
	MainDlg::self()->requestSaveCurrentState();
	if ( functionListItem )
		functionListItem->update();
	View::self()->drawPlot();
}


void FunctionEditor::saveImplicit()
{
// 	kDebug() << k_funcinfo << endl;
	
	Function * f = XParser::self()->functionWithID( m_functionID );
	if ( !f )
		return;
	
	FunctionListItem * functionListItem = static_cast<FunctionListItem*>(m_functionList->currentItem());
	
	// find a name not already used 
	if ( m_editor->implicitName->text().isEmpty() )
	{
		QString fname;
		XParser::self()->fixFunctionName(fname, Equation::Implicit, f->id );
		int const pos = fname.indexOf('(');
		m_editor->implicitName->setText(fname.mid(1,pos-1));
	}
	
	QString prefix = m_editor->implicitName->text() + " = ";
	QString f_str = prefix + m_editor->implicitEquation->text();
	m_editor->implicitEquation->setValidatePrefix( prefix );

	Function tempFunction( Function::Implicit );  // all settings are saved here until we know that no errors have appeared
	
	tempFunction.m_parameters = m_editor->implicitParameters->parameterSettings();
	if (functionListItem)
		tempFunction.plotAppearance( Function::Derivative0 ) = m_editor->implicit_f0->plot( (functionListItem->checkState() == Qt::Checked) );
	
	if ( !tempFunction.eq[0]->setFstr( f_str ) )
		return;
	
	//save all settings in the function now when we know no errors have appeared
	bool changed = f->copyFrom( tempFunction );
	if ( !changed )
		return;

// 	kDebug() << "Implicit changed, so requesting state save.\n";	
	MainDlg::self()->requestSaveCurrentState();
	if ( functionListItem )
		functionListItem->update();
	View::self()->drawPlot();
}


void FunctionEditor::saveDifferential()
{
	Function * f = XParser::self()->functionWithID( m_functionID );
	if ( !f || f->type() != Function::Differential )
		return;
	
	FunctionListItem * functionListItem = static_cast<FunctionListItem*>(m_functionList->currentItem());
	
	Function tempFunction( Function::Differential );  // all settings are saved here until we know that no errors have appeared
	QString f_str = m_editor->differentialEquation->text();
	if ( !tempFunction.eq[0]->setFstr( f_str ) )
		return;
	
	tempFunction.m_parameters = m_editor->differentialParameters->parameterSettings();
	if (functionListItem)
		tempFunction.plotAppearance( Function::Derivative0 ) = m_editor->differential_f0->plot( (functionListItem->checkState() == Qt::Checked) );
	
	m_editor->initialConditions->setOrder( tempFunction.eq[0]->order() );
	tempFunction.eq[0]->differentialStates = *m_editor->initialConditions->differentialStates();
	tempFunction.eq[0]->differentialStates.setStep( m_editor->differentialStep->text() );
	
	//save all settings in the function now when we know no errors have appeared
	bool changed = f->copyFrom( tempFunction );
	if ( !changed )
		return;
	
	MainDlg::self()->requestSaveCurrentState();
	if ( functionListItem )
		functionListItem->update();
	View::self()->drawPlot();
}
//END class FunctionEditor



//BEGIN class FunctionListWidget
FunctionListWidget::FunctionListWidget( QWidget * parent )
	: QListWidget( parent )
{
	setAcceptDrops(true);
    setDragEnabled(true);
	show();
}


QMimeData * FunctionListWidget::mimeData( const QList<QListWidgetItem *> items ) const
{
	QDomDocument doc( "kmpdoc" );
	QDomElement root = doc.createElement( "kmpdoc" );
	doc.appendChild( root );
	
	KmPlotIO io;
	
	foreach ( QListWidgetItem * item, items )
	{
		int f = static_cast<FunctionListItem*>(item)->function();
		
		if ( Function * function = XParser::self()->functionWithID( f ) )
			io.addFunction( doc, root, function );
	}
	
	QMimeData * md = new QMimeData;
	md->setData( "text/kmplot", doc.toByteArray() );
	
	return md;
}


QStringList FunctionListWidget::mimeTypes() const
{
	QStringList mt;
	mt << "text/kmplot";
	return mt;
}


void FunctionListWidget::dragEnterEvent( QDragEnterEvent * event )
{
	const QMimeData * md = event->mimeData();
	if ( md->hasFormat( "text/kmplot" ) )
		event->acceptProposedAction();
}


void FunctionListWidget::dropEvent( QDropEvent * event )
{
	const QMimeData * md = event->mimeData();
	
	QDomDocument doc( "kmpdoc" );
	doc.setContent( md->data( "text/kmplot" ) );
	QDomElement element = doc.documentElement();
	
	KmPlotIO io;
	
	for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
	{
		if ( n.nodeName() == "function" )
			io.parseFunction( n.toElement(), true );
		else
			kWarning() << k_funcinfo << "Unexpected node with name " << n.nodeName() << endl;
	}
}
//END class FunctionListWidget



//BEGIN class FunctionListItem
FunctionListItem::FunctionListItem( QListWidget * parent, int function )
	: QListWidgetItem( parent )
{
	m_function = function;
	assert( m_function != -1 );
	update();
}


void FunctionListItem::update()
{
	Function * f = XParser::self()->functionWithID( m_function );
	
	if ( !f )
	{
		// The function was probably deleted
		return;
	}
	
	QString text = f->eq[0]->fstr();
	if ( f->eq.size() == 2 )
		text += '\n' + f->eq[1]->fstr();
// 	text += QString(" id=%1").arg(m_function );
	setText( text );
	
	setCheckState( f->plotAppearance( Function::Derivative0 ).visible ? Qt::Checked : Qt::Unchecked );
	setTextColor( f->plotAppearance( Function::Derivative0 ).color );
}
//END class FunctionListItem



#include "functioneditor.moc"
