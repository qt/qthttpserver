// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtHttpServer/qhttpserverresponse.h>

#include <private/qhttpserverliterals_p.h>
#include <private/qhttpserverresponse_p.h>
#include <private/qhttpserverresponder_p.h>

#include <QtCore/qfile.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qmimedatabase.h>
#include <QtNetwork/qtcpsocket.h>

QT_BEGIN_NAMESPACE

/*!
    \class QHttpServerResponse
    \since 6.4
    \inmodule QtHttpServer
    \brief Encapsulates an HTTP response.

    API for creating, reading and modifying a response from an HTTP server,
    and for writing its contents to a QHttpServerResponder.
    It has numerous constructors, and \c static function \c fromFile for
    constructing it from the contents of a file. There are functions for
    setting, getting, and removing headers, and for getting the data, status
    code and mime type.
*/

/*!
    \internal
*/
QHttpServerResponsePrivate::QHttpServerResponsePrivate(
        QByteArray &&d, const QHttpServerResponse::StatusCode sc)
    : data(std::move(d)),
      statusCode(sc)
{ }

/*!
    \internal
*/
QHttpServerResponsePrivate::QHttpServerResponsePrivate(const QHttpServerResponse::StatusCode sc)
    : statusCode(sc)
{ }

/*!
    \typealias QHttpServerResponse::StatusCode

    Type alias for QHttpServerResponder::StatusCode
*/

/*!
    Move-constructs an instance of a QHttpServerResponse object from \a other.
*/
QHttpServerResponse::QHttpServerResponse(QHttpServerResponse &&other) noexcept
    : d_ptr(std::move(other.d_ptr))
{
}

/*!
    Move-assigns the values of \a other to this object.
*/
QHttpServerResponse& QHttpServerResponse::operator=(QHttpServerResponse &&other) noexcept
{
    if (this == &other)
        return *this;

    qSwap(d_ptr, other.d_ptr);
    return *this;
}

/*!
    Creates a QHttpServerResponse object with the status code \a statusCode.
*/
QHttpServerResponse::QHttpServerResponse(QHttpServerResponse::StatusCode statusCode)
    : QHttpServerResponse(QHttpServerLiterals::contentTypeXEmpty(), QByteArray(), statusCode)
{
}

/*!
    Creates a QHttpServerResponse object from \a data with the status code \a status.
*/
QHttpServerResponse::QHttpServerResponse(const char *data, StatusCode status)
    : QHttpServerResponse(QByteArray::fromRawData(data, qstrlen(data)), status)
{
}

/*!
    Creates a QHttpServerResponse object from \a data with the status code \a status.
*/
QHttpServerResponse::QHttpServerResponse(const QString &data, StatusCode status)
    : QHttpServerResponse(data.toUtf8(), status)
{
}

/*!
    Creates a QHttpServerResponse object from \a data with the status code \a status.
*/
QHttpServerResponse::QHttpServerResponse(const QByteArray &data, StatusCode status)
    : QHttpServerResponse(QMimeDatabase().mimeTypeForData(data).name().toLocal8Bit(), data, status)
{
}

/*!
    Move-constructs a QHttpServerResponse whose body will contain the given
    \a data with the status code \a status.
*/
QHttpServerResponse::QHttpServerResponse(QByteArray &&data, StatusCode status)
    : QHttpServerResponse(QMimeDatabase().mimeTypeForData(data).name().toLocal8Bit(),
                          std::move(data), status)
{
}

/*!
    Creates a QHttpServerResponse object from \a data with the status code \a status.
*/
QHttpServerResponse::QHttpServerResponse(const QJsonObject &data, StatusCode status)
    : QHttpServerResponse(QHttpServerLiterals::contentTypeJson(),
                          QJsonDocument(data).toJson(QJsonDocument::Compact), status)
{
}

/*!
    Creates a QHttpServerResponse object from \a data with the status code \a status.
*/
QHttpServerResponse::QHttpServerResponse(const QJsonArray &data, StatusCode status)
    : QHttpServerResponse(QHttpServerLiterals::contentTypeJson(),
                          QJsonDocument(data).toJson(QJsonDocument::Compact), status)
{
}

/*!
    \fn QHttpServerResponse::QHttpServerResponse(const QByteArray &mimeType,
                                                 const QByteArray &data,
                                                 StatusCode status)
    \fn QHttpServerResponse::QHttpServerResponse(const QByteArray &mimeType,
                                                 QByteArray &&data,
                                                 StatusCode status)

    Creates a QHttpServer response.

    The response will use the given \a status code and deliver the \a data as
    its body, with a \c ContentType header describing it as being of MIME type
    \a mimeType.
*/
QHttpServerResponse::QHttpServerResponse(const QByteArray &mimeType,
                                         const QByteArray &data,
                                         StatusCode status)
    : d_ptr(new QHttpServerResponsePrivate(QByteArray(data), status))
{
    if (!mimeType.isEmpty()) {
        d_ptr->headers.append(QHttpHeaders::WellKnownHeader::ContentType, mimeType);
    }
}

QHttpServerResponse::QHttpServerResponse(const QByteArray &mimeType,
                                         QByteArray &&data,
                                         StatusCode status)
    : d_ptr(new QHttpServerResponsePrivate(std::move(data), status))
{
    if (!mimeType.isEmpty()) {
        d_ptr->headers.append(QHttpHeaders::WellKnownHeader::ContentType, mimeType);
    }
}

/*!
    Destroys a QHttpServerResponse object.
*/
QHttpServerResponse::~QHttpServerResponse()
{
}

/*!
    Returns a QHttpServerResponse from the content of the file \a fileName.

    It is the caller's responsibility to sanity-check the filename, and to have
    a well-defined policy for which files the server will request.
*/
QHttpServerResponse QHttpServerResponse::fromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return QHttpServerResponse(StatusCode::NotFound);
    const QByteArray data = file.readAll();
    file.close();
    const QByteArray mimeType = QMimeDatabase().mimeTypeForFileNameAndData(fileName, data).name().toLocal8Bit();
    return QHttpServerResponse(mimeType, data);
}

/*!
    Returns the response body.
*/
QByteArray QHttpServerResponse::data() const
{
    Q_D(const QHttpServerResponse);
    return d->data;
}

/*!
    Returns the status code.
*/
QHttpServerResponse::StatusCode QHttpServerResponse::statusCode() const
{
    Q_D(const QHttpServerResponse);
    return d->statusCode;
}

/*!
    Adds the HTTP header with name \a name and value \a value,
    does not override any previously set headers.
*/
void QHttpServerResponse::addHeader(const QByteArray &name, const QByteArray &value)
{
    Q_D(QHttpServerResponse);
    d->headers.append(name, value);
}

/*!
    Removes the HTTP header with name \a name.
*/
void QHttpServerResponse::clearHeader(const QByteArray &name)
{
    Q_D(QHttpServerResponse);
    d->headers.removeAll(name);
}

/*!
    Removes all HTTP headers.
*/
void QHttpServerResponse::clearHeaders()
{
    Q_D(QHttpServerResponse);
    d->headers.clear();
}

/*!
    Sets the HTTP header with name \a name and value \a value,
    overriding any previously set headers.
*/
void QHttpServerResponse::setHeader(const QByteArray &name, const QByteArray &value)
{
    Q_D(QHttpServerResponse);
    d->headers.removeAll(name);
    d->headers.append(name, value);
}

/*!
    Sets \a headers as the HTTP headers, overriding any previously set headers.
    Returns a reference to this object.

    \since 6.8
*/
QHttpServerResponse &QHttpServerResponse::withHeaders(const QHttpHeaders &headers)
{
    Q_D(QHttpServerResponse);

    d->headers = headers;
    return *this;
}

/*!
    Sets \a headers as the HTTP headers, overriding any previously set headers.
    Returns a reference to this object.

    \since 6.8
*/
QHttpServerResponse& QHttpServerResponse::withHeaders(QHttpHeaders &&headers)
{
    Q_D(QHttpServerResponse);

    d->headers = std::move(headers);
    return *this;
}

/*!
    Returns the value of the HTTP "Content-Type" header.

    \note Default value is "text/html"
*/
QByteArray QHttpServerResponse::mimeType() const
{
    Q_D(const QHttpServerResponse);
    return d->headers.value(QHttpHeaders::WellKnownHeader::ContentType,
                            QHttpServerLiterals::contentTypeTextHtml()).toByteArray();
}

/*!
    Returns true if the response contains an HTTP header with name \a header,
    otherwise returns false.
*/
bool QHttpServerResponse::hasHeader(const QByteArray &header) const
{
    Q_D(const QHttpServerResponse);
    return d->headers.contains(header);
}

/*!
    Returns true if the response contains an HTTP header with name \a name and
    with value \a value, otherwise returns false.
*/
bool QHttpServerResponse::hasHeader(const QByteArray &name,
                                    const QByteArray &value) const
{
    Q_D(const QHttpServerResponse);
    return d->headers.values(name).contains(value);
}

/*!
    Returns the currently set HTTP headers.

    \since 6.8
*/
QHttpHeaders QHttpServerResponse::headers() const
{
    Q_D(const QHttpServerResponse);
    return d->headers;
}

/*!
    Returns values of the HTTP header with name \a name.
*/
QList<QByteArray> QHttpServerResponse::headerData(const QByteArray &name) const
{
    Q_D(const QHttpServerResponse);
    return d->headers.values(name);
}

QT_END_NAMESPACE
