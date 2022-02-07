/****************************************************************************
**
** Copyright (C) 2020 Mikhail Svetkin <mikhail.svetkin@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtHttpServer module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QHTTPSERVERFUTURERESPONSE_H
#define QHTTPSERVERFUTURERESPONSE_H

#include <QtHttpServer/qhttpserverresponse.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qfuture.h>

#include <QtConcurrent/qtconcurrentrunbase.h>

#include <mutex>

QT_BEGIN_NAMESPACE

namespace QtConcurrent {

template <>
class RunFunctionTask<QHttpServerResponse> : public RunFunctionTaskBase<QHttpServerResponse>
{
public:
    void run() override
    {
        if (promise.isCanceled()) {
            promise.reportFinished();
            return;
        }
#ifndef QT_NO_EXCEPTIONS
        try {
#endif
            this->runFunctor();
#ifndef QT_NO_EXCEPTIONS
        } catch (QException &e) {
            promise.reportException(e);
        } catch (...) {
            promise.reportException(QUnhandledException());
        }
#endif
        promise.reportAndMoveResult(std::move_if_noexcept(result));
        promise.reportFinished();
    }

    QHttpServerResponse result{QHttpServerResponse::StatusCode::NotFound};
};

}

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