/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2004 Fredrik Edemar <f_edemar@linux.se>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef COORDSCONFIGDIALOG_H
#define COORDSCONFIGDIALOG_H

#include <KConfigDialog>

namespace Ui
{
class EditCoords;
}

class EditCoords;

/**
@author Fredrik Edemar
*/
///
class CoordsConfigDialog : public KConfigDialog
{
    Q_OBJECT
public:
    explicit CoordsConfigDialog(QWidget *parent = nullptr);
    ~CoordsConfigDialog();
    /**
     * Updates the widgets containing the min/max values.
     */
    void updateXYRange();

public slots:
    virtual void done(int result) override;

protected slots:
    void updateButtons();

private:
    bool evalX(bool showError = true);
    bool evalY(bool showError = true);

    EditCoords *configAxesDialog;
};

#endif
