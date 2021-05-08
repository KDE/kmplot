/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "initialconditionseditor.h"
#include "equationedit.h"
#include "parser.h"

#include <QHeaderView>
#include <assert.h>

//BEGIN helper functions
/**
 * \return a pointer to the differential state for the given function and row.
 */
DifferentialState * differentialState( DifferentialStates * states, int row )
{
	if ( !states )
		return 0;
	
	if ( row < 0 || row >= states->size() )
		return 0;
	
	return & (*states)[row];
}


/**
 * \return a pointer to the Value for the given function, row and column
 */
Value * value( DifferentialStates * states, int row, int column )
{
	DifferentialState * state = differentialState( states, row );
	if ( !state )
		return 0;
	
	if ( column == 0 )
		return & state->x0;
	else
		return & state->y0[ column- 1 ];
}
//END helper functions


//BEGIN class InitialConditionsModel
InitialConditionsModel::InitialConditionsModel( InitialConditionsEditor * parent )
	: QAbstractTableModel( parent )
{
	m_parent = parent;
}


int InitialConditionsModel::rowCount( const QModelIndex & /*parent*/ ) const
{
	return m_parent->differentialStates()->size();
}


int InitialConditionsModel::columnCount( const QModelIndex & /*parent*/ ) const
{
	return m_parent->differentialStates()->order()+1;
}


QVariant InitialConditionsModel::data( const QModelIndex & index, int role ) const
{
	Value * v = value( m_parent->differentialStates(), index.row(), index.column() );
	if ( !v )
		return QVariant();
	
	switch ( (Qt::ItemDataRole)role )
	{
		case Qt::DisplayRole:
		case Qt::EditRole:
		case Qt::AccessibleTextRole:
			return v->expression();
			
		case Qt::ToolTipRole:
			/// \todo return a description of the initial condition
			return QVariant();
			
		case Qt::DecorationRole:
		case Qt::StatusTipRole:
			return QVariant();
			
		case Qt::TextAlignmentRole:
			return Qt::AlignLeft;
			
		case Qt::ForegroundRole:
			return QColor(Qt::black);
			
		case Qt::WhatsThisRole:
		case Qt::AccessibleDescriptionRole:
		case Qt::CheckStateRole:
		case Qt::BackgroundRole:
		case Qt::SizeHintRole:
		case Qt::FontRole:
		case Qt::UserRole:
			return QVariant();

		default:
			return QVariant();
	}
}


bool InitialConditionsModel::setData( const QModelIndex & index, const QVariant & variant, int role )
{
	if ( role != Qt::EditRole )
		return false;
	
	Value * v = value( m_parent->differentialStates(), index.row(), index.column() );
	if ( !v )
		return false;
	
	v->updateExpression( variant.toString() );
	emit dataChanged( index, index );
	return true;
}


QVariant InitialConditionsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	Equation * eq = m_parent->equation();
	
	if ( role != Qt::DisplayRole || !eq )
		return QAbstractTableModel::headerData( section, orientation, role );
	
	// Don't display row headers
	if ( orientation == Qt::Vertical )
		return QVariant();
	
	QString param;
	QStringList variables = eq->variables();
	if ( variables.isEmpty() )
		param = 'x';
	else
		param = variables.first();
	
	param += SubscriptZeroSymbol;
	
	if ( section == 0 )
		return param;
	else
		return QStringLiteral( "%1%2(%3)" )
				.arg( eq->name( true ) )
				.arg( QString(), section-1, '\'' )
				.arg( param );
}


Qt::ItemFlags InitialConditionsModel::flags( const QModelIndex & /*index*/ ) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}


bool InitialConditionsModel::insertRows(int position, int rows, const QModelIndex & /*parent*/ )
{
	if ( !m_parent->differentialStates() )
		return false;
	
	beginInsertRows(QModelIndex(), position, position+rows-1);

	for (int row = 0; row < rows; ++row)
		m_parent->differentialStates()->add();

	endInsertRows();
	return true;
}


bool InitialConditionsModel::removeRows(int position, int rows, const QModelIndex & /*parent*/ )
{
	beginRemoveRows(QModelIndex(), position, position+rows-1);
	m_parent->differentialStates()->remove( position, rows );
	endRemoveRows();
	return true;
}
//END class InitialConditionsModel



//BEGIN class InitialConditionsView
InitialConditionsView::InitialConditionsView( QWidget * parent )
	: QTableView( parent )
{
	setSelectionMode( QAbstractItemView::ExtendedSelection );
	setSelectionBehavior( QAbstractItemView::SelectRows );
	horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
	horizontalHeader()->setSectionsClickable( false );
	verticalHeader()->hide();
}
//END class InitialConditionsView



//BEGIN class InitialConditionsDelegate
InitialConditionsDelegate::InitialConditionsDelegate( InitialConditionsEditor * parent )
	: QItemDelegate( parent )
{
	m_parent = parent;
	m_lastEditor = 0;
}


QWidget * InitialConditionsDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & index ) const
{
	Value * v = value( m_parent->differentialStates(), index.row(), index.column() );
	if ( !v )
		return 0;
	
	m_lastEditor = new EquationEdit( parent );
	connect(m_lastEditor, &EquationEdit::returnPressed, this, &InitialConditionsDelegate::equationEditDone);
	m_lastEditor->setFocus();
	return m_lastEditor;
}


void InitialConditionsDelegate::equationEditDone()
{
	emit commitData( m_lastEditor );
	emit closeEditor( m_lastEditor );
}


void InitialConditionsDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const
{
	QString expression = index.model()->data( index, Qt::DisplayRole ).toString();
	static_cast<EquationEdit*>(editor)->setText( expression );
}


void InitialConditionsDelegate::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const
{
	EquationEdit * edit = static_cast<EquationEdit*>(editor);
	model->setData( index, edit->text() );
}


void InitialConditionsDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */ ) const
{
	editor->setGeometry(option.rect);
}
//END class InitialConditionsDelegate



//BEGIN class InitialConditionsEditor
InitialConditionsEditor::InitialConditionsEditor( QWidget * parent )
	: QWidget( parent )
{
	m_equation = 0;
	
	setupUi( this );
	layout()->setContentsMargins( 0, 0, 0, 0 );
	connect(addButton, &QPushButton::clicked, this, &InitialConditionsEditor::add);
	connect(removeButton, &QPushButton::clicked, this, &InitialConditionsEditor::remove);
	
	m_model = new InitialConditionsModel( this );
	view->setModel( m_model );
	view->setItemDelegate( new InitialConditionsDelegate( this ) );
	
	connect(m_model, &InitialConditionsModel::dataChanged, this, &InitialConditionsEditor::dataChanged);
}


void InitialConditionsEditor::setOrder( int order )
{
	m_model->beginResetModel();
	m_states.setOrder( order );
	m_model->endResetModel();
}


void InitialConditionsEditor::init( Function * function )
{
	m_model->beginResetModel();

	if ( function )
	{
		m_equation = function->eq[0];
		m_states = m_equation->differentialStates;
	}
	else
	{
		m_equation = 0;
	}
	
	m_model->endResetModel();
}


void InitialConditionsEditor::add()
{
	m_model->insertRows( 0, 1, QModelIndex() );
	emit dataChanged();
}

		
void InitialConditionsEditor::remove()
{
	const QModelIndexList selected = view->selectionModel()->selectedIndexes();
	
	QMap< int, void * > sorted;
	for ( const QModelIndex &index : selected )
		sorted.insert( -index.row(), 0l );
	const QList<int> indexes = sorted.keys();
	
	for ( int row : indexes )
		m_model->removeRows( -row, 1, QModelIndex() );
	
	emit dataChanged();
}
//END class InitialConditionsEditor
