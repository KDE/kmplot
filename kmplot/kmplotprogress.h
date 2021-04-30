/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2004 Fredrik Edemar <f_edemar@linux.se>
    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

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
		explicit KmPlotProgress( QWidget* parent = 0 );
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
