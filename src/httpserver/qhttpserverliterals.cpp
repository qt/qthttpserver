// Copyright (C) 2019 Mikhail Svetkin <mikhail.svetkin@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qhttpserverliterals_p.h"

QT_BEGIN_NAMESPACE

QByteArray QHttpServerLiterals::contentTypeHeader()
{
    return QByteArrayLiteral("Content-Type");
}

QByteArray QHttpServerLiterals::contentTypeXEmpty()
{
    return QByteArrayLiteral("application/x-empty");
}

QByteArray QHttpServerLiterals::contentTypeTextHtml()
{
    return QByteArrayLiteral("text/html");
}

QByteArray QHttpServerLiterals::contentTypeJson()
{
    return QByteArrayLiteral("application/json");
}

QByteArray QHttpServerLiterals::contentLengthHeader()
{
    return QByteArrayLiteral("Content-Length");
}

QT_END_NAMESPACE
