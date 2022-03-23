/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtHttpServer module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QHTTPSERVERREQUEST_P_H
#define QHTTPSERVERREQUEST_P_H

#include <QtHttpServer/qhttpserverrequest.h>
#include <QtNetwork/private/qhttpheaderparser_p.h>
#include <QtCore/private/qbytedata_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QHttpServer. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

QT_BEGIN_NAMESPACE

class QHttpServerRequestPrivate : public QSharedData
{
public:
    QHttpServerRequestPrivate(const QHostAddress &remoteAddress);

    quint16 port = 0;

    enum class State {
        NothingDone,
        ReadingRequestLine,
        ReadingHeader,
        ReadingData,
        AllDone,
    } state = State::NothingDone;

    QUrl url;
    QHttpServerRequest::Method method;
    QHttpHeaderParser parser;

    QByteArray header(const QByteArray &key) const;

    bool parseRequestLine(QByteArrayView line);
    qsizetype readRequestLine(QAbstractSocket *socket);
    qsizetype readHeader(QAbstractSocket *socket);
    qsizetype readBodyFast(QAbstractSocket *socket);
    qsizetype readRequestBodyRaw(QAbstractSocket *socket, qsizetype size);
    qsizetype readRequestBodyChunked(QAbstractSocket *socket);
    qsizetype getChunkSize(QAbstractSocket *socket, qsizetype *chunkSize);

    bool parse(QAbstractSocket *socket);

    void clear();

    qint64 contentLength() const;
    QByteArray headerField(const QByteArray &name,
                           const QByteArray &defaultValue = QByteArray()) const;
    QList<QByteArray> headerFieldValues(const QByteArray &name) const;

    QHostAddress remoteAddress;
    bool handling{false};
    qsizetype bodyLength;
    qsizetype contentRead;
    bool chunkedTransferEncoding;
    bool lastChunkRead;
    qsizetype currentChunkRead;
    qsizetype currentChunkSize;
    bool upgrade;

    QByteArray fragment;
    QByteDataBuffer bodyBuffer;
    QByteArray body;
};

QT_END_NAMESPACE

#endif // QHTTPSERVERREQUEST_P_H
