// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qhttpserverrequest_p.h"

#include <QtHttpServer/qhttpserverrequest.h>
#include <QtNetwork/qhttpheaders.h>

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>
#include <QtNetwork/qtcpsocket.h>
#if QT_CONFIG(ssl)
#include <QtNetwork/qsslsocket.h>
#endif
#if QT_CONFIG(http)
#include <QtNetwork/private/qhttp2connection_p.h>
#endif

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

#if !defined(QT_NO_DEBUG_STREAM)

/*!
    \fn QDebug QHttpServerRequest::operator<<(QDebug debug, const QHttpServerRequest &request)

    Writes information about \a request to the \a debug stream.

    \sa QDebug
 */
Q_HTTPSERVER_EXPORT QDebug operator<<(QDebug debug, const QHttpServerRequest &request)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "QHttpServerRequest(";
    debug << "(Url: " << request.url() << ")";
    debug << "(Headers: " << request.headers() << ")";
    debug << "(RemoteHost: " << request.remoteAddress() << ")";
    debug << "(BodySize: " << request.body().size() << ")";
    debug << ')';
    return debug;
}

#endif

/*!
    \internal
*/
QHttpServerRequestPrivate::QHttpServerRequestPrivate(const QHostAddress &remoteAddress,
                                                     quint16 remotePort,
                                                     const QHostAddress &localAddress,
                                                     quint16 localPort)
    : remoteAddress(remoteAddress),
      remotePort(remotePort),
      localAddress(localAddress),
      localPort(localPort)
{
}

#if QT_CONFIG(ssl)
/*!
    \internal
*/
QHttpServerRequestPrivate::QHttpServerRequestPrivate(const QHostAddress &remoteAddress,
                                                     quint16 remotePort,
                                                     const QHostAddress &localAddress,
                                                     quint16 localPort,
                                                     const QSslConfiguration &sslConfiguration)
    : remoteAddress(remoteAddress),
      remotePort(remotePort),
      localAddress(localAddress),
      localPort(localPort),
      sslConfiguration(sslConfiguration)
{
}
#endif

/*!
    \class QHttpServerRequest
    \since 6.4
    \inmodule QtHttpServer
    \brief Encapsulates an HTTP request.

    API for accessing the different parameters of an incoming request.
*/

/*!
    \enum QHttpServerRequest::Method

    This enum type specifies an HTTP request method:

    \value Unknown
        An unknown method.
    \value Get
        HTTP GET method.
    \value Put
        HTTP PUT method.
    \value Delete
        HTTP DELETE method.
    \value Post
        HTTP POST method.
    \value Head
        HTTP HEAD method.
    \value Options
        HTTP OPTIONS method.
    \value Patch
        HTTP PATCH method (\l {https://www.rfc-editor.org/rfc/rfc5789}{RFC 5789}).
    \value Connect
        HTTP CONNECT method.
    \value Trace
        HTTP TRACE method.
    \value AnyKnown
        Combination of all known methods.
*/

/*!
    \internal
*/
QHttpServerRequest::QHttpServerRequest(const QHostAddress &remoteAddress, quint16 remotePort,
                                       const QHostAddress &localAddress, quint16 localPort)
    : d(new QHttpServerRequestPrivate(remoteAddress, remotePort, localAddress, localPort))
{}

#if QT_CONFIG(ssl)
/*!
    \internal
*/
QHttpServerRequest::QHttpServerRequest(const QHostAddress &remoteAddress, quint16 remotePort,
                                       const QHostAddress &localAddress, quint16 localPort,
                                       const QSslConfiguration &sslConfiguration)
    : d(new QHttpServerRequestPrivate(remoteAddress, remotePort, localAddress, localPort,
                                      sslConfiguration))
{}
#endif

/*!
    Destroys a QHttpServerRequest
*/
QHttpServerRequest::~QHttpServerRequest() { }

/*!
    Returns the combined value of all headers with the named \a key.
*/
QByteArray QHttpServerRequest::value(const QByteArray &key) const
{
    return d->headers.combinedValue(key);
}

/*!
    Returns the URL the request asked for.
*/
QUrl QHttpServerRequest::url() const
{
    return d->url;
}

/*!
    Returns the query in the request.
*/
QUrlQuery QHttpServerRequest::query() const
{
    return QUrlQuery(d->url.query());
}

/*!
    Returns the method of the request.
*/
QHttpServerRequest::Method QHttpServerRequest::method() const
{
    return d->method;
}

/*!
    \fn const QHttpHeaders &QHttpServerRequest::headers() const &
    \fn QHttpHeaders QHttpServerRequest::headers() &&

    Returns all the request headers.
*/
const QHttpHeaders &QHttpServerRequest::headers() const &
{
    return d->headers;
}

QHttpHeaders QHttpServerRequest::headers() &&
{
    return std::move(d->headers);
}

/*!
    Returns the body of the request.
*/
QByteArray QHttpServerRequest::body() const
{
    return d->body;
}

/*!
    Returns the address of the origin host of the request.
*/
QHostAddress QHttpServerRequest::remoteAddress() const
{
    return d->remoteAddress;
}

/*!
    Returns the port of the origin host of the request.

    \since 6.5
*/
quint16 QHttpServerRequest::remotePort() const
{
    return d->remotePort;
}

/*!
    Returns the host address of the local socket which received the request.

    \since 6.5
*/
QHostAddress QHttpServerRequest::localAddress() const
{
    return d->localAddress;
}

/*!
    Returns the port of the local socket which received the request.

    \since 6.5
*/
quint16 QHttpServerRequest::localPort() const
{
    return d->localPort;
}

#if QT_CONFIG(ssl)
/*!
    Returns the configuration of the established TLS connection.
    The configurations will return true for isNull() if the connection
    is not using TLS.

    \since 6.7
*/
QSslConfiguration QHttpServerRequest::sslConfiguration() const
{
    return d->sslConfiguration;
}
#endif

QT_END_NAMESPACE

#include "moc_qhttpserverrequest.cpp"
