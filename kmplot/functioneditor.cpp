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
#include "MainDlg.h"
#include "parameterswidget.h"
#include "ui_functioneditorwidget.h"
#include "View.h"
#include "xparser.h"

#include <kaction.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QMenu>
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
	m_syncFunctionListTimer = new QTimer( this );
	
	m_saveCartesianTimer->setSingleShot( true );
	m_savePolarTimer->setSingleShot( true );
	m_saveParametricTimer->setSingleShot( true );
	m_syncFunctionListTimer->setSingleShot( true );
	
	connect( m_saveCartesianTimer, SIGNAL(timeout()), this, SLOT( saveCartesian() ) );
	connect( m_savePolarTimer, SIGNAL(timeout()), this, SLOT( savePolar() ) );
	connect( m_saveParametricTimer, SIGNAL(timeout()), this, SLOT( saveParametric() ) );
	connect( m_syncFunctionListTimer, SIGNAL(timeout()), this, SLOT( syncFunctionList() ) );
	
	m_editor = new FunctionEditorWidget;
	m_functionList = m_editor->functionList;
	
	m_editor->cartesianEquation->setInputType( EquationEdit::Function );
	m_editor->polarEquation->setInputType( EquationEdit::Function );
	m_editor->parametricX->setInputType( EquationEdit::Function );
	m_editor->parametricY->setInputType( EquationEdit::Function );
	
	for ( unsigned i = 0; i < 3; ++i )
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
	
	QList<QDoubleSpinBox *> doubleSpinBoxes = m_editor->findChildren<QDoubleSpinBox *>();
	foreach ( QDoubleSpinBox * w, doubleSpinBoxes )
		connect( w, SIGNAL(valueChanged(double)), this, SLOT(save()) );
	
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
	
	connect( m_editor->cartesianParameters, SIGNAL( parameterListChanged() ), this, SLOT(save()) );
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
	kDebug() << k_funcinfo << endl;
	
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
	
	kDebug() << "Deleted current, so requestion state save.\n";
	MainDlg::self()->requestSaveCurrentState();
	View::self()->drawPlot();
}


void FunctionEditor::functionsChanged()
{
	m_syncFunctionListTimer->start( 0 );
}


void FunctionEditor::syncFunctionList()
{
// 	kDebug() << k_funcinfo << endl;
	
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
	}
	
	functionItem->update();
}


void FunctionEditor::initFromCartesian()
{
// 	kDebug() << k_funcinfo << endl;
	
	Function * f = XParser::self()->functionWithID(m_functionID);
	
	if ( !f )
	{
		kWarning() << k_funcinfo << "No f! (id="<<m_functionID<<")\n";
		return;
	}
	
	m_editor->cartesianEquation->setText( f->eq[0]->fstr() );
	m_editor->cartesian_f0->init( f->f0 );
	m_editor->cartesian_f1->init( f->f1 );
	m_editor->cartesian_f2->init( f->f2 );
	m_editor->cartesian_integral->init( f->integral );
	
	m_editor->showDerivative1->setChecked( f->f1.visible );
	m_editor->showDerivative2->setChecked( f->f2.visible );
	
	m_editor->precision->setValue( f->integral_precision );
	
	m_editor->cartesianCustomMin->setChecked( f->usecustomxmin );
	m_editor->cartesianMin->setText( f->dmin.expression() );
	
	m_editor->cartesianCustomMax->setChecked( f->usecustomxmax );
	m_editor->cartesianMax->setText( f->dmax.expression() );
	
	m_editor->cartesianParameters->init( f );
	
	m_editor->showIntegral->setChecked( f->integral.visible );
	m_editor->customPrecision->setChecked( f->integral_use_precision );
	m_editor->txtInitX->setText( f->eq[0]->integralInitialX().expression() );
	m_editor->txtInitY->setText( f->eq[0]->integralInitialY().expression() );
	
	m_editor->stackedWidget->setCurrentIndex( 0 );
	m_editor->tabWidget->setCurrentIndex( 0 );
	m_editor->cartesianEquation->setFocus();
}


void FunctionEditor::initFromPolar()
{
// 	kDebug() << k_funcinfo << endl;
	
	Function * f = XParser::self()->functionWithID(m_functionID);
	
	if ( !f )
		return;
	
	QString function = f->eq[0]->fstr();
	function = function.mid( 1 );
	m_editor->polarEquation->setText( function );
	m_editor->polarMin->setText( f->dmin.expression() );
	m_editor->polarMax->setText( f->dmax.expression() );
	m_editor->polar_f0->init( f->f0 );
	
	m_editor->stackedWidget->setCurrentIndex( 2 );
	m_editor->polarEquation->setFocus();
}


void FunctionEditor::initFromParametric()
{
// 	kDebug() << k_funcinfo << endl;
	
	Function * f = XParser::self()->functionWithID(m_functionID);
	
	if ( !f )
		return;
	
	QString name, expression;
	
	splitParametricEquation( f->eq[0]->fstr(), & name, & expression );
	m_editor->parametricName->setText( name );
	m_editor->parametricX->setValidatePrefix( parametricXPrefix() );
	m_editor->parametricX->setText( expression );
        
	splitParametricEquation( f->eq[1]->fstr(), & name, & expression );
	m_editor->parametricY->setValidatePrefix( parametricYPrefix() );
	m_editor->parametricY->setText( expression );

	m_editor->parametricMin->setText( f->dmin.expression() );
	m_editor->parametricMax->setText( f->dmax.expression() );
	
	m_editor->parametric_f0->init( f->f0 );
	
	m_editor->stackedWidget->setCurrentIndex( 1 );
	m_editor->parametricName->setFocus();
}


void FunctionEditor::splitParametricEquation( const QString equation, QString * name, QString * expression )
{
	int start = 0;
	if( equation[ 0 ] == 'x' || equation[ 0 ] == 'y' )
		start++;
	int length = equation.indexOf( '(' ) - start;
	
	*name = equation.mid( start, length );   
	*expression = equation.section( '=', 1, 1 );
}


void FunctionEditor::resetFunctionEditing()
{
	m_functionID = -1;
	
	// page 3 is an empty page
	m_editor->stackedWidget->setCurrentIndex( 3 );
	
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
	m_functionID = -1;
	
	// find a name not already used
	QString fname( "f(x)=0" );
	XParser::self()->fixFunctionName( fname, Equation::Cartesian, -1 );
	
	m_functionID = XParser::self()->addFunction( fname, 0 );
	assert( m_functionID != -1 );

	kDebug() << "Created cartesian, so requestion state save.\n";
	MainDlg::self()->requestSaveCurrentState();
	View::self()->drawPlot();
}


void FunctionEditor::createParametric()
{
	m_functionID = -1;
	
	// find a name not already used
	QString fname;
	XParser::self()->fixFunctionName( fname, Equation::ParametricX, -1 );
	QString name = fname.mid( 1, fname.indexOf('(')-1 );
	
	m_functionID = XParser::self()->addFunction( QString("x%1(t)=0").arg( name ), QString("y%1(t)=0").arg( name ), Function::Parametric ); 
	assert( m_functionID != -1 );

	kDebug() << "Created parametric, so requestion state save.\n";
	MainDlg::self()->requestSaveCurrentState();
}


void FunctionEditor::createPolar()
{
	m_functionID = -1;
	
	// find a name not already used
	QString fname( "f(x)=0" );
	XParser::self()->fixFunctionName( fname, Equation::Polar, -1 );
	
	m_functionID = XParser::self()->addFunction( fname, 0 );
	assert( m_functionID != -1 );

	MainDlg::self()->requestSaveCurrentState();
}


void FunctionEditor::save()
{
// 	kDebug() << k_funcinfo << endl;
	
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
	}
}


void FunctionEditor::saveCartesian()
{
// 	kDebug() << k_funcinfo << endl;
	
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
	
	tempFunction.f0 = m_editor->cartesian_f0->plot( (functionListItem->checkState() == Qt::Checked) );
	tempFunction.f1 = m_editor->cartesian_f1->plot( m_editor->showDerivative1->isChecked() );
	tempFunction.f2 = m_editor->cartesian_f2->plot( m_editor->showDerivative2->isChecked() );
	tempFunction.integral = m_editor->cartesian_integral->plot( m_editor->showIntegral->isChecked() );
	
	tempFunction.eq[0]->setIntegralStart( m_editor->txtInitX->text(), m_editor->txtInitY->text() );

	tempFunction.integral_use_precision = m_editor->customPrecision->isChecked();
	tempFunction.integral_precision = m_editor->precision->value();
        
	if ( f_str.contains('y') != 0 && ( tempFunction.f0.visible || tempFunction.f1.visible || tempFunction.f2.visible) )
	{
// 		KMessageBox::sorry( this, i18n( "Recursive function is only allowed when drawing integral graphs") );
		return;
	}
	
	m_editor->cartesianParameters->save( & tempFunction );
	
	/// \todo Work out what this is suppose to do and fix it
// 	if ( ( (!m_parameters.isEmpty() &&
// 				m_editor->cartesianParametersList->isChecked() ) ||
// 				m_editor->cartesianParameterSlider->isChecked() ) &&
// 			!cartesianHasTwoArguments( f_str ) )
// 	{
// 		fixCartesianArguments( & f_str ); //adding an extra argument for the parameter value
// 	}
	
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


bool FunctionEditor::cartesianHasTwoArguments( const QString & function )
{
	int const openBracket = function.indexOf( "(" );
	int const closeBracket = function.indexOf( ")" );
	return function.mid( openBracket+1, closeBracket-openBracket-1 ).indexOf( "," ) != -1;
}


void FunctionEditor::fixCartesianArguments( QString * f_str )
{
	int const openBracket = f_str->indexOf( "(" );
	int const closeBracket = f_str->indexOf( ")" );
	char parameter_name;
	if ( closeBracket-openBracket == 2) //the function atribute is only one character
	{
		QChar function_name = f_str->at(openBracket+1);
		parameter_name = 'a';
		while ( parameter_name == function_name)
			parameter_name++;
	}
	else
		parameter_name = 'a';
	f_str->insert(closeBracket,parameter_name);
	f_str->insert(closeBracket,',');
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
	
	tempFunction.f0 = m_editor->polar_f0->plot( (functionListItem->checkState() == Qt::Checked) );
	
	tempFunction.use_slider = -1;
	
	if ( !tempFunction.eq[0]->setFstr( f_str ) )
		return;
	
	//save all settings in the function now when we know no errors have appeared
	bool changed = f->copyFrom( tempFunction );
	if ( !changed )
		return;

	kDebug() << "Polar changed, so requesting state save.\n";	
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
	
	m_editor->parametricX->setValidatePrefix( parametricXPrefix() );
	m_editor->parametricY->setValidatePrefix( parametricYPrefix() );
	
	if  ( m_editor->parametricX->text().contains('y') != 0 ||
			 m_editor->parametricY->text().contains('y') != 0)
	{
// 		KMessageBox::sorry( this, i18n( "Recursive function not allowed"));
// 		m_editor->kLineEditXFunction->setFocus();
// 		m_editor->kLineEditXFunction->selectAll();
		return;
	}
        
	// find a name not already used 
	if ( m_editor->parametricName->text().isEmpty() )
	{
		QString fname;
		XParser::self()->fixFunctionName(fname, Equation::ParametricX, f->id );
		int const pos = fname.indexOf('(');
		m_editor->parametricName->setText(fname.mid(1,pos-1));
	}
                
	Function tempFunction( Function::Parametric );
	
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
	
	tempFunction.f0 = m_editor->parametric_f0->plot( (functionListItem->checkState() == Qt::Checked) );
	
	if ( !tempFunction.eq[0]->setFstr( parametricXPrefix() ) )
		return;
	
	if ( !tempFunction.eq[1]->setFstr( parametricYPrefix() ) )
		return;
	
	//save all settings in the function now when we know no errors have appeared
	bool changed = f->copyFrom( tempFunction );
	if ( !changed )
		return;
	
	kDebug() << "Parametric changed, so requesting state save.\n";
	MainDlg::self()->requestSaveCurrentState();
	if ( functionListItem )
		functionListItem->update();
	View::self()->drawPlot();
}


QString FunctionEditor::parametricXPrefix() const
{
	return 'x' + m_editor->parametricName->text() + "(t)=" + m_editor->parametricX->text();
}


QString FunctionEditor::parametricYPrefix() const
{
	return 'y' + m_editor->parametricName->text() + "(t)=" + m_editor->parametricY->text();
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
	
	foreach ( QListWidgetItem * item, items )
	{
		int f = static_cast<FunctionListItem*>(item)->function();
		
		if ( Function * function = XParser::self()->functionWithID( f ) )
			KmPlotIO::addFunction( doc, root, function );
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
	
	for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
	{
		if ( n.nodeName() == "function" )
			KmPlotIO::parseFunction( n.toElement(), true );
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
	
// 	setFlags( Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
	
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
	if ( f->eq[1] )
		text += ';' + f->eq[1]->fstr();
// 	text += QString(" id=%1").arg(m_function );
	setText( text );
	
	setCheckState( f->f0.visible ? Qt::Checked : Qt::Unchecked );
	setTextColor( f->f0.color );
}
//END class FunctionListItem



#include "functioneditor.moc"
