/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2006  David Saxton <david@bluehaze.org>
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
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
		InitialConditionsModel( InitialConditionsEditor * parent );
		
		virtual int rowCount( const QModelIndex & parent ) const;
		virtual int columnCount( const QModelIndex & parent ) const;
		virtual QVariant data( const QModelIndex & index, int role ) const;
		virtual QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
		virtual bool setData( const QModelIndex & index, const QVariant & value, int role );
		virtual Qt::ItemFlags flags(const QModelIndex &index) const;
		virtual bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
		virtual bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());
		
	protected:
		InitialConditionsEditor * m_parent;
		
		friend class InitialConditionsEditor;
};


class InitialConditionsView : public QTableView
{
	public:
		InitialConditionsView( QWidget * parent );
};


class InitialConditionsDelegate : public QItemDelegate
{
	Q_OBJECT
	
	public:
		InitialConditionsDelegate( InitialConditionsEditor * parent );
		
		virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
		virtual void setEditorData( QWidget * editor, const QModelIndex & index ) const;
		virtual void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const;
		virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

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
		InitialConditionsEditor( QWidget * parent );
		
		/**
		 * Initialises the list from the given states in \p function
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
		 * For differential equations, add an initial condtion.
		 */
		void add();
		/**
		 * For differential equations, remove the selected initial condtion.
		 */
		void remove();
		
	protected:
		InitialConditionsModel * m_model;
		Equation * m_equation;
		DifferentialStates m_states;
};

#endif // INITIALCONDITIONSEDITOR_H
