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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

#ifndef EQUATIONHIGHLIGHTER_H
#define EQUATIONHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class EquationEdit;
class QString;


/**
 * \author David Saxton
 */
class EquationHighlighter : public QSyntaxHighlighter
{
	public:
		explicit EquationHighlighter(EquationEdit* parent);
		~EquationHighlighter();
		
		/**
		 * This is used to indicate the position where the error occurred.
		 * If \p position is negative, then no error will be shown.
		 */
		void setErrorPosition(int position);
		
	protected:
		void highlightBlock(const QString& text) Q_DECL_OVERRIDE;
		
		int m_errorPosition;
		EquationEdit* m_parent;
};

#endif
