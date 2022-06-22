// Copyright (C) 2020 Mikhail Svetkin <mikhail.svetkin@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QHTTPSERVERFUTURERESPONSE_H
#define QHTTPSERVERFUTURERESPONSE_H

#include <QtHttpServer/qhttpserverresponse.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qfuture.h>

QT_BEGIN_NAMESPACE

class QHttpServerFutureResponsePrivate;
class Q_HTTPSERVER_EXPORT QHttpServerFutureResponse : public QHttpServerResponse
{
    Q_DECLARE_PRIVATE(QHttpServerFutureResponse)

public:
    using QHttpServerResponse::QHttpServerResponse;

    QHttpServerFutureResponse(const QFuture<QHttpServerResponse> &futureResponse);

    virtual void write(QHttpServerResponder &&responder) const override;

protected:
    QHttpServerFutureResponse(QHttpServerFutureResponsePrivate *d);
};

QT_END_NAMESPACE

#endif // QHTTPSERVERFUTURERESPONSE_H
