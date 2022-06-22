// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QABSTRACTHTTPSERVER_H
#define QABSTRACTHTTPSERVER_H

#include <QtCore/qobject.h>

#include <QtHttpServer/qthttpserverglobal.h>

#include <QtNetwork/qhostaddress.h>

#if QT_CONFIG(ssl)
#include <QtNetwork/qsslserver.h>
#include <QtNetwork/qsslcertificate.h>
#include <QtNetwork/qsslkey.h>
#endif

QT_BEGIN_NAMESPACE

class QHttpServerRequest;
class QHttpServerResponder;
class QTcpServer;
class QTcpSocket;
class QWebSocket;

class QAbstractHttpServerPrivate;
class Q_HTTPSERVER_EXPORT QAbstractHttpServer : public QObject
{
    Q_OBJECT

public:
    QAbstractHttpServer(QObject *parent = nullptr);

    quint16 listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    QList<quint16> serverPorts();

    void bind(QTcpServer *server = nullptr);
    QList<QTcpServer *> servers() const;

#if QT_CONFIG(ssl)
    void sslSetup(const QSslCertificate &certificate, const QSslKey &privateKey,
                  QSsl::SslProtocol protocol = QSsl::SecureProtocols);
    void sslSetup(const QSslConfiguration &sslConfiguration);
#endif

Q_SIGNALS:
    void missingHandler(const QHttpServerRequest &request, QTcpSocket *socket);

#if defined(QT_WEBSOCKETS_LIB)
    void newWebSocketConnection();

public:
    bool hasPendingWebSocketConnections() const;
    QWebSocket *nextPendingWebSocketConnection();
#endif // defined(QT_WEBSOCKETS_LIB)

protected:
    QAbstractHttpServer(QAbstractHttpServerPrivate &dd, QObject *parent = nullptr);

    virtual bool handleRequest(const QHttpServerRequest &request, QTcpSocket *socket) = 0;
    static QHttpServerResponder makeResponder(const QHttpServerRequest &request,
                                              QTcpSocket *socket);

private:
    Q_DECLARE_PRIVATE(QAbstractHttpServer)
};

QT_END_NAMESPACE

#endif // QABSTRACTHTTPSERVER_H
