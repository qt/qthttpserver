// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtHttpServer/qhttpserverresponder.h>
#include <QtHttpServer/qhttpserverrequest.h>
#include <QtHttpServer/qhttpserverresponse.h>
#include <private/qhttpserverresponder_p.h>
#include <private/qhttpserverliterals_p.h>
#include <private/qhttpserverrequest_p.h>
#include <private/qhttpserverresponse_p.h>
#include <private/qhttpserverstream_p.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qtimer.h>
#include <QtNetwork/qtcpsocket.h>
#include <map>
#include <memory>

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN_TAGGED(QHttpServerResponder::StatusCode, QHttpServerResponder__StatusCode)

/*!
    \class QHttpServerResponder
    \since 6.4
    \inmodule QtHttpServer
    \brief API for sending replies from an HTTP server.

    Provides functions for writing back to an HTTP client with overloads for
    serializing JSON objects. It also has support for writing HTTP headers and
    status code.
*/

/*!
    \enum QHttpServerResponder::StatusCode

    HTTP status codes

    \value Continue
    \value SwitchingProtocols
    \value Processing

    \value Ok
    \value Created
    \value Accepted
    \value NonAuthoritativeInformation
    \value NoContent
    \value ResetContent
    \value PartialContent
    \value MultiStatus
    \value AlreadyReported
    \value IMUsed

    \value MultipleChoices
    \value MovedPermanently
    \value Found
    \value SeeOther
    \value NotModified
    \value UseProxy

    \value TemporaryRedirect
    \value PermanentRedirect

    \value BadRequest
    \value Unauthorized
    \value PaymentRequired
    \value Forbidden
    \value NotFound
    \value MethodNotAllowed
    \value NotAcceptable
    \value ProxyAuthenticationRequired
    \value RequestTimeout
    \value Conflict
    \value Gone
    \value LengthRequired
    \value PreconditionFailed
    \value PayloadTooLarge
    \value UriTooLong
    \value UnsupportedMediaType
    \value RequestRangeNotSatisfiable
    \value ExpectationFailed
    \value ImATeapot
    \value MisdirectedRequest
    \value UnprocessableEntity
    \value Locked
    \value FailedDependency
    \value UpgradeRequired
    \value PreconditionRequired
    \value TooManyRequests
    \value RequestHeaderFieldsTooLarge
    \value UnavailableForLegalReasons

    \value InternalServerError
    \value NotImplemented
    \value BadGateway
    \value ServiceUnavailable
    \value GatewayTimeout
    \value HttpVersionNotSupported
    \value VariantAlsoNegotiates
    \value InsufficientStorage
    \value LoopDetected
    \value NotExtended
    \value NetworkAuthenticationRequired
    \value NetworkConnectTimeoutError
*/

/*!
    \internal
*/
static const QLoggingCategory &rspLc()
{
    static const QLoggingCategory category("qt.httpserver.response");
    return category;
}

// https://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
static const std::map<QHttpServerResponder::StatusCode, QByteArray> statusString{
#define XX(name, string) { QHttpServerResponder::StatusCode::name, QByteArrayLiteral(string) }
    XX(Continue, "Continue"),
    XX(SwitchingProtocols, "Switching Protocols"),
    XX(Processing, "Processing"),
    XX(Ok, "OK"),
    XX(Created, "Created"),
    XX(Accepted, "Accepted"),
    XX(NonAuthoritativeInformation, "Non-Authoritative Information"),
    XX(NoContent, "No Content"),
    XX(ResetContent, "Reset Content"),
    XX(PartialContent, "Partial Content"),
    XX(MultiStatus, "Multi-Status"),
    XX(AlreadyReported, "Already Reported"),
    XX(IMUsed, "I'm Used"),
    XX(MultipleChoices, "Multiple Choices"),
    XX(MovedPermanently, "Moved Permanently"),
    XX(Found, "Found"),
    XX(SeeOther, "See Other"),
    XX(NotModified, "Not Modified"),
    XX(UseProxy, "Use Proxy"),
    XX(TemporaryRedirect, "Temporary Redirect"),
    XX(PermanentRedirect, "Permanent Redirect"),
    XX(BadRequest, "Bad Request"),
    XX(Unauthorized, "Unauthorized"),
    XX(PaymentRequired, "Payment Required"),
    XX(Forbidden, "Forbidden"),
    XX(NotFound, "Not Found"),
    XX(MethodNotAllowed, "Method Not Allowed"),
    XX(NotAcceptable, "Not Acceptable"),
    XX(ProxyAuthenticationRequired, "Proxy Authentication Required"),
    XX(RequestTimeout, "Request Timeout"),
    XX(Conflict, "Conflict"),
    XX(Gone, "Gone"),
    XX(LengthRequired, "Length Required"),
    XX(PreconditionFailed, "Precondition Failed"),
    XX(PayloadTooLarge, "Request Entity Too Large"),
    XX(UriTooLong, "Request-URI Too Long"),
    XX(UnsupportedMediaType, "Unsupported Media Type"),
    XX(RequestRangeNotSatisfiable, "Requested Range Not Satisfiable"),
    XX(ExpectationFailed, "Expectation Failed"),
    XX(ImATeapot, "I'm a teapot"),
    XX(MisdirectedRequest, "Misdirected Request"),
    XX(UnprocessableEntity, "Unprocessable Entity"),
    XX(Locked, "Locked"),
    XX(FailedDependency, "Failed Dependency"),
    XX(UpgradeRequired, "Upgrade Required"),
    XX(PreconditionRequired, "Precondition Required"),
    XX(TooManyRequests, "Too Many Requests"),
    XX(RequestHeaderFieldsTooLarge, "Request Header Fields Too Large"),
    XX(UnavailableForLegalReasons, "Unavailable For Legal Reasons"),
    XX(InternalServerError, "Internal Server Error"),
    XX(NotImplemented, "Not Implemented"),
    XX(BadGateway, "Bad Gateway"),
    XX(ServiceUnavailable, "Service Unavailable"),
    XX(GatewayTimeout, "Gateway Timeout"),
    XX(HttpVersionNotSupported, "HTTP Version Not Supported"),
    XX(VariantAlsoNegotiates, "Variant Also Negotiates"),
    XX(InsufficientStorage, "Insufficient Storage"),
    XX(LoopDetected, "Loop Detected"),
    XX(NotExtended, "Not Extended"),
    XX(NetworkAuthenticationRequired, "Network Authentication Required"),
    XX(NetworkConnectTimeoutError, "Network Connect Timeout Error"),
#undef XX
};

/*!
    \internal
*/
template <qint64 BUFFERSIZE = 128 * 1024>
struct IOChunkedTransfer
{
    // TODO This is not the fastest implementation, as it does read & write
    // in a sequential fashion, but these operation could potentially overlap.
    // TODO Can we implement it without the buffer? Direct write to the target buffer
    // would be great.

    const qint64 bufferSize = BUFFERSIZE;
    char buffer[BUFFERSIZE];
    qint64 beginIndex = -1;
    qint64 endIndex = -1;
    QPointer<QIODevice> source;
    const QPointer<QIODevice> sink;
    const QMetaObject::Connection bytesWrittenConnection;
    const QMetaObject::Connection readyReadConnection;
    IOChunkedTransfer(QIODevice *input, QIODevice *output) :
        source(input),
        sink(output),
        bytesWrittenConnection(QObject::connect(sink.data(), &QIODevice::bytesWritten, sink.data(), [this]() {
              writeToOutput();
        })),
        readyReadConnection(QObject::connect(source.data(), &QIODevice::readyRead, source.data(), [this]() {
            readFromInput();
        }))
    {
        Q_ASSERT(!source->atEnd());  // TODO error out
        QObject::connect(sink.data(), &QObject::destroyed, source.data(), &QObject::deleteLater);
        QObject::connect(source.data(), &QObject::destroyed, source.data(), [this]() {
            delete this;
        });
        readFromInput();
    }

    ~IOChunkedTransfer()
    {
        QObject::disconnect(bytesWrittenConnection);
        QObject::disconnect(readyReadConnection);
    }

    inline bool isBufferEmpty()
    {
        Q_ASSERT(beginIndex <= endIndex);
        return beginIndex == endIndex;
    }

    void readFromInput()
    {
        if (source.isNull())
            return;

        if (!isBufferEmpty()) // We haven't consumed all the data yet.
            return;
        beginIndex = 0;
        endIndex = source->read(buffer, bufferSize);
        if (endIndex < 0) {
            endIndex = beginIndex; // Mark the buffer as empty
            qCWarning(rspLc, "Error reading chunk: %ls", qUtf16Printable(source->errorString()));
        } else if (endIndex) {
            memset(buffer + endIndex, 0, sizeof(buffer) - std::size_t(endIndex));
            writeToOutput();
        }
    }

    void writeToOutput()
    {
        if (sink.isNull() || source.isNull())
            return;

        if (isBufferEmpty())
            return;

        const auto writtenBytes = sink->write(buffer + beginIndex, endIndex);
        if (writtenBytes < 0) {
            qCWarning(rspLc, "Error writing chunk: %ls", qUtf16Printable(sink->errorString()));
            return;
        }
        beginIndex += writtenBytes;
        if (isBufferEmpty()) {
            if (source->bytesAvailable())
                QTimer::singleShot(0, source.data(), [this]() { readFromInput(); });
            else if (source->atEnd()) // Finishing
                source->deleteLater();
        }
    }
};

/*!
    \internal
*/
QHttpServerResponderPrivate::QHttpServerResponderPrivate(QHttpServerStream *stream) : stream(stream)
{
    Q_ASSERT(stream);
    Q_ASSERT(!stream->handlingRequest);
    stream->handlingRequest = true;
}

/*!
    \internal
*/
QHttpServerResponderPrivate::~QHttpServerResponderPrivate()
{
    Q_ASSERT(stream);
    stream->responderDestroyed();
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::writeStatusAndHeaders(QHttpServerResponder::StatusCode status,
                                                        const QHttpHeaders &headers)
{
    Q_ASSERT(stream);
    Q_ASSERT(state == TransferState::Ready);
    stream->write("HTTP/1.1 ");
    stream->write(QByteArray::number(quint32(status)));
    const auto it = statusString.find(status);
    if (it != statusString.end()) {
        stream->write(" ");
        stream->write(statusString.at(status));
    }
    stream->write("\r\n");

    for (qsizetype i = 0; i < headers.size(); ++i) {
        const auto name = headers.nameAt(i);
        writeHeader(QByteArray(name.data(), name.size()), headers.valueAt(i).toByteArray());
    }
    stream->write("\r\n");
    state = TransferState::HeadersSent;
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::write(QHttpServerResponder::StatusCode status)
{
    Q_ASSERT(stream);
    Q_ASSERT(state == TransferState::Ready);
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::ContentType,
                   QHttpServerLiterals::contentTypeXEmpty());
    headers.append(QHttpHeaders::WellKnownHeader::ContentLength, "0");
    writeStatusAndHeaders(status, headers);
    state = TransferState::Ready;
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::writeHeader(const QByteArray &header, const QByteArray &value)
{
    Q_ASSERT(stream);
    stream->write(header);
    stream->write(": ");
    stream->write(value);
    stream->write("\r\n");
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::write(const QByteArray &body, const QHttpHeaders &headers,
                                        QHttpServerResponder::StatusCode status)
{
    Q_ASSERT(stream);
    Q_ASSERT(state == TransferState::Ready);
    writeStatusAndHeaders(status, headers);
    stream->write(body.constData(), body.size());
    state = TransferState::Ready;
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::write(QIODevice *data, const QHttpHeaders &headers,
                                        QHttpServerResponder::StatusCode status)
{
    Q_ASSERT(stream);
    Q_ASSERT(state == TransferState::Ready);
    std::unique_ptr<QIODevice, QScopedPointerDeleteLater> input(data);

    input->setParent(nullptr);
    if (!input->isOpen()) {
        if (!input->open(QIODevice::ReadOnly)) {
            // TODO Add developer error handling
            qCDebug(rspLc, "500: Could not open device %ls", qUtf16Printable(input->errorString()));
            write(QHttpServerResponder::StatusCode::InternalServerError);
            return;
        }
    } else if (!(input->openMode() & QIODevice::ReadOnly)) {
        // TODO Add developer error handling
        qCDebug(rspLc) << "500: Device is opened in a wrong mode" << input->openMode();
        write(QHttpServerResponder::StatusCode::InternalServerError);
        return;
    }

    QHttpHeaders allHeaders(headers);
    if (!input->isSequential()) { // Non-sequential QIODevice should know its data size
        allHeaders.append(QHttpHeaders::WellKnownHeader::ContentLength,
                          QByteArray::number(input->size()));
    }

    writeStatusAndHeaders(status, allHeaders);

    if (input->atEnd()) {
        qCDebug(rspLc, "No more data available.");
        return;
    }

    // input takes ownership of the IOChunkedTransfer pointer inside his constructor
    new IOChunkedTransfer<>(input.release(), stream->socket);
    state = TransferState::Ready;
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::writeBeginChunked(const QHttpHeaders &headers,
                                                    QHttpServerResponder::StatusCode status)
{
    Q_ASSERT(state == TransferState::Ready);
    QHttpHeaders allHeaders(headers);
    allHeaders.append(QHttpHeaders::WellKnownHeader::TransferEncoding, "chunked");
    writeStatusAndHeaders(status, allHeaders);
    state = TransferState::ChunkedTransferBegun;
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::writeChunk(const QByteArray &data)
{
    Q_ASSERT(state == TransferState::ChunkedTransferBegun);
    if (data.length() == 0) {
        qCWarning(rspLc, "Chunk must have length > 0");
        return;
    }

    stream->write(QByteArray::number(data.length(), 16));
    stream->write("\r\n");
    stream->write(data);
    stream->write("\r\n");
}

/*!
    \internal
*/
void QHttpServerResponderPrivate::writeEndChunked(const QByteArray &data,
                                                  const QHttpHeaders &trailers)
{
    Q_ASSERT(state == TransferState::ChunkedTransferBegun);
    writeChunk(data);
    stream->write("0\r\n");
    for (qsizetype i = 0; i < trailers.size(); ++i) {
        const auto name = trailers.nameAt(i);
        const auto value = trailers.valueAt(i);
        writeHeader({ name.data(), name.size() }, value.toByteArray());
    }
    stream->write("\r\n");
    state = TransferState::Ready;
}

/*!
    Constructs a QHttpServerResponder instance using a \a stream
    to output the response to.
*/
QHttpServerResponder::QHttpServerResponder(QHttpServerStream *stream)
    : d_ptr(new QHttpServerResponderPrivate(stream))
{
    Q_ASSERT(stream);
}

/*!
    Move-constructs a QHttpServerResponder instance, making it point
    at the same object that \a other was pointing to.
*/
QHttpServerResponder::QHttpServerResponder(QHttpServerResponder &&other)
    : d_ptr(std::move(other.d_ptr))
{}

/*!
    Destroys a QHttpServerResponder.
*/
QHttpServerResponder::~QHttpServerResponder() = default;

/*!
    Answers a request with an HTTP status code \a status and
    HTTP headers \a headers. The I/O device \a data provides the body
    of the response. If \a data is sequential, the body of the
    message is sent in chunks: otherwise, the function assumes all
    the content is available and sends it all at once but the read
    is done in chunks.

    \note This function takes the ownership of \a data.
*/
void QHttpServerResponder::write(QIODevice *data,
                                 const QHttpHeaders &headers,
                                 StatusCode status)
{
    Q_D(QHttpServerResponder);
    d->write(data, headers, status);
}

/*!
    Answers a request with an HTTP status code \a status and a
    MIME type \a mimeType. The I/O device \a data provides the body
    of the response. If \a data is sequential, the body of the
    message is sent in chunks: otherwise, the function assumes all
    the content is available and sends it all at once but the read
    is done in chunks.

    \note This function takes the ownership of \a data.
*/
void QHttpServerResponder::write(QIODevice *data,
                                 const QByteArray &mimeType,
                                 StatusCode status)
{
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::ContentType, mimeType);
    write(data, headers, status);
}

/*!
    Answers a request with an HTTP status code \a status, JSON
    document \a document and HTTP headers \a headers.

    Note: This function sets HTTP Content-Type header as "application/json".
*/
void QHttpServerResponder::write(const QJsonDocument &document,
                                 const QHttpHeaders &headers,
                                 StatusCode status)
{
    Q_D(QHttpServerResponder);
    const QByteArray &json = document.toJson();
    QHttpHeaders allHeaders(headers);
    allHeaders.append(QHttpHeaders::WellKnownHeader::ContentType,
                      QHttpServerLiterals::contentTypeJson());
    allHeaders.append(QHttpHeaders::WellKnownHeader::ContentLength,
                      QByteArray::number(json.size()));
    d->write(document.toJson(), allHeaders, status);
}

/*!
    Answers a request with an HTTP status code \a status, and JSON
    document \a document.

    Note: This function sets HTTP Content-Type header as "application/json".
*/
void QHttpServerResponder::write(const QJsonDocument &document,
                                 StatusCode status)
{
    write(document, {}, status);
}

/*!
    Answers a request with an HTTP status code \a status,
    HTTP Headers \a headers and a body \a data.

    Note: This function sets HTTP Content-Length header.
*/
void QHttpServerResponder::write(const QByteArray &data,
                                 const QHttpHeaders &headers,
                                 StatusCode status)
{
    Q_D(QHttpServerResponder);
    QHttpHeaders allHeaders(headers);
    allHeaders.append(QHttpHeaders::WellKnownHeader::ContentLength,
                      QByteArray::number(data.size()));
    d->write(data, allHeaders, status);
}

/*!
    Answers a request with an HTTP status code \a status, a
    MIME type \a mimeType and a body \a data.
*/
void QHttpServerResponder::write(const QByteArray &data,
                                 const QByteArray &mimeType,
                                 StatusCode status)
{
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::ContentType, mimeType);
    write(data, headers, status);
}

/*!
    Answers a request with an HTTP status code \a status.

    Note: This function sets HTTP Content-Type header as "application/x-empty".
*/
void QHttpServerResponder::write(StatusCode status)
{
    write(QByteArray(), QHttpServerLiterals::contentTypeXEmpty(), status);
}

/*!
    Answers a request with an HTTP status code \a status and
    HTTP Headers \a headers.
*/
void QHttpServerResponder::write(const QHttpHeaders &headers, StatusCode status)
{
    write(QByteArray(), headers, status);
}

/*!
    Sends a HTTP \a response to the client.

    \since 6.5
*/
void QHttpServerResponder::sendResponse(const QHttpServerResponse &response)
{
    Q_D(QHttpServerResponder);
    const auto &r = response.d_ptr;
    QHttpHeaders allHeaders(r->headers);
    allHeaders.append(QHttpHeaders::WellKnownHeader::ContentLength,
                      QByteArray::number(r->data.size()));

    d->write(r->data, allHeaders, r->statusCode);
}

/*!
    Start sending chunks of data with \a headers and and the status
    code \a status. This call must be followed up with an arbitrary
    number of repeated \c writeChunk calls and and a single call to
    \c writeEndChunked.

    \since 6.8
    \sa writeChunk, writeEndChunked
*/
void QHttpServerResponder::writeBeginChunked(const QHttpHeaders &headers, StatusCode status)
{
    Q_D(QHttpServerResponder);
    d->writeBeginChunked(headers, status);
}

/*!
    Start sending chunks of data with the mime type \a mimeType and
    and the given status code \a status. This call must be followed
    up with an arbitrary number of repeated \c writeChunk calls and
    and a single call to \c writeEndChunked.

    \since 6.8
    \sa writeChunk, writeEndChunked
*/
void QHttpServerResponder::writeBeginChunked(const QByteArray &mimeType, StatusCode status)
{
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::ContentType, mimeType);
    writeBeginChunked(headers, status);
}

/*!
    Start sending chunks of data with \a headers and and the given
    status code \a status. This call must be followed up with an
    arbitrary number of repeated \c writeChunk calls and and a single
    call to \c writeEndChunked with the same trailers given in
    \a trailers.

    \since 6.8
    \sa writeChunk, writeEndChunked
*/
void QHttpServerResponder::writeBeginChunked(const QHttpHeaders &headers,
                                             QList<QHttpHeaders::WellKnownHeader> trailers,
                                             StatusCode status)
{
    QHttpHeaders allHeaders(headers);
    QByteArray trailerList;
    for (qsizetype i = 0; i < trailers.size(); ++i) {
        if (i != 0)
            trailerList.append(", ");

        trailerList.append(QHttpHeaders::wellKnownHeaderName(trailers[i]).toByteArray());
    }
    allHeaders.append(QHttpHeaders::WellKnownHeader::Trailer, trailerList);
    writeBeginChunked(allHeaders, status);
}

/*!
    Write \a data back to the client. To be called when data is
    available to write. This can be called multiple times, but before
    calling this \c writeBeginChunked must called, and afterwards
    \c writeEndChunked must be called.

    \sa writeBeginChunked, writeEndChunked
    \since 6.8
*/
void QHttpServerResponder::writeChunk(const QByteArray &data)
{
    Q_D(QHttpServerResponder);
    d->writeChunk(data);
}

/*!
    Write \a data back to the client with the \a trailers
    announced in \c writeBeginChunked.

    \since 6.8
    \sa writeBeginChunked, writeChunk
*/
void QHttpServerResponder::writeEndChunked(const QByteArray &data, const QHttpHeaders &trailers)
{
    Q_D(QHttpServerResponder);
    d->writeEndChunked(data, trailers);
}

/*!
    Write \a data back to the client. Must be preceded
    by a call to \c writeBeginChunked.

    \since 6.8
    \sa writeBeginChunked, writeChunk
*/
void QHttpServerResponder::writeEndChunked(const QByteArray &data)
{
    writeEndChunked(data, {});
}

QT_END_NAMESPACE
