/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "equationeditwidget.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QWheelEvent>

#include "equationedit.h"

void EquationEditWidget::setClearSelectionOnFocusOut(bool doIt)
{
    m_clearSelectionOnFocusOut = doIt;
}

EquationEditWidget::EquationEditWidget(EquationEdit *parent)
    : KTextEdit(parent)
{
    m_clearSelectionOnFocusOut = true;
    m_parent = parent;
    recalculateGeometry();
}

void EquationEditWidget::recalculateGeometry()
{
    // Set fixed height
    ensurePolished();
    QFontMetrics fm(document()->defaultFont());
    int h = qMax(fm.lineSpacing(), 14) + 6;
    int m = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    QStyleOptionFrame opt;
    opt.rect = rect();
    opt.palette = palette();
    opt.state = QStyle::State_None;
    setFixedHeight(h + (2 * m));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setContentsMargins(0, 0, 0, 0);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWordWrapMode(QTextOption::NoWrap);
    setLineWrapMode(KTextEdit::NoWrap);
    setTabChangesFocus(true);
}

void EquationEditWidget::wheelEvent(QWheelEvent *e)
{
    e->accept();
}

void EquationEditWidget::keyPressEvent(QKeyEvent *e)
{
    if ((e->key() == Qt::Key_Return) || (e->key() == Qt::Key_Enter)) {
        e->accept();
        emit m_parent->editingFinished();
        emit m_parent->returnPressed();
    } else {
        // Still pass these keys to KTextEdit, in case the user has to scroll
        // up/down the text
        if (e->key() == Qt::Key_Up)
            emit m_parent->upPressed();
        else if (e->key() == Qt::Key_Down)
            emit m_parent->downPressed();

        KTextEdit::keyPressEvent(e);
    }
}

void EquationEditWidget::focusOutEvent(QFocusEvent *e)
{
    KTextEdit::focusOutEvent(e);

    if (m_clearSelectionOnFocusOut)
        clearSelection();
    m_parent->reHighlight();

    emit m_parent->editingFinished();
}

void EquationEditWidget::focusInEvent(QFocusEvent *e)
{
    KTextEdit::focusOutEvent(e);

    m_parent->reHighlight();
    if (e->reason() == Qt::TabFocusReason)
        selectAll();
}

void EquationEditWidget::clearSelection()
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
        return;

    cursor.clearSelection();
    setTextCursor(cursor);
}
