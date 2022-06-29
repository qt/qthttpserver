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

#include <tuple>

QT_BEGIN_NAMESPACE

class QTcpSocket;
class QHttpServerRequest;

class QHttpServerPrivate;
class Q_HTTPSERVER_EXPORT QHttpServer final : public QAbstractHttpServer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QHttpServer)

    template<int I, typename ... Ts>
    struct VariadicTypeAt { using Type = typename std::tuple_element<I, std::tuple<Ts...>>::type; };

    template<typename ... Ts>
    struct VariadicTypeLast {
        using Type = typename VariadicTypeAt<sizeof ... (Ts) - 1, Ts...>::Type;
    };

    template <typename...>
    static constexpr bool dependent_false_v = false;

    template<typename T>
    using ResponseType =
        typename std::conditional<
            std::is_base_of<QHttpServerResponse, T>::value,
            T,
            QHttpServerResponse
        >::type;

public:
    explicit QHttpServer(QObject *parent = nullptr);
    ~QHttpServer();

    QHttpServerRouter *router();

    template<typename Rule = QHttpServerRouterRule, typename ... Args>
    bool route(Args && ... args)
    {
        using ViewHandler = typename VariadicTypeLast<Args...>::Type;
        using ViewTraits = QHttpServerRouterViewTraits<ViewHandler>;
        static_assert(ViewTraits::Arguments::StaticAssert,
                      "ViewHandler arguments are in the wrong order or not supported");
        return routeHelper<Rule, ViewHandler, ViewTraits>(
                QtPrivate::makeIndexSequence<sizeof ... (Args) - 1>{},
                std::forward<Args>(args)...);
    }

    template<typename ViewHandler>
    void afterRequest(ViewHandler &&viewHandler)
    {
        using ViewTraits = QHttpServerAfterRequestViewTraits<ViewHandler>;
        static_assert(ViewTraits::Arguments::StaticAssert,
                      "ViewHandler arguments are in the wrong order or not supported");
        afterRequestHelper<ViewTraits, ViewHandler>(std::move(viewHandler));
    }

    using AfterRequestHandler =
        std::function<QHttpServerResponse(QHttpServerResponse &&response,
                      const QHttpServerRequest &request)>;
private:
    template<typename ViewTraits, typename ViewHandler>
    void afterRequestHelper(ViewHandler &&viewHandler) {
        auto handler = [viewHandler](QHttpServerResponse &&resp,
                                     const QHttpServerRequest &request) {
            if constexpr (ViewTraits::Arguments::Last::IsRequest::Value) {
                if constexpr (ViewTraits::Arguments::Count == 2)
                    return std::move(viewHandler(std::move(resp), request));
                else
                    static_assert(dependent_false_v<ViewTraits>);
            } else if constexpr (ViewTraits::Arguments::Last::IsResponse::Value) {
                if constexpr (ViewTraits::Arguments::Count == 1)
                    return std::move(viewHandler(std::move(resp)));
                else if constexpr (ViewTraits::Arguments::Count == 2)
                    return std::move(viewHandler(request, std::move(resp)));
                else
                    static_assert(dependent_false_v<ViewTraits>);
            } else {
                static_assert(dependent_false_v<ViewTraits>);
            }
        };

        afterRequestImpl(std::move(handler));
    }

    void afterRequestImpl(AfterRequestHandler afterRequestHandler);

private:
    template<typename Rule, typename ViewHandler, typename ViewTraits, int ... I, typename ... Args>
    bool routeHelper(QtPrivate::IndexesList<I...>, Args &&... args)
    {
        return routeImpl<Rule,
                         ViewHandler,
                         ViewTraits,
                         typename VariadicTypeAt<I, Args...>::Type...>(std::forward<Args>(args)...);
    }

    template<typename Rule, typename ViewHandler, typename ViewTraits, typename ... Args>
    bool routeImpl(Args &&...args, ViewHandler &&viewHandler)
    {
        auto routerHandler = [this, viewHandler = std::forward<ViewHandler>(viewHandler)] (
                    const QRegularExpressionMatch &match,
                    const QHttpServerRequest &request,
                    QTcpSocket *socket) mutable {
            auto boundViewHandler = router()->bindCaptured<ViewHandler, ViewTraits>(
                    std::move(viewHandler), match);
            responseImpl<ViewTraits>(boundViewHandler, request, socket);
        };

        return router()->addRule<ViewHandler, ViewTraits>(
                new Rule(std::forward<Args>(args)..., std::move(routerHandler)));
    }

    template<typename ViewTraits, typename T>
    void responseImpl(T &boundViewHandler, const QHttpServerRequest &request, QTcpSocket *socket)
    {
        if constexpr (ViewTraits::Arguments::PlaceholdersCount == 0) {
            ResponseType<typename ViewTraits::ReturnType> response(boundViewHandler());
            sendResponse(std::move(response), request, socket);
        } else if constexpr (ViewTraits::Arguments::PlaceholdersCount == 1) {
            if constexpr (ViewTraits::Arguments::Last::IsRequest::Value) {
                ResponseType<typename ViewTraits::ReturnType> response(boundViewHandler(request));
                sendResponse(std::move(response), request, socket);
            } else {
                boundViewHandler(makeResponder(request, socket));
            }
        } else if constexpr (ViewTraits::Arguments::PlaceholdersCount == 2) {
            if constexpr (ViewTraits::Arguments::Last::IsRequest::Value) {
                boundViewHandler(makeResponder(request, socket), request);
            } else {
                boundViewHandler(request, makeResponder(request, socket));
            }
        } else {
            static_assert(dependent_false_v<ViewTraits>);
        }
    }

    bool handleRequest(const QHttpServerRequest &request, QTcpSocket *socket) override final;

    void sendResponse(QHttpServerResponse &&response,
                      const QHttpServerRequest &request,
                      QTcpSocket *socket);
};

QT_END_NAMESPACE

#endif  // QHTTPSERVER_H
