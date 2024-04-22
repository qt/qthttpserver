// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QABSTRACTHTTPSERVER_H
#define QABSTRACTHTTPSERVER_H

#include <QtCore/qobject.h>

#include <QtHttpServer/qthttpserverglobal.h>
#include <QtHttpServer/qhttpserverwebsocketupgraderesponse.h>

#include <QtNetwork/qhostaddress.h>

#if QT_CONFIG(ssl)
#include <QtNetwork/qssl.h>
#include <QtNetwork/qhttp2configuration.h>
#endif

#if defined(QT_WEBSOCKETS_LIB)
#include <QtWebSockets/qwebsocket.h>
#endif // defined(QT_WEBSOCKETS_LIB)

#if QT_CONFIG(localserver)
#include <QtNetwork/qlocalserver.h>
#endif

#include <functional>
#include <memory>

QT_BEGIN_NAMESPACE

class QHttpServerRequest;
class QHttpServerResponder;
class QSslCertificate;
class QSslConfiguration;
class QSslKey;
class QTcpServer;

class QAbstractHttpServerPrivate;
class Q_HTTPSERVER_EXPORT QAbstractHttpServer : public QObject
{
    Q_OBJECT
    friend class QHttpServerHttp1ProtocolHandler;

public:
    explicit QAbstractHttpServer(QObject *parent = nullptr);
    ~QAbstractHttpServer() override;

    quint16 listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    QList<quint16> serverPorts();

    void bind(QTcpServer *server = nullptr);
    QList<QTcpServer *> servers() const;

#if QT_CONFIG(localserver)
    void bind(QLocalServer *server);
    QList<QLocalServer *> localServers() const;
#endif

#if QT_CONFIG(ssl)
    void sslSetup(const QSslCertificate &certificate, const QSslKey &privateKey,
                  QSsl::SslProtocol protocol = QSsl::SecureProtocols);
    void sslSetup(const QSslConfiguration &sslConfiguration);

    QHttp2Configuration http2Configuration() const;
    void setHttp2Configuration(const QHttp2Configuration &configuration);
#endif

#if defined(QT_WEBSOCKETS_LIB)
Q_SIGNALS:
    void newWebSocketConnection();

private:
    using WebSocketUpgradeVerifierPrototype =
            QHttpServerWebSocketUpgradeResponse (*)(const QHttpServerRequest &request);
    template <typename T>
    using if_compatible_callable = typename std::enable_if<
            QtPrivate::AreFunctionsCompatible<WebSocketUpgradeVerifierPrototype, T>::value,
            bool>::type;

    void registerWebSocketUpgradeVerifierImpl(QtPrivate::QSlotObjectBase *slotObjRaw);

public:
    bool hasPendingWebSocketConnections() const;
    std::unique_ptr<QWebSocket> nextPendingWebSocketConnection();

    template <typename Functor, if_compatible_callable<Functor> = true>
    void registerWebSocketUpgradeVerifier(Functor &&func)
    {
        registerWebSocketUpgradeVerifierImpl(
                QtPrivate::makeCallableObject<WebSocketUpgradeVerifierPrototype>(
                        std::forward<Functor>(func)));
    }

private:
    QHttpServerWebSocketUpgradeResponse
    verifyWebSocketUpgrade(const QHttpServerRequest &request) const;
#endif // defined(QT_WEBSOCKETS_LIB)

protected:
    QAbstractHttpServer(QAbstractHttpServerPrivate &dd, QObject *parent = nullptr);

    virtual bool handleRequest(const QHttpServerRequest &request,
                               QHttpServerResponder &responder) = 0;
    virtual void missingHandler(const QHttpServerRequest &request,
                                QHttpServerResponder &&responder) = 0;

private:
    Q_DECLARE_PRIVATE(QAbstractHttpServer)
};

QT_END_NAMESPACE

#endif // QABSTRACTHTTPSERVER_H
