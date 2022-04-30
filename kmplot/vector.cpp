/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2006 David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "vector.h"
#include "function.h"

#include <assert.h>
#include <string.h>

// BEGIN class Vector
Vector Vector::operator+(const Vector &other) const
{
    Vector v(*this);
    v += other;
    return v;
}

Vector &Vector::operator+=(const Vector &other)
{
    assert(size() == other.size());
    for (int i = 0; i < size(); ++i)
        (*this)[i] += other[i];

    return *this;
}

Vector Vector::operator-(const Vector &other) const
{
    Vector v(*this);
    v -= other;
    return v;
}

Vector &Vector::operator-=(const Vector &other)
{
    assert(size() == other.size());
    for (int i = 0; i < size(); ++i)
        (*this)[i] -= other[i];

    return *this;
}

Vector Vector::operator*(double x) const
{
    Vector v(*this);
    v *= x;
    return v;
}

Vector &Vector::operator*=(double x)
{
    for (int i = 0; i < size(); ++i)
        (*this)[i] *= x;
    return *this;
}

Vector &Vector::operator=(const Vector &other)
{
    // I don't much like Qt's copy-on-write behaviour.
    // At least in KmPlot, it makes a small number of cases marginally faster,
    // and a lot of cases a lot slower. This is because allocating and freeing
    // memory is a lot slower than copying memory.
    //
    // In fact, it can make drawing differential equations several times slower.

    resize(other.size());
    memcpy(m_data.data(), other.m_data.data(), size() * sizeof(double));

    return *this;
}

Vector &Vector::operator=(const QVector<Value> &other)
{
    int size = other.size();
    resize(size);
    for (int i = 0; i < size; ++i)
        (*this)[i] = other[i].value();
    return *this;
}

void Vector::combine(const Vector &a, double k, const Vector &b)
{
    assert(a.size() == b.size());
    int n = a.size();
    resize(n);

    double *d1 = m_data.data();
    const double *d2 = a.m_data.data();
    const double *d3 = b.m_data.data();

    for (int i = 0; i < n; ++i)
        d1[i] = d2[i] + k * d3[i];
}

void Vector::addRK4(double dx, const Vector &k1, const Vector &k2, const Vector &k3, const Vector &k4)
{
    double *d = m_data.data();
    const double *d1 = k1.m_data.data();
    const double *d2 = k2.m_data.data();
    const double *d3 = k3.m_data.data();
    const double *d4 = k4.m_data.data();

    int n = size();

    for (int i = 0; i < n; ++i)
        d[i] += (dx / 6) * (d1[i] + 2 * d2[i] + 2 * d3[i] + d4[i]);
}
// END class Vector
