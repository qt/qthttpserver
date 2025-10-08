// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qhttpserverstream_p.h"
#include <private/qhttpserverresponder_p.h>

#include <QtNetwork/qtcpsocket.h>

#if QT_CONFIG(ssl)
#include <QtNetwork/qsslsocket.h>
#endif
#if QT_CONFIG(localserver)
#include <QtNetwork/qlocalsocket.h>
#endif

QT_BEGIN_NAMESPACE

static QHttpServerParser initParserFromSocket(QIODevice *socket)
{
    QTcpSocket *tcpSocket = qobject_cast<QTcpSocket *>(socket);
    if (tcpSocket) {
        return QHttpServerParser(tcpSocket->peerAddress(), tcpSocket->peerPort(),
                                 tcpSocket->localAddress(), tcpSocket->localPort());
    }
    return QHttpServerParser(QHostAddress::LocalHost, 0, QHostAddress::LocalHost, 0);
}

#if QT_CONFIG(ssl)
static QSslConfiguration initSslConfigurationFromSocket(QIODevice *socket)
{
    if (auto *ssl = qobject_cast<const QSslSocket *>(socket))
        return ssl->sslConfiguration();

    return QSslConfiguration();
}
#endif

QHttpServerStream::QHttpServerStream(QIODevice *socket, QObject *parent)
    : QObject(parent)
    , parser(initParserFromSocket(socket))
#if QT_CONFIG(ssl)
    , sslConfiguration(initSslConfigurationFromSocket(socket))
#endif
    , clientSocket(socket)
{
}

void QHttpServerStream::connectResponder(QHttpServerResponderPrivate *responder)
{
    if (QTcpSocket *tcpSocket = qobject_cast<QTcpSocket *>(clientSocket)) {
        responderConnections.insert(responder->m_streamId,
                                    connect(tcpSocket, &QAbstractSocket::disconnected, tcpSocket,
                                            [responder]() { responder->cancel(); }));
#if QT_CONFIG(localserver)
    } else if (QLocalSocket *localSocket = qobject_cast<QLocalSocket *>(clientSocket)) {
        responderConnections.insert(responder->m_streamId,
                                    connect(localSocket, &QLocalSocket::disconnected, localSocket,
                                            [responder]() { responder->cancel(); }));
#endif
    }
}

void QHttpServerStream::disconnectResponder(quint32 streamId)
{
    auto connection = responderConnections.take(streamId);
    if (connection)
        disconnect(connection);
}

QT_END_NAMESPACE
