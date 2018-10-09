/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C)      2006  David Saxton <david@bluehaze.org>
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

#ifndef KGRADIENTDIALOG_H
#define KGRADIENTDIALOG_H

#include <QDialog>
#include <QGradient>
#include <QPushButton>
#include <QWidget>

class QMouseEvent;
class QPaintEvent;
class QStyleOptionButton;

/**
 * \short A color-gradient strip with arrows to change the stops.
 * 
 * Displays a color gradient for editing. Color stops can be added, removed or
 * repositioned by the user via the mouse.
 * 
 * This widget can't change the colors used in the gradient however. For that,
 * you should either:
 * 
 * \li Use KGradientDialog, or
 * \li Connect to the colorChanged signal, provide your own color editing
 *     widgets, and call setColor as appropriate.
 * 
 * \see QGradient
 */
class KGradientEditor : public QWidget
{
	Q_OBJECT
	Q_PROPERTY( Qt::Orientation orientation READ orientation WRITE setOrientation )
	Q_PROPERTY( QGradient gradient READ gradient WRITE setGradient USER true )
	
	public:
		explicit KGradientEditor( QWidget * parent );
		~KGradientEditor();
		
		/**
		 * \return the current color-gradient.
		 */
		QGradient gradient() const { return m_gradient; }
		/**
		 * \return the currently selected color.
		 */
		QColor color() const { return m_currentStop.second; }
		/**
		 * Set the orientation of the gradient strip.
		 */
		void setOrientation( Qt::Orientation orientation );
		/**
		 * \return the orientation of the gradient strip.
		 */
		Qt::Orientation orientation() const { return m_orientation; }
		
		QSize minimumSizeHint() const Q_DECL_OVERRIDE;
		
	public Q_SLOTS:
		/**
		 * Set the current gradient being edited.
		 */
		void setGradient( const QGradient & gradient );
		/**
		 * Set the color of the currently selected color-stop.
		 */
		void setColor( const QColor & color );
		/**
		 * Removes the currently selected stop.
		 */
		void removeStop();
		
	Q_SIGNALS:
		/**
		 * Emitted when a color-stop is selected (e.g. when it is clicked on
		 * or the previously selected color is removed).
		 */
		void colorSelected( const QColor & color );
		/**
		 * Emitted when the gradient changes.
		 */
		void gradientChanged( const QGradient & gradient );
		
	protected:
		void paintEvent( QPaintEvent * e ) Q_DECL_OVERRIDE;
		void mousePressEvent( QMouseEvent * e ) Q_DECL_OVERRIDE;
		void mouseMoveEvent( QMouseEvent * e ) Q_DECL_OVERRIDE;
		void mouseReleaseEvent( QMouseEvent * e ) Q_DECL_OVERRIDE;
		void mouseDoubleClickEvent( QMouseEvent * e ) Q_DECL_OVERRIDE;
		void contextMenuEvent( QContextMenuEvent * e ) Q_DECL_OVERRIDE;
		
	private:
		/**
		 * Attempts to get the arrow under the mouse, updating m_currentStop
		 * to the corresponding stop if an arrow was found.
		 * \return true if an arrow was under \p mousePos
		 * \return false if an arrow wasn't under \p mousePos
		 */
		bool getGradientStop( const QPoint & mousePos );
		/**
		 * Updates the current stop, redraws the widget and emits
		 * colorSelected.
		 */
		void setCurrentStop( const QGradientStop & stop );
		/**
		 * Updates m_gradient, redraws the widget and emits gradientChanged.
		 */
		void setGradient( const QGradientStops & stops );
		/**
		 * Draws the given gradient stop.
		 */
		void drawArrow( QPainter * painter, const QGradientStop & stop );
		/**
		 * Converts from \p stop (ranging from 0 to 1) to a horizontal or
		 * vertical (depending on the current orientation) widget coordinate.
		 */
		double toArrowPos( double stop ) const;
		/**
		 * Inverse of toArrowPos; converts from the widget coordinate \p pos
		 * to a stop position, which is guaranteed to be between 0 and 1.
		 */
		double fromArrowPos( double pos ) const;
		/**
		 * Find a gradient stop to be selected. This is called after e.g. the
		 * currently selected stop is deleted.
		 */
		void findGradientStop();
		
		double m_clickOffset; // from the center of the arrow
		bool m_haveArrow; // true when an arrow has been clicked on
		QGradientStop m_currentStop;
		QGradient m_gradient;
		Qt::Orientation m_orientation;
};


/**
 * \short A dialog for getting a QGradient.
 * 
 * This dialog contains a KGradientEditor for editing the gradient stops and
 * a selection of widgets for changing the current color.
 * 
 * Example:
 * 
 * \code
 *  QGradient gradient;
 *  int result = KGradientDialog::getColor( gradient );
 *  if ( result == KGradientDialog::Accepted )
 *      ...
 * \endcode
 */
class KGradientDialog : public QDialog
{
	Q_OBJECT
			
	public:
		KGradientDialog( QWidget * parent, bool modal );
		~KGradientDialog();
		
		/**
		 * Creates a modal gradient dialog, lets the user choose a gradient,
		 * and returns when the dialog is closed.
		 * 
		 * The initial gradient will be the one passed in \p gradient, and the
		 * chosen gradient is returned in this argument. If the user Cancels
		 * the dialog instead of Ok'ing it, then \p gradient will remain
		 * unchanged.
		 * 
		 * \returns QDialog::result()
		 */
		static int getGradient( QGradient & gradient, QWidget * parent = 0 );
		
		/**
		 * \return the current gradient.
		 */
		QGradient gradient() const;
		
	public Q_SLOTS:
		/**
		 * Sets the current gradient.
		 */
		void setGradient( const QGradient & gradient );
		
	Q_SIGNALS:
		/**
		 * Emitted when the current gradient changes.
		 */
		void gradientChanged( const QGradient & gradient );
	
	private:
		class KColorDialog * m_colorDialog;
		KGradientEditor * m_gradient;
};


/**
 * \short A button to display and edit color gradients
 * 
 * The button contains and displays a color gradient (see QGradient). When
 * clicked on, it creates a KGradientDialog for editing the gradient.
 */
class KGradientButton : public QPushButton
{
	Q_OBJECT
	Q_PROPERTY( QGradient gradient READ gradient WRITE setGradient USER true )
			
	public:
		explicit KGradientButton( QWidget * parent = 0 );
		~KGradientButton();
		
		/**
		 * \return the current gradient.
		 */
		QGradient gradient() const { return m_gradient; }
		
		QSize sizeHint() const Q_DECL_OVERRIDE;
		
	Q_SIGNALS:
		/**
		 * Emitted when the current gradient changes.
		 */
		void gradientChanged( const QGradient & gradient );
		
	public Q_SLOTS:
		/**
		 * Sets the current gradient.
		 */
		void setGradient( const QGradient & gradient );
		
	protected Q_SLOTS:
		/**
		 * Invokes the KGradientDialog for selecting a gradient.
		 */
		void chooseGradient();
		
	protected:
		void paintEvent( QPaintEvent *pe ) Q_DECL_OVERRIDE;
		
	private:
		void initStyleOption( QStyleOptionButton * opt ) const;
		QGradient m_gradient;
};
 
#endif // KGRADIENTEDITOR_H
