/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar <f_edemar@linux.se>
*               2006  David Saxton <david@bluehaze.org>
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

#ifndef kmplotprogress_included
#define kmplotprogress_included

#include <QWidget>

class QPushButton;
class QProgressBar;
class QTimer;

/// The progress-widget in the statusbar which appears when drawing integrals
class KmPlotProgress: public QWidget
{
	Q_OBJECT
	public:
		KmPlotProgress( QWidget* parent = 0 );
		~KmPlotProgress();
		/**
		 * Sets proportion done (\p progress should be between 0 and 1). The
		 * progress bar won't be shown immediately, but only after a small
		 * delay.
		 *
		 * If \p progress is >= 1, then this progress bar will be hidden.
		 */
		void setProgress( double progress );

	Q_SIGNALS:
		/**
		 * Emitted when the stop button is clicked.
		 */
		void cancelDraw();

	private Q_SLOTS:
		/**
		 * Called from timeout of m_showTimer.
		 */
		void showProgressBar();

	private:
		QTimer * m_showTimer;
		QPushButton *m_button;
		QProgressBar *m_progress;
};

#endif // kmplotprogress_included
