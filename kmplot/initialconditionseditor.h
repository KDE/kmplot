/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef INITIALCONDITIONSEDITOR_H
#define INITIALCONDITIONSEDITOR_H

#include "function.h"

#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QTableView>

class DifferentialStates;
class EquationEdit;
class Function;
class InitialConditionsEditor;

class InitialConditionsModel : public QAbstractTableModel
{
	public:
		explicit InitialConditionsModel( InitialConditionsEditor * parent );
		
		int rowCount( const QModelIndex & parent ) const Q_DECL_OVERRIDE;
		int columnCount( const QModelIndex & parent ) const Q_DECL_OVERRIDE;
		QVariant data( const QModelIndex & index, int role ) const Q_DECL_OVERRIDE;
		QVariant headerData( int section, Qt::Orientation orientation, int role ) const Q_DECL_OVERRIDE;
		bool setData( const QModelIndex & index, const QVariant & value, int role ) Q_DECL_OVERRIDE;
		Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
		bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) Q_DECL_OVERRIDE;
		bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) Q_DECL_OVERRIDE;
		
	protected:
		InitialConditionsEditor * m_parent;
		
		friend class InitialConditionsEditor;
};


class InitialConditionsView : public QTableView
{
	public:
		explicit InitialConditionsView( QWidget * parent );
};


class InitialConditionsDelegate : public QItemDelegate
{
	Q_OBJECT
	
	public:
		explicit InitialConditionsDelegate( InitialConditionsEditor * parent );
		
		QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const Q_DECL_OVERRIDE;
		void setEditorData( QWidget * editor, const QModelIndex & index ) const Q_DECL_OVERRIDE;
		void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const Q_DECL_OVERRIDE;
		void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

	protected slots:
		void equationEditDone();
		
	protected:
		InitialConditionsEditor * m_parent;
		mutable EquationEdit * m_lastEditor;
};

#include "ui_initialconditionswidget.h"

class InitialConditionsEditor : public QWidget, public Ui::InitialConditionsWidget
{
	Q_OBJECT
			
	public:
		explicit InitialConditionsEditor( QWidget * parent );
		
		/**
		 * Initializes the list from the given states in \p function
		 */
		void init( Function * function );
		/**
		 * Changes the order of the differential equation.
		 */
		void setOrder( int order );
		/**
		 * \return the current function.
		 */
		DifferentialStates * differentialStates() { return & m_states; }
		/**
		 * \return the equation currently in use. Use differentialStates
		 * for accessing the DifferentialStates instead of through equation,
		 * however, since the DifferentialStates in equation will be
		 * overwritten after saving via function editor.
		 */
		Equation * equation() const { return m_equation; }
		
	signals:
		void dataChanged();
		
	public slots:
		/**
		 * For differential equations, add an initial condition.
		 */
		void add();
		/**
		 * For differential equations, remove the selected initial condition.
		 */
		void remove();
		
	protected:
		InitialConditionsModel * m_model;
		Equation * m_equation;
		DifferentialStates m_states;
};

#endif // INITIALCONDITIONSEDITOR_H
