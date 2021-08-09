/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 1998-2002 Klaus-Dieter Möller <kd.moeller@t-online.de>
    SPDX-FileCopyrightText: 2004 Fredrik Edemar <f_edemar@linux.se>
    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef FUNCTIONEDITOR_H
#define FUNCTIONEDITOR_H

#include "parser.h"

#include <QDockWidget>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMimeData>

class FunctionEditorWidget;
class FunctionListItem;
class FunctionListWidget;
class QMenu;
class QTimer;
class Function;

class FunctionEditor : public QDockWidget
{
	Q_OBJECT
		
	public:
		FunctionEditor( QMenu * createNewPlotsMenu, QWidget * parent );
		virtual ~ FunctionEditor();
		
		/**
		 * Highlights the function item for the given function.
		 */
		void setCurrentFunction( int functionID );
		
	public slots:
		/**
		 * Deletes the current selected function.
		 */
		void deleteCurrent();
		/**
		 * Creates a new cartesian function.
		 */
		void createCartesian();
		/**
		 * Creates a new parametric function.
		 */
		void createParametric();
		/**
		 * Creates a new polar function.
		 */
		void createPolar();
		/**
		 * Creates a new implicit function.
		 */
		void createImplicit();
		/**
		 * Creates a new differential function.
		 */
		void createDifferential();
		/**
		 * Called when the list of functions in the parser changes.
		 */
		void functionsChanged();
		
	protected slots:
		/**
		 * Called when a function in the list is selected.
		 */
		void functionSelected( QListWidgetItem * function );
		/**
		 * Called when the user changes a function widget.
		 */
		void save();
		/**
		 * Called when the user ticks or unticks an item in the function list.
		 */
		void saveItem(QListWidgetItem *item);
		/**
		 * Updates the list of functions (called when a function is added or
		 * removed from Parser).
		 */
		void syncFunctionList();
		/**
		 * Called when the user edits any of the widgets relating to a
		 * cartesian function.
		 */
		void saveCartesian();
		/**
		 * Called when the user edits any of the widgets relating to a
		 * polar function.
		 */
		void savePolar();
		/**
		 * Called when the user edits any of the widgets relating to a
		 * parametric function.
		 */
		void saveParametric();
		/**
		 * Called when the user edits any of the widgets relating to an
		 * implicit function.
		 */
		void saveImplicit();
		/**
		 * Called when the user edits any of the widgets relating to a
		 * differential function.
		 */
		void saveDifferential();
		
		
	protected:
		/**
		 * Initialize the widgets from the cartesian function set in
		 * m_functionID.
		 */
		void initFromCartesian();
		/**
		 * Initialize the widgets from a polar function set in m_functionID.
		 */
		void initFromPolar();
		/**
		 * Initialize the widgets from a parametric function set in
		 * m_functionID.
		 */
		void initFromParametric();
		/**
		 * Initialize the widgets from an implicit function set in m_functionID.
		 */
		void initFromImplicit();
		/**
		 * Initialize the widgets from a differential function set in m_functionID.
		 */
		void initFromDifferential();
		/**
		 * Resets all the function editing widgets (i.e. those in the widget
		 * stack, but not the list of functions).
		 */
		void resetFunctionEditing();
		/**
		 * Extract function \p name and \p expression from the given
		 * \p equation - for implicit functions.
		 */
		void splitImplicitEquation( const QString &equation, QString * name, QString * expression );
		/**
		 * Used at end of the save* functions.
		 * \a tempFunction is the function to copy the settings from.
		 */
		void saveFunction( Function * tempFunction );
		/**
		 * Called from the create* functions - finishes of saving of the function.
		 */
		void createFunction( const QString & eq0, const QString & eq1, Function::Type type );

		/**
		 * The main editing widget.
		 */
		FunctionEditorWidget * m_editor;
		/**
		 * The id of the function currently being edited.
		 */
		int m_functionID;
		
		/**
		 * Used to ensure only one update when multiple widgets are changed.
		 * A timer for each function type.
		 */
		QTimer * m_saveTimer[5];
		/**
		 * Ensure only one update when functions are added or removed, and all
		 * the piece of code that is adding or removing a function to make the
		 * necessary changes first.
		 */
		QTimer * m_syncFunctionListTimer;
		/**
		 * The list of functions.
		 */
		FunctionListWidget * m_functionList;
};


class FunctionListWidget : public QListWidget
{
	public:
		explicit FunctionListWidget( QWidget * parent );
		
	protected:
		void dragEnterEvent( QDragEnterEvent * event ) Q_DECL_OVERRIDE;
		void dropEvent( QDropEvent * event ) Q_DECL_OVERRIDE;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QMimeData * mimeData( const QList<QListWidgetItem *> items ) const Q_DECL_OVERRIDE;
#else
		QMimeData * mimeData(const QList<QListWidgetItem *> &items) const Q_DECL_OVERRIDE;
#endif
		QStringList mimeTypes() const Q_DECL_OVERRIDE;
};


class FunctionListItem : public QListWidgetItem
{
	public:
		FunctionListItem( QListWidget * parent, int function );
		
		/**
		 * Updates the displayed text and checked state depending on the
		 * function types and states.
		 */
		void update();
		
		int function() const { return m_function; }
		
	protected:
		int m_function;
};


#endif // FUNCTIONEDITOR_H

