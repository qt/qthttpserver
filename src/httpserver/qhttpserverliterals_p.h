// Copyright (C) 2019 Mikhail Svetkin <mikhail.svetkin@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QHTTPSERVERLITERALS_P_H
#define QHTTPSERVERLITERALS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QHttpServer. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include <QtHttpServer/qthttpserverglobal.h>

#include <QtCore/qstringfwd.h>

QT_BEGIN_NAMESPACE

namespace QHttpServerLiterals {

Q_HTTPSERVER_EXPORT QByteArray contentTypeHeader();
Q_HTTPSERVER_EXPORT QByteArray contentTypeXEmpty();
Q_HTTPSERVER_EXPORT QByteArray contentTypeTextHtml();
Q_HTTPSERVER_EXPORT QByteArray contentTypeJson();
Q_HTTPSERVER_EXPORT QByteArray contentLengthHeader();

}

QT_END_NAMESPACE

#endif // QHTTPSERVERLITERALS_P_H
