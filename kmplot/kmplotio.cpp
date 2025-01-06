/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 1998, 1999, 2000, 2002 Klaus-Dieter Möller <kd.moeller@t-online.de>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "kmplotio.h"

// Qt includes
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomText>
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>

// KDE includes
#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>

// ANSI-C includes
#include <stdlib.h>

// local includes
#include "maindlg.h"
#include "settings.h"
#include "xparser.h"

static QString CurrentVersionString("4");

class XParser;

KmPlotIO::KmPlotIO()
{
    KmPlotIO::version = CurrentVersionString.toInt();
    lengthScaler = 1.0;
}

KmPlotIO::~KmPlotIO()
{
}

QDomDocument KmPlotIO::currentState()
{
    // saving as xml by a QDomDocument
    QDomDocument doc("kmpdoc");
    // the root tag
    QDomElement root = doc.createElement("kmpdoc");
    root.setAttribute("version", CurrentVersionString);
    doc.appendChild(root);

    // the axes tag
    QDomElement tag = doc.createElement("axes");

    tag.setAttribute("color", Settings::axesColor().name());
    tag.setAttribute("width", Settings::axesLineWidth());
    tag.setAttribute("tic-width", Settings::ticWidth());
    tag.setAttribute("tic-legth", Settings::ticLength());

    addTag(doc, tag, "show-axes", Settings::showAxes() ? "1" : "-1");
    addTag(doc, tag, "show-arrows", Settings::showArrows() ? "1" : "-1");
    addTag(doc, tag, "show-label", Settings::showLabel() ? "1" : "-1");

    addTag(doc, tag, "xmin", Settings::xMin());
    addTag(doc, tag, "xmax", Settings::xMax());

    addTag(doc, tag, "ymin", Settings::yMin());
    addTag(doc, tag, "ymax", Settings::yMax());

    root.appendChild(tag);

    tag = doc.createElement("grid");

    tag.setAttribute("color", Settings::gridColor().name());
    tag.setAttribute("width", Settings::gridLineWidth());

    addTag(doc, tag, "mode", QString::number(Settings::gridStyle()));

    root.appendChild(tag);

    tag = doc.createElement("scale");

    addTag(doc, tag, "tic-x-mode", QString::number(Settings::xScalingMode()));
    addTag(doc, tag, "tic-y-mode", QString::number(Settings::yScalingMode()));
    addTag(doc, tag, "tic-x", Settings::xScaling());
    addTag(doc, tag, "tic-y", Settings::yScaling());

    root.appendChild(tag);

    for (Function *f : std::as_const(XParser::self()->m_ufkt))
        addFunction(doc, root, f);

    addConstants(doc, root);

    tag = doc.createElement("fonts");
    addTag(doc, tag, "axes-font", Settings::axesFont().family());
    addTag(doc, tag, "label-font", Settings::labelFont().family());
    addTag(doc, tag, "header-table-font", Settings::headerTableFont().family());
    root.appendChild(tag);

    return doc;
}

bool KmPlotIO::save(const QUrl &url)
{
    QDomDocument doc = currentState();

    if (!url.isLocalFile()) {
        QTemporaryFile tmpfile;
        if (!tmpfile.open()) {
            qWarning() << "Could not open " << QUrl(tmpfile.fileName()).toLocalFile() << " for writing.\n";
            return false;
        }
        QTextStream ts(&tmpfile);
        doc.save(ts, 4);
        ts.flush();

        Q_CONSTEXPR int permission = -1;
        QFile file(tmpfile.fileName());
        file.open(QIODevice::ReadOnly);
        KIO::StoredTransferJob *putjob = KIO::storedPut(file.readAll(), url, permission, KIO::JobFlag::Overwrite);
        if (!putjob->exec()) {
            qWarning() << "Could not open " << url.toString() << " for writing (" << putjob->errorString() << ").\n";
            return false;
        }
        file.close();
    } else {
        QFile xmlfile(url.toLocalFile());
        if (!xmlfile.open(QIODevice::WriteOnly)) {
            qWarning() << "Could not open " << url.path() << " for writing.\n";
            return false;
        }
        QTextStream ts(&xmlfile);
        doc.save(ts, 4);
        xmlfile.close();
        return true;
    }
    return true;
}

void KmPlotIO::addConstants(QDomDocument &doc, QDomElement &root)
{
    ConstantList constants = XParser::self()->constants()->list(Constant::Document);

    for (ConstantList::iterator it = constants.begin(); it != constants.end(); ++it) {
        QDomElement tag = doc.createElement("constant");
        root.appendChild(tag);
        tag.setAttribute("name", it.key());
        tag.setAttribute("value", it.value().value.expression());
    }
}

void KmPlotIO::addFunction(QDomDocument &doc, QDomElement &root, Function *function)
{
    QDomElement tag = doc.createElement("function");

    QString names[] = {"f0", "f1", "f2", "integral"};
    PlotAppearance *plots[] = {&function->plotAppearance(Function::Derivative0),
                               &function->plotAppearance(Function::Derivative1),
                               &function->plotAppearance(Function::Derivative2),
                               &function->plotAppearance(Function::Integral)};

    for (int i = 0; i < 4; ++i) {
        tag.setAttribute(QString("%1-width").arg(names[i]), plots[i]->lineWidth);
        tag.setAttribute(QString("%1-color").arg(names[i]), plots[i]->color.name());
        tag.setAttribute(QString("%1-use-gradient").arg(names[i]), plots[i]->useGradient);
        tag.setAttribute(QString("%1-gradient").arg(names[i]), gradientToString(plots[i]->gradient.stops()));
        tag.setAttribute(QString("%1-show-tangent-field").arg(names[i]), plots[i]->showTangentField);
        tag.setAttribute(QString("%1-visible").arg(names[i]), plots[i]->visible);
        tag.setAttribute(QString("%1-style").arg(names[i]), PlotAppearance::penStyleToString(plots[i]->style));
        tag.setAttribute(QString("%1-show-extrema").arg(names[i]), plots[i]->showExtrema);
        tag.setAttribute(QString("%1-show-plot-name").arg(names[i]), plots[i]->showPlotName);
    }

    // BEGIN parameters
    tag.setAttribute("use-parameter-slider", function->m_parameters.useSlider);
    tag.setAttribute("parameter-slider", function->m_parameters.sliderID);

    tag.setAttribute("use-parameter-list", function->m_parameters.useList);
    QStringList str_parameters;
    for (const Value &k : std::as_const(function->m_parameters.list))
        str_parameters << k.expression();

    if (!str_parameters.isEmpty())
        addTag(doc, tag, "parameter-list", str_parameters.join(";"));
    // END parameters

    tag.setAttribute("type", Function::typeToString(function->type()));
    for (int i = 0; i < function->eq.size(); ++i) {
        Equation *equation = function->eq[i];

        QString fstr = equation->fstr();
        if (fstr.isEmpty())
            continue;
        QDomElement element = addTag(doc, tag, QString("equation-%1").arg(i), fstr);
        element.setAttribute("step", equation->differentialStates.step().expression());

        for (int i = 0; i < equation->differentialStates.size(); ++i) {
            DifferentialState *state = &equation->differentialStates[i];

            QDomElement differential = doc.createElement("differential");
            element.appendChild(differential);

            bool first = true;
            QString ys;
            for (const Value &y : std::as_const(state->y0)) {
                if (!first)
                    ys += ';';
                first = false;
                ys += y.expression();
            }

            differential.setAttribute("x", state->x0.expression());
            differential.setAttribute("y", ys);
        }
    }

    addTag(doc, tag, "arg-min", function->dmin.expression()).setAttribute("use", function->usecustomxmin);
    addTag(doc, tag, "arg-max", function->dmax.expression()).setAttribute("use", function->usecustomxmax);

    root.appendChild(tag);
}

QDomElement KmPlotIO::addTag(QDomDocument &doc, QDomElement &parentTag, const QString &tagName, const QString &tagValue)
{
    QDomElement tag = doc.createElement(tagName);
    QDomText value = doc.createTextNode(tagValue);
    tag.appendChild(value);
    parentTag.appendChild(tag);
    return tag;
}

bool KmPlotIO::restore(const QDomDocument &doc)
{
    // temporary measure: for now, delete all previous functions
    XParser::self()->removeAllFunctions();

    QDomElement element = doc.documentElement();
    QString versionString = element.attribute("version");
    if (versionString.isNull()) // an old kmplot-file
    {
        MainDlg::oldfileversion = true;
        for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            version = 0;
            lengthScaler = 0.1;

            if (n.nodeName() == "axes")
                parseAxes(n.toElement());
            if (n.nodeName() == "grid")
                parseGrid(n.toElement());
            if (n.nodeName() == "scale")
                parseScale(n.toElement());
            if (n.nodeName() == "function")
                oldParseFunction(n.toElement());
        }
    } else if (versionString == "1" || versionString == "2" || versionString == "3" || versionString == "4") {
        MainDlg::oldfileversion = false;
        version = versionString.toInt();
        lengthScaler = (version < 3) ? 0.1 : 1.0;

        for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
            if (n.nodeName() == "axes")
                parseAxes(n.toElement());
            else if (n.nodeName() == "grid")
                parseGrid(n.toElement());
            else if (n.nodeName() == "scale")
                parseScale(n.toElement());
            else if (n.nodeName() == "constant")
                parseConstant(n.toElement());
            else if (n.nodeName() == "function") {
                if (version < 3)
                    oldParseFunction2(n.toElement());
                else
                    parseFunction(n.toElement());
            }
        }
    } else {
        KMessageBox::error(nullptr, i18n("The file had an unknown version number"));
        return false;
    }

    // Because we may not have loaded the constants / functions in the right order
    // to account for dependencies
    XParser::self()->reparseAllFunctions();

    return true;
}

bool KmPlotIO::load(const QUrl &url)
{
    QDomDocument doc("kmpdoc");
    QFile f;
    bool downloadedFile = false;
    if (!url.isLocalFile()) {
        if (!MainDlg::fileExists(url)) {
            KMessageBox::error(nullptr, i18n("The file does not exist."));
            return false;
        }
        downloadedFile = true;
        KIO::StoredTransferJob *transferjob = KIO::storedGet(url);
        KJobWidgets::setWindow(transferjob, nullptr);
        if (!transferjob->exec()) {
            KMessageBox::error(nullptr, i18n("An error appeared when opening this file (%1)", transferjob->errorString()));
            return false;
        }
        QTemporaryFile file;
        file.setAutoRemove(false);
        file.open();
        file.write(transferjob->data());
        f.setFileName(file.fileName());
        file.close();
    } else
        f.setFileName(url.toLocalFile());

    if (!f.open(QIODevice::ReadOnly)) {
        KMessageBox::error(nullptr, i18n("%1 could not be opened", f.fileName()));
        return false;
    }
    QString errorMessage;
    int errorLine, errorColumn;
    if (!doc.setContent(&f, &errorMessage, &errorLine, &errorColumn)) {
        KMessageBox::error(nullptr, i18n("%1 could not be loaded (%2 at line %3, column %4)", f.fileName(), errorMessage, errorLine, errorColumn));
        f.close();
        return false;
    }
    f.close();

    if (!restore(doc))
        return false;

    if (downloadedFile)
        QFile::remove(f.fileName());
    return true;
}

void KmPlotIO::parseConstant(const QDomElement &n)
{
    QString name = n.attribute("name");
    QString value = n.attribute("value");

    /// \todo how to handle overwriting constants, etc?
    Constant c;
    c.value.updateExpression(value);
    c.type = Constant::Document;

    if (XParser::self()->constants()->list(Constant::Global).contains(name))
        c.type |= Constant::Global;

    XParser::self()->constants()->add(name, c);
}

void KmPlotIO::parseAxes(const QDomElement &n)
{
    Settings::setAxesLineWidth(n.attribute("width", (version < 3) ? "2" : "0.2").toDouble() * lengthScaler);
    Settings::setAxesColor(QColor(n.attribute("color", "#000000")));
    Settings::setTicWidth(n.attribute("tic-width", (version < 3) ? "3" : "0.3").toDouble() * lengthScaler);
    Settings::setTicLength(n.attribute("tic-length", (version < 3) ? "5" : "0.5").toDouble() * lengthScaler);

    if (version < 1) {
        Settings::setShowAxes(true);
        Settings::setShowArrows(true);
        Settings::setShowLabel(true);
    } else {
        Settings::setShowAxes(n.namedItem("show-axes").toElement().text().toInt() == 1);
        Settings::setShowArrows(n.namedItem("show-arrows").toElement().text().toInt() == 1);
        Settings::setShowLabel(n.namedItem("show-label").toElement().text().toInt() == 1);
    }

    Settings::setXMin(n.namedItem("xmin").toElement().text());
    Settings::setXMax(n.namedItem("xmax").toElement().text());
    Settings::setYMin(n.namedItem("ymin").toElement().text());
    Settings::setYMax(n.namedItem("ymax").toElement().text());
}

void KmPlotIO::parseGrid(const QDomElement &n)
{
    Settings::setGridColor(QColor(n.attribute("color", "#c0c0c0")));
    Settings::setGridLineWidth(n.attribute("width", (version < 3) ? "1" : "0.1").toDouble() * lengthScaler);

    Settings::setGridStyle(n.namedItem("mode").toElement().text().toInt());
}

int unit2index(const QString &unit)
{
    QString units[9] = {"10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4", i18n("automatic")};
    int index = 0;
    while ((index < 9) && (unit != units[index]))
        index++;
    if (index == 9)
        index = -1;
    return index;
}

void KmPlotIO::parseScale(const QDomElement &n)
{
#if 0
	if ( version < 1 )
	{
		Settings::setXScaling( unit2index( n.namedItem( "tic-x" ).toElement().text() ) );
		Settings::setYScaling( unit2index( n.namedItem( "tic-y" ).toElement().text() ) );
		Settings::setXPrinting( unit2index( n.namedItem( "print-tic-x" ).toElement().text() ) );
		Settings::setYPrinting( unit2index( n.namedItem( "print-tic-y" ).toElement().text() ) );
	}
	else
	{
		Settings::setXScaling( n.namedItem( "tic-x" ).toElement().text().toInt() );
		Settings::setYScaling( n.namedItem( "tic-y" ).toElement().text().toInt() );
		Settings::setXPrinting( n.namedItem( "print-tic-x" ).toElement().text().toInt()  );
		Settings::setYPrinting( n.namedItem( "print-tic-y" ).toElement().text().toInt() );
	}
#endif

    if (version >= 4) {
        Settings::setXScalingMode(n.namedItem("tic-x-mode").toElement().text().toInt());
        Settings::setYScalingMode(n.namedItem("tic-y-mode").toElement().text().toInt());
        Settings::setXScaling(n.namedItem("tic-x").toElement().text());
        Settings::setYScaling(n.namedItem("tic-y").toElement().text());
    }
}

void KmPlotIO::parseFunction(const QDomElement &n, bool allowRename)
{
    QDomElement equation0 = n.namedItem("equation-0").toElement();
    QDomElement equation1 = n.namedItem("equation-1").toElement();

    QString eq0 = equation0.text();
    QString eq1 = equation1.text();

    Function::Type type = Function::stringToType(n.attribute("type"));

    if (allowRename) {
        switch (type) {
        case Function::Polar:
            XParser::self()->fixFunctionName(eq0, Equation::Polar, -1);
            break;

        case Function::Parametric:
            XParser::self()->fixFunctionName(eq0, Equation::ParametricX, -1);
            XParser::self()->fixFunctionName(eq1, Equation::ParametricY, -1);
            break;

        case Function::Cartesian:
            XParser::self()->fixFunctionName(eq0, Equation::Cartesian, -1);
            break;

        case Function::Implicit:
            XParser::self()->fixFunctionName(eq0, Equation::Implicit, -1);
            break;

        case Function::Differential:
            XParser::self()->fixFunctionName(eq0, Equation::Differential, -1);
            break;
        }
    }

    int functionID = XParser::self()->Parser::addFunction(eq0, eq1, type, true);
    if (functionID == -1) {
        qWarning() << "Could not create function!\n";
        return;
    }
    Function *function = XParser::self()->functionWithID(functionID);

    parseDifferentialStates(equation0, function->eq[0]);
    if (function->eq.size() > 1)
        parseDifferentialStates(equation1, function->eq[1]);

    PlotAppearance *plots[] = {&function->plotAppearance(Function::Derivative0),
                               &function->plotAppearance(Function::Derivative1),
                               &function->plotAppearance(Function::Derivative2),
                               &function->plotAppearance(Function::Integral)};

    QString names[] = {"f0", "f1", "f2", "integral"};

    for (int i = 0; i < 4; ++i) {
        plots[i]->lineWidth = n.attribute(QString("%1-width").arg(names[i])).toDouble() * lengthScaler;
        plots[i]->color = n.attribute(QString("%1-color").arg(names[i]));
        plots[i]->useGradient = n.attribute(QString("%1-use-gradient").arg(names[i])).toInt();
        plots[i]->gradient.setStops(stringToGradient(n.attribute(QString("%1-gradient").arg(names[i]))));
        plots[i]->visible = n.attribute(QString("%1-visible").arg(names[i])).toInt();
        plots[i]->style = PlotAppearance::stringToPenStyle(n.attribute(QString("%1-style").arg(names[i])));
        plots[i]->showExtrema = n.attribute(QString("%1-show-extrema").arg(names[i])).toInt();
        plots[i]->showTangentField = n.attribute(QString("%1-show-tangent-field").arg(names[i])).toInt();
        plots[i]->showPlotName = n.attribute(QString("%1-show-plot-name").arg(names[i])).toInt();
    }

    // BEGIN parameters
    parseParameters(n, function);

    function->m_parameters.useSlider = n.attribute("use-parameter-slider").toInt();
    function->m_parameters.sliderID = n.attribute("parameter-slider").toInt();
    function->m_parameters.useList = n.attribute("use-parameter-list").toInt();
    // END parameters

    QDomElement minElement = n.namedItem("arg-min").toElement();
    QString expression = minElement.text();
    if (expression.isEmpty())
        function->usecustomxmin = false;
    else {
        function->dmin.updateExpression(expression);
        function->usecustomxmin = minElement.attribute("use", "1").toInt();
    }

    QDomElement maxElement = n.namedItem("arg-max").toElement();
    expression = maxElement.text();
    if (expression.isEmpty())
        function->usecustomxmax = false;
    else {
        function->dmax.updateExpression(expression);
        function->usecustomxmax = maxElement.attribute("use", "1").toInt();
    }
}

void KmPlotIO::parseParameters(const QDomElement &n, Function *function)
{
    QChar separator = (version < 1) ? ',' : ';';
    QString tagName = (version < 4) ? "parameterlist" : "parameter-list";

    const QStringList str_parameters = n.namedItem(tagName).toElement().text().split(separator, Qt::SkipEmptyParts);
    for (QStringList::const_iterator it = str_parameters.constBegin(); it != str_parameters.constEnd(); ++it)
        function->m_parameters.list.append(Value(*it));
}

void KmPlotIO::parseDifferentialStates(const QDomElement &n, Equation *equation)
{
    equation->differentialStates.setStep(n.attribute("step"));

    QDomNode node = n.firstChild();

    while (!node.isNull()) {
        if (node.isElement()) {
            QDomElement e = node.toElement();

            QString x = e.attribute("x");
            const QStringList y = e.attribute("y").split(';');

            DifferentialState *state = equation->differentialStates.add();
            if (state->y0.size() != y.size()) {
                qWarning() << "Invalid y count!\n";
                return;
            }

            state->x0.updateExpression(x);

            int at = 0;
            for (const QString &f : y)
                state->y0[at++] = f;
        }
        node = node.nextSibling();
    }
}

void KmPlotIO::oldParseFunction2(const QDomElement &n)
{
    Function::Type type;
    QString eq0, eq1;

    eq0 = n.namedItem("equation").toElement().text();

    switch (eq0[0].unicode()) {
    case 'r':
        type = Function::Polar;
        break;

    case 'x':
        parametricXEquation = eq0;
        return;

    case 'y':
        type = Function::Parametric;
        eq1 = eq0;
        eq0 = parametricXEquation;
        break;

    default:
        type = Function::Cartesian;
        break;
    }

    Function ufkt(type);
    ufkt.eq[0]->setFstr(eq0, nullptr, nullptr, true);
    if (!eq1.isEmpty())
        ufkt.eq[1]->setFstr(eq1, nullptr, nullptr, true);

    PlotAppearance *plots[] = {&ufkt.plotAppearance(Function::Derivative0),
                               &ufkt.plotAppearance(Function::Derivative1),
                               &ufkt.plotAppearance(Function::Derivative2),
                               &ufkt.plotAppearance(Function::Integral)};

    plots[0]->visible = n.attribute("visible").toInt();
    plots[0]->color = QColor(n.attribute("color"));
    plots[0]->lineWidth = n.attribute("width").toDouble() * lengthScaler;

    plots[1]->visible = n.attribute("visible-deriv", "0").toInt();
    plots[1]->color = QColor(n.attribute("deriv-color"));
    plots[1]->lineWidth = n.attribute("deriv-width").toDouble() * lengthScaler;

    plots[2]->visible = n.attribute("visible-2nd-deriv", "0").toInt();
    plots[2]->color = QColor(n.attribute("deriv2nd-color"));
    plots[2]->lineWidth = n.attribute("deriv2nd-width").toDouble() * lengthScaler;

    plots[3]->visible = n.attribute("visible-integral", "0").toInt();
    plots[3]->color = QColor(n.attribute("integral-color"));
    plots[3]->lineWidth = n.attribute("integral-width").toDouble() * lengthScaler;

    // BEGIN parameters
    parseParameters(n, &ufkt);

    int use_slider = n.attribute("use-slider").toInt();
    ufkt.m_parameters.useSlider = (use_slider >= 0);
    ufkt.m_parameters.sliderID = use_slider;

    ufkt.m_parameters.useList = !ufkt.m_parameters.list.isEmpty();
    // END parameters

    if (type == Function::Cartesian) {
        DifferentialState *state = &ufkt.eq[0]->differentialStates[0];
        state->x0.updateExpression(n.attribute("integral-startx"));
        state->y0[0].updateExpression(n.attribute("integral-starty"));
    }

    QDomElement minElement = n.namedItem("arg-min").toElement();
    QString expression = minElement.text();
    if (expression.isEmpty())
        ufkt.usecustomxmin = false;
    else {
        ufkt.dmin.updateExpression(expression);
        ufkt.usecustomxmin = minElement.attribute("use", "1").toInt();
    }

    QDomElement maxElement = n.namedItem("arg-max").toElement();
    expression = maxElement.text();
    if (expression.isEmpty())
        ufkt.usecustomxmax = false;
    else {
        ufkt.dmax.updateExpression(expression);
        ufkt.usecustomxmax = maxElement.attribute("use", "1").toInt();
    }

    QString fstr = ufkt.eq[0]->fstr();
    if (!fstr.isEmpty()) {
        int const i = fstr.indexOf(';');
        QString str;
        if (i == -1)
            str = fstr;
        else
            str = fstr.left(i);

        int id = XParser::self()->Parser::addFunction(str, eq1, type, true);

        Function *added_function = XParser::self()->m_ufkt[id];
        added_function->copyFrom(ufkt);
    }
}

void KmPlotIO::oldParseFunction(const QDomElement &n)
{
    QString tmp_fstr = n.namedItem("equation").toElement().text();
    if (tmp_fstr.isEmpty()) {
        qWarning() << "tmp_fstr is empty!\n";
        return;
    }

    Function::Type type;
    switch (tmp_fstr[0].unicode()) {
    case 'r':
        type = Function::Polar;
        break;

    case 'x':
        parametricXEquation = tmp_fstr;
        return;

    case 'y':
        type = Function::Parametric;
        break;

    default:
        type = Function::Cartesian;
        break;
    }

    Function ufkt(type);

    ufkt.plotAppearance(Function::Derivative0).visible = n.attribute("visible").toInt();
    ufkt.plotAppearance(Function::Derivative1).visible = n.attribute("visible-deriv").toInt();
    ufkt.plotAppearance(Function::Derivative2).visible = n.attribute("visible-2nd-deriv").toInt();
    ufkt.plotAppearance(Function::Derivative0).lineWidth = n.attribute("width").toDouble() * lengthScaler;
    ufkt.plotAppearance(Function::Derivative0).color = ufkt.plotAppearance(Function::Derivative1).color = ufkt.plotAppearance(Function::Derivative2).color =
        ufkt.plotAppearance(Function::Integral).color = QColor(n.attribute("color"));

    QString expression = n.namedItem("arg-min").toElement().text();
    ufkt.dmin.updateExpression(expression);
    ufkt.usecustomxmin = !expression.isEmpty();

    expression = n.namedItem("arg-max").toElement().text();
    ufkt.dmax.updateExpression(expression);
    ufkt.usecustomxmax = !expression.isEmpty();

    if (ufkt.usecustomxmin && ufkt.usecustomxmax && ufkt.dmin.expression() == ufkt.dmax.expression()) {
        ufkt.usecustomxmin = false;
        ufkt.usecustomxmax = false;
    }

    const int pos = tmp_fstr.indexOf(';');
    if (pos == -1)
        ufkt.eq[0]->setFstr(tmp_fstr, nullptr, nullptr, true);
    else {
        ufkt.eq[0]->setFstr(tmp_fstr.left(pos), nullptr, nullptr, true);
        if (!XParser::self()->getext(&ufkt, tmp_fstr)) {
            KMessageBox::error(nullptr, i18n("The function %1 could not be loaded", ufkt.eq[0]->fstr()));
            return;
        }
    }

    QString fstr = ufkt.eq[0]->fstr();
    if (!fstr.isEmpty()) {
        int const i = fstr.indexOf(';');
        QString str;
        if (i == -1)
            str = fstr;
        else
            str = fstr.left(i);

        int id;
        if (type == Function::Parametric)
            id = XParser::self()->Parser::addFunction(str, parametricXEquation, type, true);
        else
            id = XParser::self()->Parser::addFunction(str, nullptr, type, true);

        Function *added_function = XParser::self()->m_ufkt[id];
        added_function->copyFrom(ufkt);
    }
}

// static
QString KmPlotIO::gradientToString(const QGradientStops &stops)
{
    QString string;
    for (const QGradientStop &stop : std::as_const(stops))
        string += QString("%1;%2,").arg(stop.first).arg(stop.second.name());
    return string;
}

// static
QGradientStops KmPlotIO::stringToGradient(const QString &string)
{
    const QStringList stopStrings = string.split(',', Qt::SkipEmptyParts);

    QGradientStops stops;
    for (const QString &stopString : stopStrings) {
        QString pos = stopString.section(';', 0, 0);
        QString color = stopString.section(';', 1, 1);

        QGradientStop stop;
        stop.first = pos.toDouble();
        stop.second = color;
        stops << stop;
    }

    return stops;
}
