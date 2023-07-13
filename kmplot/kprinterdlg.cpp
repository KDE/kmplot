/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 1998, 1999, 2000, 2002 Klaus-Dieter Möller <kd.moeller@t-online.de>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "kprinterdlg.h"

// Qt includes
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>

// KDE includes
#include <KLocalizedString>

// local includes
#include "equationedit.h"
#include "parser.h"

KPrinterDlg::KPrinterDlg(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(i18nc("@title:window", "KmPlot Options"));

    QGridLayout *layout = new QGridLayout(this);

    m_printHeaderTable = new QCheckBox(i18n("Print header table"), this);
    m_transparentBackground = new QCheckBox(i18n("Transparent background"), this);

    m_widthEdit = new EquationEdit(this);
    m_heightEdit = new EquationEdit(this);

    m_widthEdit->setText(QStringLiteral("12"));
    m_heightEdit->setText(QStringLiteral("12"));

    m_lengthScalingCombo = new QComboBox(this);
    m_lengthScalingCombo->addItem(i18n("Pixels (1/72nd in)"));
    m_lengthScalingCombo->addItem(i18n("Inches (in)"));
    m_lengthScalingCombo->addItem(i18n("Centimeters (cm)"));
    m_lengthScalingCombo->addItem(i18n("Millimeters (mm)"));

    m_lengthScalingCombo->setCurrentIndex(2); // default of centimeters

    QLabel *widthLabel = new QLabel(i18n("Width:"), this);
    QLabel *heightLabel = new QLabel(i18n("Height:"), this);

    layout->addWidget(m_printHeaderTable, 0, 0, 1, 2);
    layout->addWidget(m_transparentBackground, 1, 0, 1, 2);
    layout->addWidget(widthLabel, 2, 0, 1, 1);
    layout->addWidget(m_widthEdit, 2, 1, 1, 1);
    layout->addWidget(heightLabel, 3, 0, 1, 1);
    layout->addWidget(m_heightEdit, 3, 1, 1, 1);
    layout->addWidget(m_lengthScalingCombo, 4, 1, 1, 1);

    layout->setRowStretch(5, 1);
}

bool KPrinterDlg::printHeaderTable()
{
    return m_printHeaderTable->isChecked();
}

void KPrinterDlg::setPrintHeaderTable(bool status)
{
    m_printHeaderTable->setChecked(status);
}

bool KPrinterDlg::printBackground()
{
    return !m_transparentBackground->isChecked();
}

void KPrinterDlg::setPrintBackground(bool status)
{
    m_transparentBackground->setChecked(!status);
}

double KPrinterDlg::printWidth()
{
    return m_widthEdit->value() * lengthScaling();
}

void KPrinterDlg::setPrintWidth(double _width)
{
    double width = _width / lengthScaling();

    if (width <= 0)
        width = 0.12 / lengthScaling();

    m_widthEdit->setText(Parser::number(width));
}

double KPrinterDlg::printHeight()
{
    return m_heightEdit->value() * lengthScaling();
}

void KPrinterDlg::setPrintHeight(double _height)
{
    double height = _height / lengthScaling();

    if (height <= 0)
        height = 0.12 / lengthScaling();

    m_heightEdit->setText(Parser::number(height));
}

bool KPrinterDlg::isValid(QString &msg)
{
    bool ok;

    m_widthEdit->value(&ok);
    if (!ok) {
        msg = i18n("Width is invalid");
        return false;
    }

    m_heightEdit->value(&ok);
    if (!ok) {
        msg = i18n("Height is invalid");
        return false;
    }

    return true;
}

double KPrinterDlg::scalingToMeter(LengthScaling scaling)
{
    switch (scaling) {
    case Centimeters:
        return 0.01;
    case Millimeters:
        return 0.001;
    case Inches:
        return 0.0254;
    case Pixels:
        return 0.0254 / 72.0;
    }

    return 1;
}

double KPrinterDlg::lengthScaling() const
{
    LengthScaling scaling = (LengthScaling)m_lengthScalingCombo->currentIndex();
    return scalingToMeter(scaling);
}

#include "moc_kprinterdlg.cpp"
