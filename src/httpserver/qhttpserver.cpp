// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtHttpServer/qhttpserver.h>


#include <QtHttpServer/qhttpserverrequest.h>
#include <QtHttpServer/qhttpserverresponder.h>
#include <QtHttpServer/qhttpserverresponse.h>

#include <private/qhttpserver_p.h>

#include <QtCore/qloggingcategory.h>

#include <QtNetwork/qtcpsocket.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcHS, "qt.httpserver");

/*!
    \class QHttpServer
    \inmodule QtHttpServer
    \brief QHttpServer is a simplified API for QAbstractHttpServer and QHttpServerRouter.

    \code

    QHttpServer server;

    server.route("/", [] () {
        return "hello world";
    });
    server.listen();

    \endcode
*/

/*!
    Creates an instance of QHttpServer with parent \a parent.
*/
QHttpServer::QHttpServer(QObject *parent)
    : QAbstractHttpServer(*new QHttpServerPrivate, parent)
{
    connect(this, &QAbstractHttpServer::missingHandler, this,
            [=] (const QHttpServerRequest &request, QTcpSocket *socket) {
        qCDebug(lcHS) << tr("missing handler:") << request.url().path();
        sendResponse(QHttpServerResponder::StatusCode::NotFound, request, socket);
    });
}

/*! \fn template<typename Rule = QHttpServerRouterRule, typename ... Args> bool QHttpServer::route(Args && ... args)

    This function is just a wrapper to simplify the router API.

    This function takes variadic arguments \a args. The last argument is a
    callback (\c{ViewHandler}). The remaining arguments are used to create a
    new \c Rule (the default is QHttpServerRouterRule). This is in turn added
    to the QHttpServerRouter. It returns \c true if a new rule is created,
    otherwise it returns \c false.

    \c ViewHandler can only be a lambda. The lambda definition can take two
    optional special arguments: \c {const QHttpServerRequest&} and
    \c {QHttpServerResponder&&}. These special arguments must be the last in
    the parameter list, but in any order, and there can be none, one, or both
    of them present.

    Examples:

    \code

    QHttpServer server;

    // Valid:
    server.route("test", [] (const int page) { return ""; });
    server.route("test", [] (const int page, const QHttpServerRequest &request) { return ""; });
    server.route("test", [] (QHttpServerResponder &&responder) { return ""; });

    // Invalid (compile time error):
    server.route("test", [] (const QHttpServerRequest &request, const int page) { return ""; }); // request must be last
    server.route("test", [] (QHttpServerRequest &request) { return ""; });      // request must be passed by const reference
    server.route("test", [] (QHttpServerResponder &responder) { return ""; });  // responder must be passed by universal reference

    \endcode

    \sa QHttpServerRouter::addRule
*/

/*! \fn template<typename ViewHandler> void QHttpServer::afterRequest(ViewHandler &&viewHandler)
    Register a function to be run after each request.

    The \a viewHandler argument can only be a lambda. The lambda definition
    can take one or two optional arguments: \c {QHttpServerResponse &&} and
    \c {const QHttpServerRequest &}. If both are given, they can be in either
    order.

    Examples:

    \code

    QHttpServer server;

    // Valid:
    server.afterRequest([] (QHttpServerResponse &&resp, const QHttpServerRequest &request) {
        return std::move(resp);
    }
    server.afterRequest([] (const QHttpServerRequest &request, QHttpServerResponse &&resp) {
        return std::move(resp);
    }
    server.afterRequest([] (QHttpServerResponse &&resp) { return std::move(resp); }

    // Invalid (compile time error):
    // resp must be passed by universal reference
    server.afterRequest([] (QHttpServerResponse &resp, const QHttpServerRequest &request) {
        return std::move(resp);
    }
    // request must be passed by const reference
    server.afterRequest([] (QHttpServerResponse &&resp, QHttpServerRequest &request) {
        return std::move(resp);
    }

    \endcode
*/

/*!
    Destroys a QHttpServer.
*/
QHttpServer::~QHttpServer()
{
}

/*!
    Returns the router object.
*/
QHttpServerRouter *QHttpServer::router()
{
    Q_D(QHttpServer);
    return &d->router;
}

/*!
    \internal
*/
void QHttpServer::afterRequestImpl(AfterRequestHandler &&afterRequestHandler)
{
    Q_D(QHttpServer);
    d->afterRequestHandlers.push_back(std::move(afterRequestHandler));
}

/*!
    \internal
*/
void QHttpServer::sendResponse(QHttpServerResponse &&response,
                               const QHttpServerRequest &request,
                               QTcpSocket *socket)
{
    Q_D(QHttpServer);
    for (auto afterRequestHandler : d->afterRequestHandlers)
        response = afterRequestHandler(std::move(response), request);
    response.write(makeResponder(request, socket));
}

/*!
    \internal
*/
bool QHttpServer::handleRequest(const QHttpServerRequest &request, QTcpSocket *socket)
{
    Q_D(QHttpServer);
    return d->router.handleRequest(request, socket);
}

QT_END_NAMESPACE

#include "moc_qhttpserver.cpp"
