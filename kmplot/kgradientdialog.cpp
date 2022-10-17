/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "kgradientdialog.h"

#include <KLocalizedString>

#include <QApplication>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLinearGradient>
#include <QPaintEvent>
#include <QPainter>
#include <QPointer>
#include <QStyleOption>
#include <QStyleOptionButton>
#include <QTileRules>
#include <QVBoxLayout>

const double SQRT_3 = 1.732050808;
const double ArrowLength = 8;
const double ArrowHalfWidth = ArrowLength / SQRT_3;

// BEGIN class KGradientEditor
KGradientEditor::KGradientEditor(QWidget *parent)
    : QWidget(parent)
{
    m_haveArrow = false;
    m_clickOffset = 0;
    m_orientation = Qt::Horizontal;
    findGradientStop();
}

KGradientEditor::~KGradientEditor()
{
}

void KGradientEditor::setGradient(const QGradient &gradient)
{
    if (m_gradient == gradient)
        return;
    setGradient(gradient.stops());
    findGradientStop();
}

void KGradientEditor::setColor(const QColor &color)
{
    // Hmm...why doesn't qvector have some sortof search / replace functionality?
    QGradientStops stops = m_gradient.stops();
    for (int i = 0; i < stops.size(); ++i) {
        if (stops[i] != m_currentStop)
            continue;

        if (stops[i].second == color)
            return;

        m_currentStop.second = color;
        stops[i] = m_currentStop;
        break;
    }

    setGradient(stops);
}

QSize KGradientEditor::minimumSizeHint() const
{
    double w = 3 * ArrowHalfWidth;
    double h = 12 + ArrowLength;

    if (m_orientation == Qt::Vertical)
        qSwap(w, h);

    return QSizeF(w, h).toSize();
}

void KGradientEditor::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // BEGIN draw gradient
    QRectF r;
    QLinearGradient lg;

    if (m_orientation == Qt::Horizontal) {
        lg = QLinearGradient(0, 0, width(), 0);
        r = QRectF(ArrowHalfWidth - 1, 0, width() - 2 * ArrowHalfWidth + 1, height() - ArrowLength);
    } else {
        lg = QLinearGradient(0, 0, 0, height());
        r = QRectF(0, ArrowHalfWidth - 1, width() - ArrowLength, height() - 2 * ArrowHalfWidth + 1);
    }

    lg.setStops(m_gradient.stops());
    painter.setBrush(lg);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(r);
    // END draw gradient

    // BEGIN draw arrows
    painter.setRenderHint(QPainter::Antialiasing, true);
    const QGradientStops stops = m_gradient.stops();
    for (const QGradientStop &stop : stops)
        drawArrow(&painter, stop);
    // END draw arrows
}

void KGradientEditor::drawArrow(QPainter *painter, const QGradientStop &stop)
{
    QPolygonF arrow(3);

    double mid = toArrowPos(stop.first);

    if (m_orientation == Qt::Horizontal) {
        arrow[0] = QPointF(mid, height() - ArrowLength + 0.5);
        arrow[1] = QPointF(mid + ArrowHalfWidth, height() - 0.5);
        arrow[2] = QPointF(mid - ArrowHalfWidth, height() - 0.5);
    } else {
        arrow[0] = QPointF(width() - ArrowLength + 0.5, mid);
        arrow[1] = QPointF(width() - 0.5, mid + ArrowHalfWidth);
        arrow[2] = QPointF(width() - 0.5, mid - ArrowHalfWidth);
    }

    bool selected = (stop == m_currentStop);
    QColor color(selected ? palette().color(QPalette::Dark) : Qt::black);

    painter->setPen(color);
    painter->setBrush(stop.second);
    painter->drawPolygon(arrow);
}

void KGradientEditor::contextMenuEvent(QContextMenuEvent *e)
{
    // Prevent the "QWhatsThis" menu from popping up when right-clicking
    e->accept();
}

void KGradientEditor::removeStop()
{
    QGradientStops stops = m_gradient.stops();
    for (int i = 0; i < stops.size(); ++i) {
        if (stops[i] != m_currentStop)
            continue;

        stops.remove(i);
        break;
    }

    setGradient(stops);
    findGradientStop();
}

void KGradientEditor::mousePressEvent(QMouseEvent *e)
{
    if (!getGradientStop(e->pos()))
        return;
    e->accept();

    if (e->button() == Qt::RightButton)
        removeStop();
    else
        m_haveArrow = true;
}

bool KGradientEditor::getGradientStop(const QPoint &point)
{
    double dl; // the vertical (for horizontal layout) distance from the tip of the arrows
    if (m_orientation == Qt::Horizontal)
        dl = point.y() - (height() - ArrowLength);
    else
        dl = point.x() - (width() - ArrowLength);

    // Is the arrow in the strip?
    if (dl < 0)
        return false;

    QGradientStops stops = m_gradient.stops();

    // Iterate over stops in reverse as the last stops are displayed on top of
    // the first stops.
    for (int i = stops.size() - 1; i >= 0; --i) {
        QGradientStop stop = stops[i];

        double pos = toArrowPos(stop.first);

        // Is the click inside the arrow?
        double lower = pos - dl * (ArrowHalfWidth / ArrowLength);
        double upper = pos + dl * (ArrowHalfWidth / ArrowLength);

        double x = (m_orientation == Qt::Horizontal) ? point.x() : point.y();
        if (x < lower || x > upper)
            continue;

        // Is inside arrow! :)
        m_clickOffset = x - pos;

        setCurrentStop(stop);
        return true;
    }

    return false;
}

void KGradientEditor::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_haveArrow)
        return;

    e->accept();
    QPoint point = e->pos();

    // Hmm...why doesn't qvector have some sortof search / replace functionality?
    QGradientStops stops = m_gradient.stops();
    for (int i = 0; i < stops.size(); ++i) {
        if (stops[i] != m_currentStop)
            continue;

        double x = (m_orientation == Qt::Horizontal) ? point.x() : point.y();

        m_currentStop.first = fromArrowPos(x - m_clickOffset);

        stops[i] = m_currentStop;
        break;
    }

    setGradient(stops);
}

void KGradientEditor::mouseReleaseEvent(QMouseEvent *)
{
    m_haveArrow = false;
}

void KGradientEditor::mouseDoubleClickEvent(QMouseEvent *e)
{
    e->accept();

    if (getGradientStop(e->pos()))
        return;

    // Create new stop
    QPoint point = e->pos();
    double pos = fromArrowPos((m_orientation == Qt::Horizontal) ? point.x() : point.y());

    QGradientStop stop;
    stop.first = pos;
    stop.second = Qt::red;

    QGradientStops stops = m_gradient.stops();
    stops << stop;

    setGradient(stops);
    setCurrentStop(stop);
}

void KGradientEditor::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    update();
}

void KGradientEditor::findGradientStop()
{
    QGradientStops stops = m_gradient.stops();

    // The QGradientStops should always have at least one stop in, since
    // QGradient returns a Black->White gradient if its stops are empty.
    Q_ASSERT(!stops.isEmpty());

    // Pick a stop in the center
    setCurrentStop(stops[stops.size() / 2]);
}

void KGradientEditor::setCurrentStop(const QGradientStop &stop)
{
    if (m_currentStop == stop)
        return;

    bool colorChanged = stop.second != m_currentStop.second;

    m_currentStop = stop;
    update();

    if (colorChanged)
        emit colorSelected(stop.second);
}

void KGradientEditor::setGradient(const QGradientStops &stops)
{
    if (stops == m_gradient.stops())
        return;

    m_gradient.setStops(stops);
    update();
    emit gradientChanged(m_gradient);
}

double KGradientEditor::toArrowPos(double stop) const
{
    double l = (m_orientation == Qt::Horizontal) ? width() : height();
    l -= 2 * ArrowHalfWidth;
    return stop * l + ArrowHalfWidth;
}

double KGradientEditor::fromArrowPos(double pos) const
{
    double l = (m_orientation == Qt::Horizontal) ? width() : height();
    l -= 2 * ArrowHalfWidth;

    double stop = (pos - ArrowHalfWidth) / l;

    if (stop < 0)
        stop = 0;
    else if (stop > 1)
        stop = 1;

    return stop;
}
// END class KGradientEditor

// BEGIN class KGradientDialog
KGradientDialog::KGradientDialog(QWidget *parent, bool modal)
    : QDialog(parent)
{
    QWidget *widget = new QWidget(this);
    m_gradient = new KGradientEditor(widget);
    m_colorDialog = new QColorDialog(widget);
    m_colorDialog->setWindowFlags(Qt::Widget);
    m_colorDialog->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::NoButtons);

    QLabel *label = new QLabel(i18n("(Double-click on the gradient to add a stop)"), widget);
    QPushButton *button = new QPushButton(i18n("Remove stop"), widget);
    connect(button, &QPushButton::clicked, m_gradient, &KGradientEditor::removeStop);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(modal ? QDialogButtonBox::Ok | QDialogButtonBox::Cancel : QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &KGradientDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KGradientDialog::reject);

    // BEGIN layout widgets
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_gradient->setFixedHeight(24);
    layout->addWidget(m_gradient);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(label);
    hLayout->addStretch(1);
    hLayout->addWidget(button);
    layout->addLayout(hLayout);
    layout->addWidget(m_colorDialog);
    layout->addWidget(buttonBox);
    resize(layout->minimumSize());
    // END layout widgets

    setWindowTitle(i18nc("@title:window", "Choose a Gradient"));

    setModal(modal);

    connect(m_gradient, &KGradientEditor::colorSelected, m_colorDialog, &QColorDialog::setCurrentColor);
    connect(m_colorDialog, &QColorDialog::currentColorChanged, m_gradient, &KGradientEditor::setColor);
    connect(m_gradient, &KGradientEditor::gradientChanged, this, &KGradientDialog::gradientChanged);

    m_colorDialog->setCurrentColor(m_gradient->color());
}

KGradientDialog::~KGradientDialog()
{
}

// static
int KGradientDialog::getGradient(QGradient &gradient, QWidget *parent)
{
    QPointer<KGradientDialog> dlg = new KGradientDialog(parent, true);
    dlg->setGradient(gradient);

    int result = dlg->exec();
    if (result == Accepted)
        gradient = dlg->gradient();
    delete dlg;
    return result;
}

void KGradientDialog::setGradient(const QGradient &gradient)
{
    m_gradient->setGradient(gradient);
}

QGradient KGradientDialog::gradient() const
{
    return m_gradient->gradient();
}
// END class KGradientDialog

// BEGIN class KGradientButton
KGradientButton::KGradientButton(QWidget *parent)
    : QPushButton(parent)
{
    connect(this, &KGradientButton::clicked, this, &KGradientButton::chooseGradient);
}

KGradientButton::~KGradientButton()
{
}

void KGradientButton::initStyleOption(QStyleOptionButton *opt) const
{
    opt->initFrom(this);
    opt->text.clear();
    opt->icon = QIcon();
    opt->features = QStyleOptionButton::None;
}

QSize KGradientButton::sizeHint() const
{
    QStyleOptionButton opt;
    initStyleOption(&opt);
    return style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(40, 15), this).expandedTo(QApplication::globalStrut());
}

void KGradientButton::setGradient(const QGradient &gradient)
{
    if (m_gradient.stops() == gradient.stops())
        return;

    m_gradient.setStops(gradient.stops());
    emit gradientChanged(m_gradient);
}

void KGradientButton::chooseGradient()
{
    int result = KGradientDialog::getGradient(m_gradient, this);
    if (result == KGradientDialog::Accepted)
        emit gradientChanged(m_gradient);
}

void KGradientButton::paintEvent(QPaintEvent *)
{
    // Mostly copied verbatim from KColorButton - thanks! :)

    QPainter painter(this);

    // First, we need to draw the bevel.
    QStyleOptionButton butOpt;
    initStyleOption(&butOpt);
    style()->drawControl(QStyle::CE_PushButtonBevel, &butOpt, &painter, this);

    // OK, now we can muck around with drawing out pretty little color box
    // First, sort out where it goes
    QRect labelRect = style()->subElementRect(QStyle::SE_PushButtonContents, &butOpt, this);
    int shift = style()->pixelMetric(QStyle::PM_ButtonMargin);
    labelRect.adjust(shift, shift, -shift, -shift);
    int x, y, w, h;
    labelRect.getRect(&x, &y, &w, &h);

    if (isChecked() || isDown()) {
        x += style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal);
        y += style()->pixelMetric(QStyle::PM_ButtonShiftVertical);
    }

    qDrawShadePanel(&painter, x, y, w, h, palette(), true, 1, NULL);

    if (isEnabled()) {
        QLinearGradient lg(x + 1, 0, x + w - 1, 0);
        lg.setStops(m_gradient.stops());
        painter.setBrush(lg);
    } else
        painter.setBrush(palette().color(backgroundRole()));

    painter.drawRect(x + 1, y + 1, w - 2, h - 2);

    if (hasFocus()) {
        QRect focusRect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &butOpt, this);
        QStyleOptionFocusRect focusOpt;
        focusOpt.initFrom(this);
        focusOpt.rect = focusRect;
        focusOpt.backgroundColor = palette().window().color();
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOpt, &painter, this);
    }
}
// END class KGradientButton
