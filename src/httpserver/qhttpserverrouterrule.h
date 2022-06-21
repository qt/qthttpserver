// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QHTTPSERVERROUTERRULE_H
#define QHTTPSERVERROUTERRULE_H

#include <QtHttpServer/qhttpserverrequest.h>

#include <QtCore/qmap.h>

#include <functional> // for std::function
#include <initializer_list>

QT_BEGIN_NAMESPACE

class QString;
class QHttpServerRequest;
class QTcpSocket;
class QRegularExpressionMatch;
class QHttpServerRouter;

class QHttpServerRouterRulePrivate;
class Q_HTTPSERVER_EXPORT QHttpServerRouterRule
{
    Q_DECLARE_PRIVATE(QHttpServerRouterRule)
    Q_DISABLE_COPY_MOVE(QHttpServerRouterRule)

public:
    using RouterHandler = std::function<void(const QRegularExpressionMatch &,
                                             const QHttpServerRequest &,
                                             QTcpSocket *)>;

    explicit QHttpServerRouterRule(const QString &pathPattern, RouterHandler routerHandler);
    explicit QHttpServerRouterRule(const QString &pathPattern,
                                   const QHttpServerRequest::Methods methods,
                                   RouterHandler routerHandler);
    explicit QHttpServerRouterRule(const QString &pathPattern,
                                   const char * methods,
                                   RouterHandler routerHandler);
    virtual ~QHttpServerRouterRule();

protected:
    bool exec(const QHttpServerRequest &request, QTcpSocket *socket) const;

    bool hasValidMethods() const;

    bool createPathRegexp(std::initializer_list<int> metaTypes,
                          const QMap<int, QLatin1StringView> &converters);

    virtual bool matches(const QHttpServerRequest &request,
                         QRegularExpressionMatch *match) const;

    QHttpServerRouterRule(QHttpServerRouterRulePrivate *d);

private:
    QScopedPointer<QHttpServerRouterRulePrivate> d_ptr;

    friend class QHttpServerRouter;
};

QT_END_NAMESPACE

#endif // QHTTPSERVERROUTERRULE_H
