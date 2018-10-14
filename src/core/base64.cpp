/*
 * base64.cpp - namespace base64 with methods for encoding/decoding binary
 * data to/from base64
 *
 * Copyright (c) 2006-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#include "base64.h"

#include <QBuffer>
#include <QDataStream>

namespace base64
{

QVariant decodeVariant(const QString& _b64, QVariant::Type _force_type)
{
    QByteArray ba = QByteArray::fromBase64(_b64.toUtf8());
    QBuffer    buf(&ba);
    buf.open(QBuffer::ReadOnly);
    QDataStream in(&buf);
    QVariant    ret;
    in >> ret;
    if(_force_type != QVariant::Invalid && ret.type() != _force_type)
    {
        buf.reset();
        in.setVersion(QDataStream::Qt_3_3);
        in >> ret;
    }
    return ret;
}

QString encodeChars(const char* _data, const int _size)
{
    return QByteArray(_data, _size).toBase64();
}

QString encodeFloats(const FLOAT* _data, const int _size)
{
    return encodeChars((const char*)_data, _size * sizeof(FLOAT));
}

QString encodeDoublesAsFloats(const double* _data, const int _size)
{
    FLOAT* buf = new FLOAT[_size];
    for(int i = 0; i < _size; i++)
        buf[i] = _data[i];
    QString r = encodeFloats(buf, _size);
    delete[] buf;
    return r;
}

int decodeChars(const QString& _b64, char** _data)
{
    QByteArray data = QByteArray::fromBase64(_b64.toUtf8());
    int        size = data.size();
    *_data          = new char[size];
    memcpy(*_data, data.constData(), size);
    return size;
}

int decodeFloats(const QString& _b64, FLOAT** _data)
{
    int size = decodeChars(_b64, (char**)_data);
    return size / sizeof(FLOAT);
}

int decodeFloatsAsDoubles(const QString& _b64, double** _data)
{
    FLOAT* buf  = nullptr;
    int    size = decodeFloats(_b64, &buf);
    *_data      = new double[size];
    for(int i = 0; i < size; i++)
        (*_data)[i] = buf[i];
    delete[] buf;
    return size;
}

};  // namespace base64
