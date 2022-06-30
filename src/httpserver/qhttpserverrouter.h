// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QHTTPSERVERROUTER_H
#define QHTTPSERVERROUTER_H

#include <QtHttpServer/qthttpserverglobal.h>
#include <QtHttpServer/qhttpserverrouterviewtraits.h>

#include <QtCore/qscopedpointer.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qregularexpression.h>

#include <functional>
#include <initializer_list>

QT_BEGIN_NAMESPACE

namespace QtPrivate {
    template<int> struct QHttpServerRouterPlaceholder {};
}

QT_END_NAMESPACE

namespace std {

template<int N>
struct is_placeholder<QT_PREPEND_NAMESPACE(QtPrivate::QHttpServerRouterPlaceholder<N>)> :
    integral_constant<int , N + 1>
{};

}

QT_BEGIN_NAMESPACE

class QTcpSocket;
class QHttpServerRequest;
class QHttpServerRouterRule;

class QHttpServerRouterPrivate;
class Q_HTTPSERVER_EXPORT QHttpServerRouter
{
    Q_DECLARE_PRIVATE(QHttpServerRouter)

public:
    QHttpServerRouter();
    ~QHttpServerRouter();

    template<typename Type>
    bool addConverter(QLatin1StringView regexp) {
        static_assert(QMetaTypeId2<Type>::Defined,
                      "Type is not registered with Qt's meta-object system: "
                      "please apply Q_DECLARE_METATYPE() to it");

        if (!QMetaType::registerConverter<QString, Type>())
            return false;

        addConverter(QMetaType::fromType<Type>(), regexp);
        return true;
    }

    void addConverter(QMetaType metaType, QLatin1StringView regexp);
    void removeConverter(QMetaType metaType);
    void clearConverters();
    const QHash<QMetaType, QLatin1StringView> &converters() const;

    template<typename ViewHandler, typename ViewTraits = QHttpServerRouterViewTraits<ViewHandler>>
    bool addRule(std::unique_ptr<QHttpServerRouterRule> rule)
    {
        return addRuleHelper<ViewTraits>(
                std::move(rule),
                typename ViewTraits::Arguments::Indexes{});
    }

    template<typename ViewHandler, typename ViewTraits = QHttpServerRouterViewTraits<ViewHandler>>
    typename ViewTraits::BindableType bindCaptured(ViewHandler &&handler,
                      const QRegularExpressionMatch &match) const
    {
        return bindCapturedImpl<ViewHandler, ViewTraits>(
                std::forward<ViewHandler>(handler),
                match,
                typename ViewTraits::Arguments::CapturableIndexes{},
                typename ViewTraits::Arguments::PlaceholdersIndexes{});
    }

    bool handleRequest(const QHttpServerRequest &request,
                       QTcpSocket *socket) const;

private:
    template<typename ViewTraits, int ... Idx>
    bool addRuleHelper(std::unique_ptr<QHttpServerRouterRule> rule,
                       QtPrivate::IndexesList<Idx...>)
    {
        return addRuleImpl(std::move(rule), {ViewTraits::Arguments::template metaType<Idx>()...});
    }

    bool addRuleImpl(std::unique_ptr<QHttpServerRouterRule> rule,
                     std::initializer_list<QMetaType> metaTypes);

    template<typename ViewHandler, typename ViewTraits, int ... Cx, int ... Px>
    typename ViewTraits::BindableType
            bindCapturedImpl(ViewHandler &&handler,
                          const QRegularExpressionMatch &match,
                          QtPrivate::IndexesList<Cx...>,
                          QtPrivate::IndexesList<Px...>) const
    {
        return std::bind(
            std::forward<ViewHandler>(handler),
            QVariant(match.captured(Cx + 1))
                .value<typename ViewTraits::Arguments::template Arg<Cx>::CleanType>()...,
            QtPrivate::QHttpServerRouterPlaceholder<Px>{}...);
    }

    QScopedPointer<QHttpServerRouterPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QHTTPSERVERROUTER_H
