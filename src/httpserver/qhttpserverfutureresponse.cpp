// Copyright (C) 2020 Mikhail Svetkin <mikhail.svetkin@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qhttpserverfutureresponse.h"

#include <QtCore/qfuture.h>
#include <QtCore/qfuturewatcher.h>

#include <QtNetwork/qtcpsocket.h>

#include <QtHttpServer/qhttpserverresponder.h>

#include <private/qhttpserverresponse_p.h>


QT_BEGIN_NAMESPACE

/*!
    \class QHttpServerFutureResponse
    \inmodule QtHttpServer
    \brief QHttpServerFutureResponse is a simplified API for asynchronous responses.

    \code

    QHttpServer server;

    server.route("/feature/", [] (int id) -> QHttpServerFutureResponse {
        auto future = QtConcurrent::run([] () {
            return QHttpServerResponse("the future is coming");
        });

        return future;
    });
    server.listen();

    \endcode
*/

/*!
    \internal
*/
struct QResponseWatcher : public QFutureWatcher<QHttpServerResponse>
{
    Q_OBJECT

public:
    QResponseWatcher(QHttpServerResponder &&_responder)
        : QFutureWatcher<QHttpServerResponse>(),
          responder(std::move(_responder)) {
    }

    QHttpServerResponder responder;
};

/*!
    \internal
*/
class QHttpServerFutureResponsePrivate : public QHttpServerResponsePrivate
{
public:
    QHttpServerFutureResponsePrivate(const QFuture<QHttpServerResponse> &futureResponse)
        : QHttpServerResponsePrivate(),
          futureResp(futureResponse)
    {
    }

    QFuture<QHttpServerResponse> futureResp;
};

/*!
    Constructs a new QHttpServerFutureResponse with the \a futureResp response.
*/
QHttpServerFutureResponse::QHttpServerFutureResponse(const QFuture<QHttpServerResponse> &futureResp)
    : QHttpServerFutureResponse(new QHttpServerFutureResponsePrivate{futureResp})
{
}

/*!
    \internal
*/
QHttpServerFutureResponse::QHttpServerFutureResponse(QHttpServerFutureResponsePrivate *d)
    : QHttpServerResponse(d)
{
}

/*!
    \reimp
*/
void QHttpServerFutureResponse::write(QHttpServerResponder &&responder) const
{
    if (!d_ptr->derived) {
        QHttpServerResponse::write(std::move(responder));
        return;
    }

    Q_D(const QHttpServerFutureResponse);

    auto socket = responder.socket();
    auto futureWatcher = new QResponseWatcher(std::move(responder));

    QObject::connect(socket, &QObject::destroyed,
                     futureWatcher, &QObject::deleteLater);
    QObject::connect(futureWatcher, &QFutureWatcherBase::finished,
                     socket,
                    [futureWatcher] () mutable {
        auto resp = futureWatcher->future().takeResult();
        resp.write(std::move(futureWatcher->responder));
        futureWatcher->deleteLater();
    });

    futureWatcher->setFuture(d->futureResp);
}

QT_END_NAMESPACE

#include "qhttpserverfutureresponse.moc"
