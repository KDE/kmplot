/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef EQUATIONEDITWIDGET_H
#define EQUATIONEDITWIDGET_H

#include <KTextEdit>

class EquationEdit;

/**
 * The actual line edit.
 */
class EquationEditWidget : public KTextEdit
{
public:
    explicit EquationEditWidget(EquationEdit *parent);

    /**
     * Call this after changing font size.
     */
    void recalculateGeometry();
    /**
     * Whether to clear the selection when focus is lost.
     */
    void setClearSelectionOnFocusOut(bool doIt);

protected:
    void clearSelection();

    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;

    EquationEdit *m_parent;
    bool m_clearSelectionOnFocusOut;
};

#endif
