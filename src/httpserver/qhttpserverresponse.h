// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QHTTPSERVERRESPONSE_H
#define QHTTPSERVERRESPONSE_H

#include <QtHttpServer/qhttpserverresponder.h>
#include <QtNetwork/qhttpheaders.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QJsonObject;

class QHttpServerResponsePrivate;
class Q_HTTPSERVER_EXPORT QHttpServerResponse final
{
    Q_DECLARE_PRIVATE(QHttpServerResponse)
    Q_DISABLE_COPY(QHttpServerResponse)

    friend class QHttpServerResponder;
public:
    using StatusCode = QHttpServerResponder::StatusCode;

    QHttpServerResponse(QHttpServerResponse &&other) noexcept;
    QHttpServerResponse& operator=(QHttpServerResponse &&other) noexcept;

    QHttpServerResponse(StatusCode statusCode);

    QHttpServerResponse(const char *data, StatusCode status = StatusCode::Ok);

    QHttpServerResponse(const QString &data, StatusCode status = StatusCode::Ok);

    explicit QHttpServerResponse(const QByteArray &data, StatusCode status = StatusCode::Ok);
    explicit QHttpServerResponse(QByteArray &&data, StatusCode status = StatusCode::Ok);

    QHttpServerResponse(const QJsonObject &data, StatusCode status = StatusCode::Ok);
    QHttpServerResponse(const QJsonArray &data, StatusCode status = StatusCode::Ok);

    QHttpServerResponse(const QByteArray &mimeType, const QByteArray &data,
                        StatusCode status = StatusCode::Ok);
    QHttpServerResponse(const QByteArray &mimeType, QByteArray &&data,
                        StatusCode status = StatusCode::Ok);

    ~QHttpServerResponse();
    static QHttpServerResponse fromFile(const QString &fileName);

    QByteArray data() const;

    QByteArray mimeType() const;

    StatusCode statusCode() const;

    void addHeader(const QByteArray &name, const QByteArray &value);

    void clearHeader(const QByteArray &name);
    void clearHeaders();

    void setHeader(const QByteArray &name, const QByteArray &value);

    QHttpServerResponse& withHeaders(const QHttpHeaders &headers);
    QHttpServerResponse& withHeaders(QHttpHeaders &&headers);

    bool hasHeader(const QByteArray &name) const;
    bool hasHeader(const QByteArray &name, const QByteArray &value) const;

    QHttpHeaders headers() const;
    QList<QByteArray> headerData(const QByteArray &name) const;

private:
    std::unique_ptr<QHttpServerResponsePrivate> d_ptr;
};

QT_END_NAMESPACE

#endif   // QHTTPSERVERRESPONSE_H
