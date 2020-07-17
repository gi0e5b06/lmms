/*
 * base64.h - namespace base64 with methods for encoding/decoding binary data
 *            to/from base64
 *
 * Copyright (c) 2006-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef _BASE64_H
#define _BASE64_H

#include "lmms_basics.h"

#include <QByteArray>
#include <QString>
#include <QVariant>

namespace base64
{

/*
inline void encode(const char* _data, const int _size, QString& _dst)
{
_dst = QByteArray(_data, _size).toBase64();
}

template <class T>
inline void decode(const QString& _b64, T** _data, int* _size)
{
QByteArray data = QByteArray::fromBase64(_b64.toUtf8());
*_size          = data.size();
*_data          = new T[*_size / sizeof(T)];
memcpy(*_data, data.constData(), *_size);
}
*/

QString encodeChars(const char* _data, const int _size);
QString encodeInt32s(const int32_t* _data, const int _size);
QString encodeFloats(const FLOAT* _data, const int _size);
QString encodeDoublesAsFloats(const double* _data, const int _size);
QString encodeReals(const real_t* _data, const int _size);

int decodeChars(const QString& _b64, char** _data);
int decodeInt32s(const QString& _b64, int32_t** _data);
int decodeFloats(const QString& _b64, FLOAT** _data);
int decodeFloatsAsDoubles(const QString& _b64, double** _data);
int decodeReals(const QString& _b64, real_t** _data);

// for compatibility-code only
QVariant decodeVariant(const QString& _b64,
                       QVariant::Type _force_type = QVariant::Invalid);
}  // namespace base64

#endif
