/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

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
    explicit EquationHighlighter(EquationEdit *parent);
    ~EquationHighlighter();

    /**
     * This is used to indicate the position where the error occurred.
     * If \p position is negative, then no error will be shown.
     */
    void setErrorPosition(int position);

protected:
    void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

    int m_errorPosition;
    EquationEdit *m_parent;
};

#endif
