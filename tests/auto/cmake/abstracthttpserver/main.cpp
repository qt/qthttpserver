// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtHttpServer/qabstracthttpserver.h>

int main()
{
    struct HttpServer : QAbstractHttpServer {
        bool handleRequest(const QHttpServerRequest &, QTcpSocket *) override
        {
            return false;
        }
    } httpServer;
    Q_UNUSED(httpServer);
    return 0;
}
