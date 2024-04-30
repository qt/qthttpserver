// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qhttpserverhttp2protocolhandler_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtHttpServer/qabstracthttpserver.h>
#include <QtNetwork/private/qhttp2connection_p.h>
#include <QtNetwork/qtcpsocket.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcHttpServerHttp2Handler, "qt.httpserver.http2handler")

QHttpServerHttp2ProtocolHandler::QHttpServerHttp2ProtocolHandler(QAbstractHttpServer *server,
                                                                 QIODevice *socket)
    : QHttpServerStream(server),
      server(server),
      socket(socket),
      tcpSocket(qobject_cast<QTcpSocket *>(socket))
{
    socket->setParent(this);

    m_connection = QHttp2Connection::createDirectServerConnection(socket, {});
    if (!m_connection)
        return;

    Q_ASSERT(tcpSocket);

    QObject::connect(tcpSocket,
                     &QTcpSocket::readyRead,
                     m_connection,
                     &QHttp2Connection::handleReadyRead);
}

void QHttpServerHttp2ProtocolHandler::responderDestroyed()
{

}

void QHttpServerHttp2ProtocolHandler::startHandlingRequest()
{

}

void QHttpServerHttp2ProtocolHandler::socketDisconnected()
{
    deleteLater();
}

void QHttpServerHttp2ProtocolHandler::write(const QByteArray &, const QHttpHeaders &,
                                  QHttpServerResponder::StatusCode)
{

}

void QHttpServerHttp2ProtocolHandler::write(QHttpServerResponder::StatusCode)
{

}

void QHttpServerHttp2ProtocolHandler::write(QIODevice *, const QHttpHeaders &,
                                  QHttpServerResponder::StatusCode)
{

}

void QHttpServerHttp2ProtocolHandler::writeBeginChunked(const QHttpHeaders &,
                                              QHttpServerResponder::StatusCode)
{

}

void QHttpServerHttp2ProtocolHandler::writeChunk(const QByteArray &)
{

}

void QHttpServerHttp2ProtocolHandler::writeEndChunked(const QByteArray &, const QHttpHeaders &)
{

}

QT_END_NAMESPACE
