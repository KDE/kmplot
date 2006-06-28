/*
 * This file was generated by dbusidl2cpp version 0.5
 * when processing input file org.kde.kmplot.Parser.xml
 *
 * dbusidl2cpp is Copyright (C) 2006 Trolltech AS. All rights reserved.
 *
 * This is an auto-generated file.
 */

#include "parseradaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class ParserAdaptor
 */

ParserAdaptor::ParserAdaptor(QObject *parent)
   : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

ParserAdaptor::~ParserAdaptor()
{
    // destructor
}

int ParserAdaptor::addFunction(const QString &f_str0, const QString &f_str1)
{
    // handle method call org.kde.kmplot.Parser.addFunction
    int out0;
    QMetaObject::invokeMethod(parent(), "addFunction", Q_RETURN_ARG(int, out0), Q_ARG(QString, f_str0), Q_ARG(QString, f_str1));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->addFunction(f_str0, f_str1);
    return out0;
}

uint ParserAdaptor::countFunctions()
{
    // handle method call org.kde.kmplot.Parser.countFunctions
    uint out0;
    QMetaObject::invokeMethod(parent(), "countFunctions", Q_RETURN_ARG(uint, out0));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->countFunctions();
    return out0;
}

double ParserAdaptor::fkt(uint id, uint eq, double eq_)
{
    // handle method call org.kde.kmplot.Parser.fkt
    double out0;
    QMetaObject::invokeMethod(parent(), "fkt", Q_RETURN_ARG(double, out0), Q_ARG(uint, id), Q_ARG(uint, eq), Q_ARG(double, eq_));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->fkt(id, eq, eq_);
    return out0;
}

int ParserAdaptor::fnameToID(const QString &name)
{
    // handle method call org.kde.kmplot.Parser.fnameToID
    int out0;
    QMetaObject::invokeMethod(parent(), "fnameToID", Q_RETURN_ARG(int, out0), Q_ARG(QString, name));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->fnameToID(name);
    return out0;
}

bool ParserAdaptor::functionAddParameter(const QString &new_parameter, uint id)
{
    // handle method call org.kde.kmplot.Parser.functionAddParameter
    bool out0;
    QMetaObject::invokeMethod(parent(), "functionAddParameter", Q_RETURN_ARG(bool, out0), Q_ARG(QString, new_parameter), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionAddParameter(new_parameter, id);
    return out0;
}

double ParserAdaptor::functionF1LineWidth(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionF1LineWidth
    double out0;
    QMetaObject::invokeMethod(parent(), "functionF1LineWidth", Q_RETURN_ARG(double, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionF1LineWidth(id);
    return out0;
}

bool ParserAdaptor::functionF1Visible(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionF1Visible
    bool out0;
    QMetaObject::invokeMethod(parent(), "functionF1Visible", Q_RETURN_ARG(bool, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionF1Visible(id);
    return out0;
}

double ParserAdaptor::functionF2LineWidth(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionF2LineWidth
    double out0;
    QMetaObject::invokeMethod(parent(), "functionF2LineWidth", Q_RETURN_ARG(double, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionF2LineWidth(id);
    return out0;
}

bool ParserAdaptor::functionF2Visible(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionF2Visible
    bool out0;
    QMetaObject::invokeMethod(parent(), "functionF2Visible", Q_RETURN_ARG(bool, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionF2Visible(id);
    return out0;
}

double ParserAdaptor::functionFLineWidth(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionFLineWidth
    double out0;
    QMetaObject::invokeMethod(parent(), "functionFLineWidth", Q_RETURN_ARG(double, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionFLineWidth(id);
    return out0;
}

bool ParserAdaptor::functionFVisible(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionFVisible
    bool out0;
    QMetaObject::invokeMethod(parent(), "functionFVisible", Q_RETURN_ARG(bool, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionFVisible(id);
    return out0;
}

double ParserAdaptor::functionIntLineWidth(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionIntLineWidth
    double out0;
    QMetaObject::invokeMethod(parent(), "functionIntLineWidth", Q_RETURN_ARG(double, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionIntLineWidth(id);
    return out0;
}

bool ParserAdaptor::functionIntVisible(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionIntVisible
    bool out0;
    QMetaObject::invokeMethod(parent(), "functionIntVisible", Q_RETURN_ARG(bool, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionIntVisible(id);
    return out0;
}

QString ParserAdaptor::functionMaxValue(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionMaxValue
    QString out0;
    QMetaObject::invokeMethod(parent(), "functionMaxValue", Q_RETURN_ARG(QString, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionMaxValue(id);
    return out0;
}

QString ParserAdaptor::functionMinValue(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionMinValue
    QString out0;
    QMetaObject::invokeMethod(parent(), "functionMinValue", Q_RETURN_ARG(QString, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionMinValue(id);
    return out0;
}

QStringList ParserAdaptor::functionParameterList(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionParameterList
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "functionParameterList", Q_RETURN_ARG(QStringList, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionParameterList(id);
    return out0;
}

bool ParserAdaptor::functionRemoveParameter(const QString &remove_parameter, uint id)
{
    // handle method call org.kde.kmplot.Parser.functionRemoveParameter
    bool out0;
    QMetaObject::invokeMethod(parent(), "functionRemoveParameter", Q_RETURN_ARG(bool, out0), Q_ARG(QString, remove_parameter), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionRemoveParameter(remove_parameter, id);
    return out0;
}

QString ParserAdaptor::functionStartXValue(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionStartXValue
    QString out0;
    QMetaObject::invokeMethod(parent(), "functionStartXValue", Q_RETURN_ARG(QString, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionStartXValue(id);
    return out0;
}

QString ParserAdaptor::functionStartYValue(uint id)
{
    // handle method call org.kde.kmplot.Parser.functionStartYValue
    QString out0;
    QMetaObject::invokeMethod(parent(), "functionStartYValue", Q_RETURN_ARG(QString, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionStartYValue(id);
    return out0;
}

QString ParserAdaptor::functionStr(uint eq, uint id)
{
    // handle method call org.kde.kmplot.Parser.functionStr
    QString out0;
    QMetaObject::invokeMethod(parent(), "functionStr", Q_RETURN_ARG(QString, out0), Q_ARG(uint, eq), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->functionStr(eq, id);
    return out0;
}

QStringList ParserAdaptor::listFunctionNames()
{
    // handle method call org.kde.kmplot.Parser.listFunctionNames
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "listFunctionNames", Q_RETURN_ARG(QStringList, out0));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->listFunctionNames();
    return out0;
}

bool ParserAdaptor::removeFunction(uint id)
{
    // handle method call org.kde.kmplot.Parser.removeFunction
    bool out0;
    QMetaObject::invokeMethod(parent(), "removeFunction", Q_RETURN_ARG(bool, out0), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->removeFunction(id);
    return out0;
}

bool ParserAdaptor::setFunctionExpression(const QString &f_str, uint id, uint eq)
{
    // handle method call org.kde.kmplot.Parser.setFunctionExpression
    bool out0;
    QMetaObject::invokeMethod(parent(), "setFunctionExpression", Q_RETURN_ARG(bool, out0), Q_ARG(QString, f_str), Q_ARG(uint, id), Q_ARG(uint, eq));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->setFunctionExpression(f_str, id, eq);
    return out0;
}

bool ParserAdaptor::setFunctionF1LineWidth(uint id, double linewidth)
{
    // handle method call org.kde.kmplot.Parser.setFunctionF1LineWidth
    bool out0;
    QMetaObject::invokeMethod(parent(), "setFunctionF1LineWidth", Q_RETURN_ARG(bool, out0), Q_ARG(uint, id), Q_ARG(double, linewidth));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->setFunctionF1LineWidth(id, linewidth);
    return out0;
}

void ParserAdaptor::setFunctionF1Visible(bool visible, uint id)
{
    // handle method call org.kde.kmplot.Parser.setFunctionF1Visible
    QMetaObject::invokeMethod(parent(), "setFunctionF1Visible", Q_ARG(bool, visible), Q_ARG(uint, id));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->setFunctionF1Visible(visible, id);
}

bool ParserAdaptor::setFunctionF2LineWidth(uint id, double linewidth)
{
    // handle method call org.kde.kmplot.Parser.setFunctionF2LineWidth
    bool out0;
    QMetaObject::invokeMethod(parent(), "setFunctionF2LineWidth", Q_RETURN_ARG(bool, out0), Q_ARG(uint, id), Q_ARG(double, linewidth));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->setFunctionF2LineWidth(id, linewidth);
    return out0;
}

void ParserAdaptor::setFunctionF2Visible(bool visible, uint id)
{
    // handle method call org.kde.kmplot.Parser.setFunctionF2Visible
    QMetaObject::invokeMethod(parent(), "setFunctionF2Visible", Q_ARG(bool, visible), Q_ARG(uint, id));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->setFunctionF2Visible(visible, id);
}

bool ParserAdaptor::setFunctionFLineWidth(uint id, double linewidth)
{
    // handle method call org.kde.kmplot.Parser.setFunctionFLineWidth
    bool out0;
    QMetaObject::invokeMethod(parent(), "setFunctionFLineWidth", Q_RETURN_ARG(bool, out0), Q_ARG(uint, id), Q_ARG(double, linewidth));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->setFunctionFLineWidth(id, linewidth);
    return out0;
}

void ParserAdaptor::setFunctionFVisible(bool visible, uint id)
{
    // handle method call org.kde.kmplot.Parser.setFunctionFVisible
    QMetaObject::invokeMethod(parent(), "setFunctionFVisible", Q_ARG(bool, visible), Q_ARG(uint, id));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->setFunctionFVisible(visible, id);
}

bool ParserAdaptor::setFunctionIntLineWidth(uint id, double linewidth)
{
    // handle method call org.kde.kmplot.Parser.setFunctionIntLineWidth
    bool out0;
    QMetaObject::invokeMethod(parent(), "setFunctionIntLineWidth", Q_RETURN_ARG(bool, out0), Q_ARG(uint, id), Q_ARG(double, linewidth));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->setFunctionIntLineWidth(id, linewidth);
    return out0;
}

void ParserAdaptor::setFunctionIntVisible(bool visible, uint id)
{
    // handle method call org.kde.kmplot.Parser.setFunctionIntVisible
    QMetaObject::invokeMethod(parent(), "setFunctionIntVisible", Q_ARG(bool, visible), Q_ARG(uint, id));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->setFunctionIntVisible(visible, id);
}

bool ParserAdaptor::setFunctionMaxValue(const QString &min, uint id)
{
    // handle method call org.kde.kmplot.Parser.setFunctionMaxValue
    bool out0;
    QMetaObject::invokeMethod(parent(), "setFunctionMaxValue", Q_RETURN_ARG(bool, out0), Q_ARG(QString, min), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->setFunctionMaxValue(min, id);
    return out0;
}

bool ParserAdaptor::setFunctionMinValue(const QString &min, uint id)
{
    // handle method call org.kde.kmplot.Parser.setFunctionMinValue
    bool out0;
    QMetaObject::invokeMethod(parent(), "setFunctionMinValue", Q_RETURN_ARG(bool, out0), Q_ARG(QString, min), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->setFunctionMinValue(min, id);
    return out0;
}

bool ParserAdaptor::setFunctionStartValue(const QString &x, const QString &y, uint id)
{
    // handle method call org.kde.kmplot.Parser.setFunctionStartValue
    bool out0;
    QMetaObject::invokeMethod(parent(), "setFunctionStartValue", Q_RETURN_ARG(bool, out0), Q_ARG(QString, x), Q_ARG(QString, y), Q_ARG(uint, id));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->setFunctionStartValue(x, y, id);
    return out0;
}


#include "parseradaptor.moc"