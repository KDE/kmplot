/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "parameterswidget.h"

#include "kparametereditor.h"
#include "xparser.h"
#include <QPointer>

// BEGIN class ParametersWidget
ParametersWidget::ParametersWidget(QWidget *parent)
    : QGroupBox(parent)
{
    setupUi(this);

    for (int number = 0; number < SLIDER_COUNT; number++)
        listOfSliders->addItem(i18n("Slider No. %1", number + 1));

    connect(editParameterListButton, &QPushButton::clicked, this, &ParametersWidget::editParameterList);
    connect(useSlider, &QCheckBox::toggled, this, &ParametersWidget::updateEquationEdits);
    connect(useList, &QCheckBox::toggled, this, &ParametersWidget::updateEquationEdits);
}

void ParametersWidget::init(const ParameterSettings &settings)
{
    useSlider->setChecked(settings.useSlider);
    useList->setChecked(settings.useList);

    listOfSliders->setCurrentIndex(settings.sliderID);
    m_parameters = settings.list;
}

ParameterSettings ParametersWidget::parameterSettings() const
{
    ParameterSettings s;

    s.useSlider = useSlider->isChecked();
    s.useList = useList->isChecked();

    s.sliderID = listOfSliders->currentIndex();
    s.list = m_parameters;

    return s;
}

void ParametersWidget::editParameterList()
{
    QPointer<KParameterEditor> dlg = new KParameterEditor(&m_parameters, nullptr);
    dlg->exec();
    delete dlg;
    Q_EMIT parameterListChanged();
}

void ParametersWidget::updateEquationEdits()
{
    if (!useSlider->isChecked() && !useList->isChecked()) {
        // Don't need to add any parameter variables
        return;
    }

    for (EquationEdit *edit : std::as_const(m_equationEdits)) {
        if (edit->equation()->usesParameter() || !edit->equation()->looksLikeFunction())
            continue;

        QString text = edit->text();
        int bracket = text.indexOf(')');
        if (bracket < 0)
            continue;

        text.replace(bracket, 1, ",k)");
        edit->setText(text);
    }
}

void ParametersWidget::associateEquationEdit(EquationEdit *edit)
{
    m_equationEdits << edit;
}
// END class ParametersWidget

#include "moc_parameterswidget.cpp"
