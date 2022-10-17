/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 1998, 1999, 2000, 2002 Klaus-Dieter Möller <kd.moeller@t-online.de>
    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef CONSTANT_H
#define CONSTANT_H

#include "function.h"

#include <QMap>
#include <QObject>

/**
 * Stores the details of a constant other than its name.
 * \author David Saxton
 */
class Constant
{
public:
    Constant();

    /**
     * The scope of the constant.
     */
    enum Type {
        Document = 0x1, ///< The constant is part of the document
        Global = 0x2, ///< The constant is to be saved globally in the application settings
        All = 0x4 - 1
    };

    /**
     * The actual value of the constant.
     */
    Value value;
    /**
     * The OR'ed types.
     */
    int type;
};

typedef QMap<QString, Constant> ConstantList;

/**
 * @short Manages a list of constants.
 */
class Constants : public QObject
{
    Q_OBJECT

public:
    Constants();
    virtual ~Constants();

    /**
     * Load the constants at the start.
     */
    void load();
    /**
     * Save the constants when closing the program.
     */
    void save();
    /**
     * \return if the constant name is valid.
     */
    bool isValidName(const QString &name) const;
    /**
     * \return the value of the constant with the given name. This will
     * return a default Value if the constant does not exist; use
     * Constants::have to check for existence.
     */
    Value value(const QString &name) const;
    /**
     * Removes the constant with the given name from the constants list.
     */
    void remove(const QString &name);
    /**
     * Adds the constant to the internal list (overwriting any previous
     * constant with the same name).
     */
    void add(const QString &name, const Constant &constant);
    /**
     * \return whether the constant with the given name exists.
     */
    bool have(const QString &name) const;
    /**
     * \return a unique (i.e. unused) constant name.
     */
    QString generateUniqueName() const;
    /**
     * \param type OR'ed list of Constant::Type
     * \return a copy of the list of constants.
     */
    ConstantList list(int type) const;
    /**
     * \return a list of the constant names.
     */
    QStringList names() const
    {
        return m_constants.keys();
    }

signals:
    /**
     * Emitted when a constant is added or removed, or the value of an
     * existing constant has changed.
     */
    void constantsChanged();

protected:
    ConstantList m_constants;
};

#endif // CONSTANT_H
