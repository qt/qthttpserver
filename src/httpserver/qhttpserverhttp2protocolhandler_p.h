// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QHttpServerHttp2ProtocolHandler_H
#define QHttpServerHttp2ProtocolHandler_H

#include <QtHttpServer/qthttpserverglobal.h>
#include <private/qhttpserverstream_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QHttpServer. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

QT_REQUIRE_CONFIG(http);

QT_BEGIN_NAMESPACE

class QTcpSocket;
class QAbstractHttpServer;
class QHttp2Connection;

class QHttpServerHttp2ProtocolHandler : public QHttpServerStream
{
    Q_OBJECT

    friend class QAbstractHttpServerPrivate;

private:
    QHttpServerHttp2ProtocolHandler(QAbstractHttpServer *server, QIODevice *socket);

    void responderDestroyed() final;
    void startHandlingRequest() final;
    void socketDisconnected() final;

    void write(const QByteArray &body, const QHttpHeaders &headers,
               QHttpServerResponder::StatusCode status) final;
    void write(QHttpServerResponder::StatusCode status) final;
    void write(QIODevice *data, const QHttpHeaders &headers,
               QHttpServerResponder::StatusCode status) final;
    void writeBeginChunked(const QHttpHeaders &headers,
                           QHttpServerResponder::StatusCode status) final;
    void writeChunk(const QByteArray &body) final;
    void writeEndChunked(const QByteArray &data, const QHttpHeaders &trailers) final;

    QAbstractHttpServer *server;
    QIODevice *socket;
    QTcpSocket *tcpSocket;
    QHttp2Connection *m_connection = nullptr;
};

QT_END_NAMESPACE

#endif // QHttpServerHttp2ProtocolHandler_H
