// Copyright (C) 2020 Mikhail Svetkin <mikhail.svetkin@gmail.com>
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QHTTPSERVER_H
#define QHTTPSERVER_H

#include <QtHttpServer/qabstracthttpserver.h>
#include <QtHttpServer/qhttpserverrouter.h>
#include <QtHttpServer/qhttpserverrouterrule.h>
#include <QtHttpServer/qhttpserverresponse.h>
#include <QtHttpServer/qhttpserverrouterviewtraits.h>
#include <QtHttpServer/qhttpserverviewtraits.h>

#if QT_CONFIG(future)
#  include <QtCore/qfuture.h>
#endif

#include <functional>
#include <tuple>

QT_BEGIN_NAMESPACE

class QHttpServerRequest;

class QHttpServerPrivate;
class Q_HTTPSERVER_EXPORT QHttpServer final : public QAbstractHttpServer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QHttpServer)

    template <typename...>
    static constexpr bool dependent_false_v = false;

    template<typename T>
    using ResponseType =
#if QT_CONFIG(future)
        std::conditional_t<
            std::is_same_v<QFuture<QHttpServerResponse>, T>,
            QFuture<QHttpServerResponse>,
            QHttpServerResponse
        >;
#else
        QHttpServerResponse;
#endif

public:
    explicit QHttpServer(QObject *parent = nullptr);
    ~QHttpServer() override;

    QHttpServerRouter *router();
    const QHttpServerRouter *router() const;

    template<typename Rule = QHttpServerRouterRule, typename ViewHandler>
    Rule *route(const QString &pathPattern, ViewHandler &&viewHandler)
    {
        using ViewTraits = QHttpServerRouterViewTraits<ViewHandler>;
        static_assert(ViewTraits::Arguments::StaticAssert,
                      "ViewHandler arguments are in the wrong order or not supported");
        return routeImpl<Rule, ViewHandler, ViewTraits>(pathPattern, std::forward<ViewHandler>(viewHandler));
    }

    template<typename Rule = QHttpServerRouterRule, typename ViewHandler>
    Rule *route(const QString &pathPattern, QHttpServerRequest::Methods method, ViewHandler &&viewHandler)
    {
        using ViewTraits = QHttpServerRouterViewTraits<ViewHandler>;
        static_assert(ViewTraits::Arguments::StaticAssert,
                      "ViewHandler arguments are in the wrong order or not supported");
        return routeImpl<Rule, ViewHandler, ViewTraits>(pathPattern, method, std::forward<ViewHandler>(viewHandler));
    }

    template<typename ViewHandler>
    void afterRequest(ViewHandler &&viewHandler)
    {
        using ViewTraits = QHttpServerAfterRequestViewTraits<ViewHandler>;
        static_assert(ViewTraits::Arguments::StaticAssert,
                      "ViewHandler arguments are in the wrong order or not supported");
        afterRequestHelper<ViewTraits, ViewHandler>(std::move(viewHandler));
    }

    using MissingHandler = std::function<void(const QHttpServerRequest &request,
                                              QHttpServerResponder &&responder)>;

    void setMissingHandler(MissingHandler handler);

private:
    using AfterRequestHandler =
        std::function<QHttpServerResponse(QHttpServerResponse &&response,
                      const QHttpServerRequest &request)>;

    template<typename ViewTraits, typename ViewHandler>
    void afterRequestHelper(ViewHandler &&viewHandler) {
        auto handler = [viewHandler](QHttpServerResponse &&resp,
                                     const QHttpServerRequest &request) {
            if constexpr (ViewTraits::Arguments::Last::IsRequest::Value) {
                if constexpr (ViewTraits::Arguments::Count == 2)
                    return viewHandler(std::move(resp), request);
                else
                    static_assert(dependent_false_v<ViewTraits>);
            } else if constexpr (ViewTraits::Arguments::Last::IsResponse::Value) {
                if constexpr (ViewTraits::Arguments::Count == 1)
                    return viewHandler(std::move(resp));
                else if constexpr (ViewTraits::Arguments::Count == 2)
                    return viewHandler(request, std::move(resp));
                else
                    static_assert(dependent_false_v<ViewTraits>);
            } else {
                static_assert(dependent_false_v<ViewTraits>);
            }
        };

        afterRequestImpl(std::move(handler));
    }

    void afterRequestImpl(AfterRequestHandler afterRequestHandler);

    template<typename ViewHandler, typename ViewTraits>
    auto createRouteHandler(ViewHandler &&viewHandler)
    {
        return [this, viewHandler = std::forward<ViewHandler>(viewHandler)](
                       const QRegularExpressionMatch &match,
                       const QHttpServerRequest &request,
                       QHttpServerResponder &&responder) {
            auto boundViewHandler = router()->bindCaptured(viewHandler, match);
            responseImpl<ViewTraits>(boundViewHandler, request, std::move(responder));
        };
    }

    template<typename Rule, typename ViewHandler, typename ViewTraits>
    Rule *routeImpl(const QString &pathPattern, ViewHandler &&viewHandler)
    {
        auto routerHandler = createRouteHandler<ViewHandler, ViewTraits>(std::forward<ViewHandler>(viewHandler));
        auto rule = std::make_unique<Rule>(pathPattern, std::move(routerHandler));
        return reinterpret_cast<Rule*>(router()->addRule<ViewHandler, ViewTraits>(std::move(rule)));
    }

    template<typename Rule, typename ViewHandler, typename ViewTraits>
    Rule *routeImpl(const QString &pathPattern, QHttpServerRequest::Methods method, ViewHandler &&viewHandler)
    {
        auto routerHandler = createRouteHandler<ViewHandler, ViewTraits>(std::forward<ViewHandler>(viewHandler));
        auto rule = std::make_unique<Rule>(pathPattern, method, std::move(routerHandler));
        return reinterpret_cast<Rule*>(router()->addRule<ViewHandler, ViewTraits>(std::move(rule)));
    }

    template<typename ViewTraits, typename T>
    void responseImpl(T &boundViewHandler, const QHttpServerRequest &request,
                      QHttpServerResponder &&responder)
    {
        if constexpr (ViewTraits::Arguments::PlaceholdersCount == 0) {
            ResponseType<typename ViewTraits::ReturnType> response(boundViewHandler());
            sendResponse(std::move(response), request, std::move(responder));
        } else if constexpr (ViewTraits::Arguments::PlaceholdersCount == 1) {
            if constexpr (ViewTraits::Arguments::Last::IsRequest::Value) {
                ResponseType<typename ViewTraits::ReturnType> response(boundViewHandler(request));
                sendResponse(std::move(response), request, std::move(responder));
            } else {
                static_assert(std::is_same_v<typename ViewTraits::ReturnType, void>,
                    "Handlers with responder argument must have void return type.");
                boundViewHandler(std::move(responder));
            }
        } else if constexpr (ViewTraits::Arguments::PlaceholdersCount == 2) {
            static_assert(std::is_same_v<typename ViewTraits::ReturnType, void>,
                "Handlers with responder argument must have void return type.");
            if constexpr (ViewTraits::Arguments::Last::IsRequest::Value) {
                boundViewHandler(std::move(responder), request);
            } else {
                boundViewHandler(request, std::move(responder));
            }
        } else {
            static_assert(dependent_false_v<ViewTraits>);
        }
    }

    bool handleRequest(const QHttpServerRequest &request,
                       QHttpServerResponder &responder) override;
    void missingHandler(const QHttpServerRequest &request,
                        QHttpServerResponder &&responder) override;

    void sendResponse(QHttpServerResponse &&response, const QHttpServerRequest &request,
                      QHttpServerResponder &&responder);

#if QT_CONFIG(future)
    void sendResponse(QFuture<QHttpServerResponse> &&response, const QHttpServerRequest &request,
                      QHttpServerResponder &&responder);
#endif
};

QT_END_NAMESPACE

#endif  // QHTTPSERVER_H
