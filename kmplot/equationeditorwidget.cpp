/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "equationeditorwidget.h"

#include <KAcceleratorManager>

#include "equationeditwidget.h"
#include "maindlg.h"
#include "xparser.h"

EquationEditorWidget::EquationEditorWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    QFont font;
    double pointSize = font.pointSizeF() * 1.1;
    font.setPointSizeF(pointSize);
    edit->m_equationEditWidget->document()->setDefaultFont(font);
    edit->m_equationEditWidget->recalculateGeometry();

    QFont buttonFont;
    buttonFont.setPointSizeF(font.pointSizeF() * 1.1);

    const QList<QToolButton *> buttons = findChildren<QToolButton *>();
    for (QToolButton *w : buttons) {
        KAcceleratorManager::setNoAccel(w);

        connect(w, &QToolButton::clicked, this, &EquationEditorWidget::characterButtonClicked);

        // Also increase the font size, since the fractions, etc are probably not that visible
        // at the default font size
        w->setFont(buttonFont);
    }

    connect(constantsButton, &QPushButton::clicked, this, &EquationEditorWidget::editConstants);
    connect(functionList, &QComboBox::textActivated, this, &EquationEditorWidget::insertFunction);
    connect(constantList, QOverload<int>::of(&QComboBox::activated), this, &EquationEditorWidget::insertConstant);

    QStringList functions = XParser::self()->predefinedFunctions(false);
    functions.sort();
    functionList->addItems(functions);

    connect(XParser::self()->constants(), &Constants::constantsChanged, this, &EquationEditorWidget::updateConstantList);
    updateConstantList();
}

void EquationEditorWidget::updateConstantList()
{
    QStringList items;

    // The first item text is "Insert constant..."
    items << constantList->itemText(0);

    ConstantList constants = XParser::self()->constants()->list(Constant::All);
    for (ConstantList::iterator it = constants.begin(); it != constants.end(); ++it) {
        QString text = it.key() + " = " + it.value().value.expression();
        items << text;
    }

    constantList->clear();
    constantList->addItems(items);
}

void EquationEditorWidget::insertFunction(const QString &function)
{
    if (functionList->currentIndex() == 0)
        return;

    functionList->setCurrentIndex(0);
    edit->wrapSelected(function + '(', ")");
    edit->setFocus();
}

void EquationEditorWidget::editConstants()
{
    MainDlg::self()->editConstantsModal(this);
}

void EquationEditorWidget::insertConstant(int index)
{
    if (index == 0)
        return;

    ConstantList constants = XParser::self()->constants()->list(Constant::All);

    if (constants.size() < index)
        return;

    // Don't forget that index==0 corresponds to "Insert constant..."

    ConstantList::iterator it = constants.begin();
    int at = 0;
    while (++at < index)
        ++it;

    QString constant = it.key();

    constantList->setCurrentIndex(0);
    edit->insertText(constant);
    edit->setFocus();
}

void EquationEditorWidget::characterButtonClicked()
{
    const QToolButton *tb = static_cast<const QToolButton *>(sender());

    // Something (I can't work out what) is 'helpfully' inserting an ampersand (for keyboard acceleration).
    // Get rid of it.
    edit->insertText(tb->text().remove('&'));
}

#include "moc_equationeditorwidget.cpp"
