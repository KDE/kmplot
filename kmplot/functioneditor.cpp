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

#include "functioneditor.h"
#include "functioneditorwidget.h"
#include "kmplotio.h"
#include "kparametereditor.h"
#include "View.h"
#include "MainDlg.h"
#include "xparser.h"

#include <kaction.h>
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
	m_function = -1;
	m_functionX = -1;
	m_functionY = -1;
	m_createNewPlotsMenu = createNewPlotsMenu;
	
	// need a name for saving and restoring the position of this dock widget
	setObjectName( "FunctionEditor" );
	
	setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
	
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
	m_functionList = new FunctionListWidget( m_editor );
	m_editor->functionListContainer->insertWidget( 0, m_functionList );
	m_editor->functionListContainer->setCurrentIndex( 0 );
	
	for ( unsigned i = 0; i < 3; ++i )
		m_editor->stackedWidget->widget(i)->layout()->setMargin( 0 );
	
	for( int number = 0; number < SLIDER_COUNT; number++ )
		m_editor->listOfSliders->addItem( i18n( "Slider No. %1" ).arg( number +1) );
	
	connect( m_editor->deleteButton, SIGNAL(clicked()), this, SLOT(deleteCurrent()) );
	connect( m_functionList, SIGNAL(currentItemChanged( QListWidgetItem *, QListWidgetItem * )), this, SLOT(functionSelected( QListWidgetItem* )) );
	connect( m_functionList, SIGNAL(itemClicked( QListWidgetItem * )), this, SLOT(save()) ); // user might have checked or unchecked the item
	connect( m_editor->editParameterListButton, SIGNAL(clicked()), this, SLOT(editParameterList()) );
	
	//BEGIN connect up all editing widgets
	QList<QLineEdit *> lineEdits = m_editor->findChildren<QLineEdit *>();
	foreach ( QLineEdit * w, lineEdits )
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
	//END connect up all editing widgets
	
	connect( View::self()->parser(), SIGNAL(functionAdded(int)), this, SLOT(functionsChanged()) );
	connect( View::self()->parser(), SIGNAL(functionRemoved(int)), this, SLOT(functionsChanged()) );
	
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
	
	if ( !View::self()->parser()->delfkt( functionItem->function1() ) )
	{
		kDebug() << "Couldn't delete function 1.\n";
		// couldn't delete it, as e.g. another function depends on it
		return;
	}
	
	kDebug() << "Deleting function 2 status: " <<
			View::self()->parser()->delfkt( functionItem->function2() ) << endl;
	
	kDebug() << "Deleted current, so requestion state save.\n";
	View::self()->mainDlg()->requestSaveCurrentState();
	View::self()->drawPlot();
}


void FunctionEditor::functionsChanged()
{
	m_syncFunctionListTimer->start( 0 );
}


void FunctionEditor::syncFunctionList()
{
	kDebug() << k_funcinfo << endl;
	
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
		currentIDs[ item->function1() ] = item;
		if ( item->function2() != -1 )
			currentIDs[ item->function2() ] = item;
		
		// also update what is displayed
		item->update();
	}
	
	FunctionListItem * toSelect = 0l;
	int newFunctionCount = 0;
	
	for ( QMap<int, Ufkt*>::iterator it = View::self()->parser()->m_ufkt.begin(); it != View::self()->parser()->m_ufkt.end(); ++it)
	{
		Ufkt * function = *it;
		
		if ( function->fname.isEmpty() )
			continue;
		
		if ( currentIDs.contains( function->id ) )
		{
			// already have the function
			currentFunctionItems.removeAll( currentIDs[ function->id ] );
			currentIDs.remove( function->id );
			continue;
		}
		
		if ( function->fstr[0] == 'y' )
			continue;
		
		int f1 = function->id;
		int f2 = -1;
		
		if ( function->fstr[0] == 'x' )
		{
			++it;
			assert( it != View::self()->parser()->m_ufkt.end() );
			f2 = (*it)->id;
		}
		
		toSelect = new FunctionListItem( m_functionList, f1, f2 );
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
		if ( m_function == item->function1() )
			m_function = -1;
		
		else if ( m_functionX == item->function1() || m_functionY == item->function2() )
		{
			m_functionX = -1;
			m_functionY = -1;
		}
		
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
		if ( (item->function1() != functionID) && (item->function2() != functionID) )
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
	
	if ( functionItem->function2() != -1 )
	{
		m_function = -1;
		m_functionX = functionItem->function1();
		m_functionY = functionItem->function2();
		
		initFromParametric();
	}
	else
	{
		m_function = functionItem->function1();
		m_functionX = -1;
		m_functionY = -1;
		
		if ( Ufkt * function = View::self()->parser()->functionWithID(m_function) )
		{
			QChar prefix = function->fstr[0];
			if ( prefix == 'r' )
				initFromPolar();
			else
				initFromCartesian();
		}
	}
	
	functionItem->update();
}


void FunctionEditor::initFromCartesian()
{
// 	kDebug() << k_funcinfo << endl;
	
	Ufkt * f = View::self()->parser()->functionWithID(m_function);
	
	if ( !f )
	{
		kWarning() << k_funcinfo << "No f!\n";
		return;
	}
	
	m_parameters = f->parameters;
	
	m_editor->cartesianEquation->setText( f->fstr );
	m_editor->cartesian_f_lineWidth->setValue( f->linewidth );
	m_editor->cartesian_f_lineColor->setColor( f->color );
	
	m_editor->cartesianCustomMin->setChecked( f->usecustomxmin );
	m_editor->cartesianMin->setText( f->str_dmin );
	
	m_editor->cartesianCustomMax->setChecked( f->usecustomxmax );
	m_editor->cartesianMax->setText( f->str_dmax );
	
	if( f->use_slider == -1 )
	{
		m_editor->listOfSliders->setCurrentIndex( f->use_slider );
		if ( f->parameters.isEmpty() )
			m_editor->cartesianDisableParameters->setChecked( true );
		else    
			m_editor->cartesianParametersList->setChecked( true );
	}
	else
	{
		m_editor->cartesianParameterSlider->setChecked( true );
		m_editor->listOfSliders->setCurrentIndex( f->use_slider );
	}
        
	m_editor->showDerivative1->setChecked( f->f1_mode );
	m_editor->cartesian_f1_lineWidth->setValue( f->f1_linewidth );
	m_editor->cartesian_f1_lineColor->setColor( f->f1_color );
	
	m_editor->showDerivative2->setChecked( f->f2_mode );
	m_editor->cartesian_f2_lineWidth->setValue( f->f2_linewidth );
	m_editor->cartesian_f2_lineColor->setColor( f->f2_color );
	
	m_editor->precision->setValue( f->integral_precision );
	m_editor->cartesian_F_lineWidth->setValue( f->integral_linewidth );
	m_editor->cartesian_F_lineColor->setColor( f->integral_color );
	
	m_editor->showIntegral->setChecked( f->integral_mode );
	m_editor->customPrecision->setChecked( f->integral_use_precision );
	m_editor->txtInitX->setText(f->str_startx);
	m_editor->txtInitY->setText(f->str_starty);
	
	m_editor->stackedWidget->setCurrentIndex( 0 );
	m_editor->tabWidget->setCurrentIndex( 0 );
	m_editor->cartesianEquation->setFocus();
}


void FunctionEditor::initFromPolar()
{
// 	kDebug() << k_funcinfo << endl;
	
	Ufkt * f = View::self()->parser()->functionWithID(m_function);
	
	if ( !f )
		return;
	
	QString function = f->fstr;
	function = function.right( function.length()-1 );
	m_editor->polarEquation->setText( function );
	m_editor->polarCustomMin->setChecked( f->usecustomxmin );
	m_editor->polarMin->setText( f->str_dmin );
	m_editor->polarCustomMax->setChecked( f->usecustomxmax );
	m_editor->polarMax->setText( f->str_dmax );
	m_editor->polarLineWidth->setValue( f->linewidth );
	m_editor->polarLineColor->setColor( f->color );
	
	m_editor->stackedWidget->setCurrentIndex( 2 );
	m_editor->polarEquation->setFocus();
}


void FunctionEditor::initFromParametric()
{
// 	kDebug() << k_funcinfo << endl;
	
	Ufkt * fx = View::self()->parser()->functionWithID(m_functionX);
	Ufkt * fy = View::self()->parser()->functionWithID(m_functionY);
	
	if ( !fx || !fy )
		return;
	
	QString name, expression;
	
	splitParametricEquation( fx->fstr, & name, & expression );
	m_editor->parametricName->setText( name );
	m_editor->parametricX->setText( expression );
        
	splitParametricEquation( fy->fstr, & name, & expression );
	m_editor->parametricY->setText( expression );

	m_editor->parametricMin->setText( fx->str_dmin );
	m_editor->parametricMax->setText( fx->str_dmax );
        
	m_editor->parametricLineWidth->setValue( fx->linewidth );
	m_editor->parametricLineColor->setColor( fx->color );
	
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
	m_function = -1;
	m_functionX = -1;
	m_functionY = -1;
	
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
	KMessageBox::information( this, QString("View::m_self=%1").arg((unsigned long long)View::self()) );
	
	m_function = -1;
	m_functionX = -1;
	m_functionY = -1;
	
	// find a name not already used
	QString fname( "f(x)=0" );
	View::self()->parser()->fixFunctionName( fname, XParser::Function, -1 );
	
	m_function = View::self()->parser()->addFunction( fname );
	assert( m_function != -1 );

	kDebug() << "Created cartesian, so requestion state save.\n";
	View::self()->mainDlg()->requestSaveCurrentState();
}


void FunctionEditor::createParametric()
{
	m_function = -1;
	m_functionX = -1;
	m_functionY = -1;
	
	// find a name not already used
	QString fname;
	View::self()->parser()->fixFunctionName( fname, XParser::ParametricX, -1 );
	QString name = fname.mid( 1, fname.indexOf('(')-1 );
	
	kDebug() << "AAA\n";
	
	m_functionX = View::self()->parser()->addfkt( QString("x%1(t)=0").arg( name ) ); 
	assert( m_functionX != -1 );
	
	kDebug() << "BBB\n";
	
	m_functionY = View::self()->parser()->addfkt( QString("y%1(t)=0").arg( name ) );
	assert( m_functionY != -1 );
	
	kDebug() << "CCC\n";

	kDebug() << "Created parametric, so requestion state save.\n";
	View::self()->mainDlg()->requestSaveCurrentState();
	
	kDebug() << "DDD\n";
}


void FunctionEditor::createPolar()
{
	m_function = -1;
	m_functionX = -1;
	m_functionY = -1;
	
	// find a name not already used
	QString fname( "f(x)=0" );
	View::self()->parser()->fixFunctionName( fname, XParser::Polar, -1 );
	
	m_function = View::self()->parser()->addFunction( fname );
	assert( m_function != -1 );

	View::self()->mainDlg()->requestSaveCurrentState();
}


void FunctionEditor::save()
{
// 	kDebug() << k_funcinfo << endl;
	
	if ( m_function != -1 )
	{
		Ufkt * f = View::self()->parser()->functionWithID( m_function );
		if ( !f )
			return;
		
		QChar prefix = f->fstr[0];
		if ( prefix == 'r' )
			m_savePolarTimer->start( 0 );
		else
			m_saveCartesianTimer->start( 0 );
	}
	else if ( (m_functionX != -1) && (m_functionY != -1) )
	{
		m_saveParametricTimer->start( 0 );
	}
}


void FunctionEditor::saveCartesian()
{
	kDebug() << k_funcinfo << endl;
	
	Ufkt * f = View::self()->parser()->functionWithID( m_function );
	if ( !f )
		return;
	
	FunctionListItem * functionListItem = static_cast<FunctionListItem*>(m_functionList->currentItem());
	
	QString f_str( m_editor->cartesianEquation->text() );
    View::self()->parser()->fixFunctionName(f_str, XParser::Function, f->id );
	
	//all settings are saved here until we know that no errors have appeared
	Ufkt tempFunction;
	
	tempFunction.usecustomxmin = m_editor->cartesianCustomMin->isChecked();
	tempFunction.str_dmin = m_editor->cartesianMin->text();
	tempFunction.dmin = View::self()->parser()->eval( tempFunction.str_dmin );
	if ( tempFunction.usecustomxmin && View::self()->parser()->parserError( false ) != 0)
	{
// 		showPage(0);
// 		editfunctionpage->min->setFocus();
// 		editfunctionpage->min->selectAll();
		return;
	}
	
	tempFunction.usecustomxmax = m_editor->cartesianCustomMax->isChecked();
	tempFunction.str_dmax = m_editor->cartesianMax->text();
	tempFunction.dmax = View::self()->parser()->eval( tempFunction.str_dmax );
	if ( tempFunction.usecustomxmax && View::self()->parser()->parserError( false ) != 0)
	{
// 		showPage(0);
// 		editfunctionpage->max->setFocus();
// 		editfunctionpage->max->selectAll();
		return;
	}
        
	if( tempFunction.usecustomxmin && tempFunction.usecustomxmax )
	{
		if ( tempFunction.dmin >=  tempFunction.dmax)
		{
// 			KMessageBox::sorry(this,i18n("The minimum range value must be lower than the maximum range value"));
// 			showPage(0);
// 			editfunctionpage->min->setFocus();
// 			editfunctionpage->min->selectAll();
			return;
		}
                
		if (  tempFunction.dmin<View::xmin || tempFunction.dmax>View::xmax )
		{
// 			KMessageBox::sorry(this,i18n("Please insert a minimum and maximum range between %1 and %2").arg(View::xmin).arg(View::xmax) );
// 			showPage(0);
// 			editfunctionpage->min->setFocus();
// 			editfunctionpage->min->selectAll();
			return;
		}
	}

	tempFunction.linewidth = m_editor->cartesian_f_lineWidth->value();
	tempFunction.color = m_editor->cartesian_f_lineColor->color().rgb();
	
	tempFunction.integral_mode = m_editor->showIntegral->isChecked();
	double initx = View::self()->parser()->eval( m_editor->txtInitX->text() );
	tempFunction.startx = initx;
	tempFunction.str_startx = m_editor->txtInitX->text();
	if ( tempFunction.integral_mode && View::self()->parser()->parserError(false) != 0)
	{
// 			KMessageBox::sorry(this,i18n("Please insert a valid x-value"));
// 			showPage(2);
// 			editintegralpage->txtInitX->setFocus();
// 			editintegralpage->txtInitX->selectAll();
		return;
	}
                
	double inity = View::self()->parser()->eval(m_editor->txtInitY->text());
	tempFunction.starty = inity;
	tempFunction.str_starty = m_editor->txtInitY->text();
	if ( tempFunction.integral_mode && View::self()->parser()->parserError(false) != 0)
	{
// 			KMessageBox::sorry(this,i18n("Please insert a valid y-value"));
// 			showPage(2);
// 			editintegralpage->txtInitY->setFocus();
// 			editintegralpage->txtInitY->selectAll();
		return;
	}

	tempFunction.integral_color = m_editor->cartesian_F_lineColor->color().rgb();
	tempFunction.integral_use_precision = m_editor->customPrecision->isChecked();
	tempFunction.integral_precision = m_editor->precision->value();
	tempFunction.integral_linewidth = m_editor->cartesian_F_lineWidth->value();

	if ( functionListItem )
		tempFunction.f_mode = (functionListItem->checkState() == Qt::Checked);
	
	tempFunction.parameters = m_parameters;
	if( m_editor->cartesianParameterSlider->isChecked() )
		tempFunction.use_slider = m_editor->listOfSliders->currentIndex(); //specify which slider that will be used
	else
		tempFunction.use_slider = -1;

	tempFunction.f1_mode =  m_editor->showDerivative1->isChecked();
	tempFunction.f1_linewidth = m_editor->cartesian_f1_lineWidth->value();
	tempFunction.f1_color = m_editor->cartesian_f1_lineColor->color().rgb();
        
	tempFunction.f2_mode =  m_editor->showDerivative2->isChecked();
	tempFunction.f2_linewidth = m_editor->cartesian_f2_lineWidth->value();
	tempFunction.f2_color = m_editor->cartesian_f1_lineColor->color().rgb();
        
	if ( f_str.contains('y') != 0 && ( tempFunction.f_mode || tempFunction.f1_mode || tempFunction.f2_mode) )
	{
// 		KMessageBox::sorry( this, i18n( "Recursive function is only allowed when drawing integral graphs") );
		return;
	}
	
	QString const old_fstr = f->fstr;
	if ( ( (!m_parameters.isEmpty() &&
				m_editor->cartesianParametersList->isChecked() ) ||
				m_editor->cartesianParameterSlider->isChecked() ) &&
			!cartesianHasTwoArguments( f_str ) )
		fixCartesianArguments( & f_str ); //adding an extra argument for the parameter value
	
	f->fstr = f_str;
	View::self()->parser()->reparse(f); //reparse the funcion
	if ( View::self()->parser()->parserError( false ) != 0)
	{
		f->fstr = old_fstr;
		View::self()->parser()->reparse(f); 
// 		raise();
// 		showPage(0);
// 		editfunctionpage->equation->setFocus();
// 		editfunctionpage->equation->selectAll();
		return;
	}
	
	//save all settings in the function now when we know no errors have appeared
	bool changed = f->copyFrom( tempFunction );
	changed |= (old_fstr != f->fstr );
	kDebug() << "old_fstr="<<old_fstr<<" f->fstr="<<f->fstr<<" changed="<<changed<<endl;
	if ( !changed )
		return;

		
	View::self()->mainDlg()->requestSaveCurrentState();
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
	
	Ufkt * f = View::self()->parser()->functionWithID( m_function );
	if ( !f )
		return;
	
	FunctionListItem * functionListItem = static_cast<FunctionListItem*>(m_functionList->currentItem());
	
	QString f_str = m_editor->polarEquation->text();

	View::self()->parser()->fixFunctionName( f_str, XParser::Polar, f->id );
	Ufkt tempFunction;  //all settings are saved here until we know that no errors have appeared

	if ( functionListItem )
		tempFunction.f_mode = (functionListItem->checkState() == Qt::Checked);
	
	tempFunction.usecustomxmin = m_editor->polarCustomMin->isChecked();
	tempFunction.str_dmin = m_editor->polarMin->text();
	tempFunction.dmin = View::self()->parser()->eval( tempFunction.str_dmin );
	if ( tempFunction.usecustomxmin && View::self()->parser()->parserError( false ) )
	{
		kWarning() << "invalid xmin\n";
// 		m_editor->min->setFocus();
// 		m_editor->min->selectAll();
		return;
	}
	
	tempFunction.usecustomxmax = m_editor->polarCustomMax->isChecked();
	tempFunction.str_dmax = m_editor->polarMax->text();
	tempFunction.dmax = View::self()->parser()->eval( tempFunction.str_dmax );
	if ( tempFunction.usecustomxmax && View::self()->parser()->parserError( false ) )
	{
		kWarning() << "invalid xmax\n";
// 		m_editor->max->setFocus();
// 		m_editor->max->selectAll();
		return;
	}
	
	if ( tempFunction.usecustomxmin && tempFunction.usecustomxmax && tempFunction.dmin >= tempFunction.dmax)
	{
		kWarning() << "min > max\n";
// 		KMessageBox::sorry(this,i18n("The minimum range value must be lower than the maximum range value"));
// 		m_editor->min->setFocus();
// 		m_editor->min->selectAll();
		return;
	}
	
	tempFunction.linewidth = m_editor->polarLineWidth->value();
	tempFunction.color = m_editor->polarLineColor->color().rgb();
	tempFunction.use_slider = -1;
        
	
	QString const old_fstr = f->fstr;
	f->fstr = f_str;
	View::self()->parser()->reparse( f ); //reparse the funcion
	if ( View::self()->parser()->parserError( false ) != 0)
	{
		kWarning() << "parse error\n";
		f->fstr = old_fstr;
		View::self()->parser()->reparse( f );
// 		raise();
// 		m_editor->kLineEditYFunction->setFocus();
// 		m_editor->kLineEditYFunction->selectAll();
		return;
	}
	
	//save all settings in the function now when we know no errors have appeared
	bool changed = f->copyFrom( tempFunction );
	changed |= (old_fstr != f->fstr);
	if ( !changed )
		return;

	kDebug() << "Polar changed, so requestion state save.\n";	
	View::self()->mainDlg()->requestSaveCurrentState();
	if ( functionListItem )
		functionListItem->update();
	View::self()->drawPlot();
}


void FunctionEditor::saveParametric()
{
// 	kDebug() << k_funcinfo << endl;
	
	FunctionListItem * functionListItem = static_cast<FunctionListItem*>(m_functionList->currentItem());
	
	Ufkt * fx = View::self()->parser()->functionWithID( m_functionX );
	Ufkt * fy = View::self()->parser()->functionWithID( m_functionY );
	
	if ( !fx || !fy )
		return;
	
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
		View::self()->parser()->fixFunctionName(fname, XParser::ParametricX, fx->id );
		int const pos = fname.indexOf('(');
		m_editor->parametricName->setText(fname.mid(1,pos-1));
	}
                
	Ufkt tempFunction;
	if ( functionListItem )
		tempFunction.f_mode = (functionListItem->checkState() == Qt::Checked);
	
	tempFunction.usecustomxmin = true;
	tempFunction.str_dmin = m_editor->parametricMin->text();
	tempFunction.dmin = View::self()->parser()->eval( tempFunction.str_dmin );
	if ( tempFunction.usecustomxmin && View::self()->parser()->parserError( false ) )
	{
// 		m_editor->min->setFocus();
// 		m_editor->min->selectAll();
		return;
	}
	
	tempFunction.usecustomxmax = true;
	tempFunction.str_dmax = m_editor->parametricMax->text();
	tempFunction.dmax = View::self()->parser()->eval( tempFunction.str_dmax );
	if ( tempFunction.usecustomxmax && View::self()->parser()->parserError( false ) )
	{
// 		m_editor->max->setFocus();
// 		m_editor->max->selectAll();
		return;
	}
	
	if ( tempFunction.usecustomxmin && tempFunction.usecustomxmax && tempFunction.dmin >=  tempFunction.dmax)
	{
// 		KMessageBox::sorry(this,i18n("The minimum range value must be lower than the maximum range value"));
// 		m_editor->min->setFocus();
// 		m_editor->min->selectAll();
		return;
	}
        
	tempFunction.linewidth = m_editor->parametricLineWidth->value();
	tempFunction.color = m_editor->parametricLineColor->color().rgb();
	tempFunction.f1_color = tempFunction.f2_color = tempFunction.integral_color = tempFunction.color;
	
	QString old_fstr = fx->fstr;
	fx->fstr = "x" + m_editor->parametricName->text() + "(t)=" + m_editor->parametricX->text();
	View::self()->parser()->reparse(fx); //reparse the funcion
	if ( View::self()->parser()->parserError( false ) != 0)
	{
		fx->fstr = old_fstr;
		View::self()->parser()->reparse(fx); 
// 		raise();
// 		m_editor->kLineEditXFunction->setFocus();
// 		m_editor->kLineEditXFunction->selectAll();
		return;
	}
	
	//save all settings in the function now when we know no errors have appeared
	bool changed = fx->copyFrom( tempFunction );
	changed |= (old_fstr != fx->fstr);
	
	
	// now for the y function
	
	old_fstr = fy->fstr;
	fy->fstr = "y" + m_editor->parametricName->text() + "(t)=" + m_editor->parametricY->text();
	View::self()->parser()->reparse( fy ); //reparse the funcion
	if ( View::self()->parser()->parserError( false ) != 0 ) //when something went wrong:
	{
		fy->fstr = old_fstr; //go back to the old expression
		View::self()->parser()->reparse(fy);  //reparse
// 		raise();
// 		m_editor->kLineEditXFunction->setFocus();
// 		m_editor->kLineEditXFunction->selectAll();
		return;
	}
    
	//save all settings in the function now when we now no errors have appeared
	changed |= fy->copyFrom( tempFunction );
	changed |= (old_fstr != fy->fstr);
	
	if ( !changed )
		return;
	
	kDebug() << "Parametric changed, so requestion state save.\n";
	View::self()->mainDlg()->requestSaveCurrentState();
	if ( functionListItem )
		functionListItem->update();
	View::self()->drawPlot();
}


void FunctionEditor::editParameterList()
{
	KParameterEditor * dlg = new KParameterEditor( View::self()->parser(), & m_parameters );
	dlg->exec();
	saveCartesian();
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
		int f1 = static_cast<FunctionListItem*>(item)->function1();
		int f2 = static_cast<FunctionListItem*>(item)->function2();
		
		if ( Ufkt * function = View::self()->parser()->functionWithID( f1 ) )
			KmPlotIO::addFunction( doc, root, function );
		if ( Ufkt * function = View::self()->parser()->functionWithID( f2 ) )
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
			KmPlotIO::parseFunction( View::self()->parser(), n.toElement(), true );
		else
			kWarning() << k_funcinfo << "Unexpected node with name " << n.nodeName() << endl;
	}
}
//END class FunctionListWidget



//BEGIN class FunctionListItem
FunctionListItem::FunctionListItem( QListWidget * parent, int function1, int function2 )
	: QListWidgetItem( parent )
{
	m_function1 = function1;
	m_function2 = function2;
	
	assert( m_function1 != -1 );
	
// 	setFlags( Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
	
	update();
}


void FunctionListItem::update()
{
	Ufkt * f1 = View::self()->parser()->functionWithID( m_function1 );
	Ufkt * f2 = View::self()->parser()->functionWithID( m_function2 );
	
	if ( !f1 )
	{
		// The function was probably deleted
		return;
	}
	
	if ( f2 )
	{
		// a parametric function
		setText( f1->fstr + ";" + f2->fstr );
	}
	else
	{
		// a cartesian or polar function
		setText( f1->fstr );
	}
	
	setCheckState( f1->f_mode ? Qt::Checked : Qt::Unchecked );
	setTextColor( f1->color );
}
//END class FunctionListItem



#include "functioneditor.moc"
