// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qhttpserverconfiguration.h"

QT_BEGIN_NAMESPACE

/*!
    \class QHttpServerConfiguration
    \since 6.9
    \inmodule QtHttpServer
    \brief The QHttpServerConfiguration class controls server parameters.
*/
class QHttpServerConfigurationPrivate : public QSharedData
{
public:
    quint32 rateLimit = 0;
    QList<QPair<QHostAddress, int>> whitelist;
    QList<QPair<QHostAddress, int>> blacklist;
};

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QHttpServerConfigurationPrivate)

/*!
    Default constructs a QHttpServerConfiguration object.

    Such a configuration has the following values:
     \list
         \li Rate limit is disabled
     \endlist
*/
QHttpServerConfiguration::QHttpServerConfiguration()
    : d(new QHttpServerConfigurationPrivate)
{
}

/*!
    Copy-constructs this QHttpServerConfiguration.
*/
QHttpServerConfiguration::QHttpServerConfiguration(const QHttpServerConfiguration &) = default;

/*!
    \fn QHttpServerConfiguration::QHttpServerConfiguration(QHttpServerConfiguration &&other) noexcept

    Move-constructs this QHttpServerConfiguration from \a other
*/

/*!
    Copy-assigns \a other to this QHttpServerConfiguration.
*/
QHttpServerConfiguration &QHttpServerConfiguration::operator=(const QHttpServerConfiguration &) = default;

/*!
    \fn QHttpServerConfiguration &QHttpServerConfiguration::operator=(QHttpServerConfiguration &&other) noexcept

    Move-assigns \a other to this QHttpServerConfiguration.
*/

/*!
    Destructor.
*/
QHttpServerConfiguration::~QHttpServerConfiguration()
    = default;

/*!
    Sets \a maxRequests as the maximum number of incoming requests
    per second per IP that will be accepted by QHttpServer.
    If the limit is exceeded, QHttpServer will respond with
    QHttpServerResponder::StatusCode::TooManyRequests.

    \sa rateLimitPerSecond(), QHttpServerResponder::StatusCode
*/
void QHttpServerConfiguration::setRateLimitPerSecond(quint32 maxRequests)
{
    d.detach();
    d->rateLimit = maxRequests;
}

/*!
    Returns maximum number of incoming requests per second per IP
    accepted by the server.

    \sa setRateLimitPerSecond()
*/
quint32 QHttpServerConfiguration::rateLimitPerSecond() const
{
    return d->rateLimit;
}

/*!
    Sets \a subnetList as the whitelist of allowed subnets.

    When the list is not empty, only IP addresses in this list
    will be allowed by QHttpServer. The whitelist takes priority
    over the blacklist.

    Each subnet is represented as a pair consisting of:
    \list
      \li A base IP address of type QHostAddress.
      \li A CIDR prefix length of type int, which defines the subnet mask.
    \endlist

    To allow only a specific IP address, use a prefix length of 32 for IPv4
    (e.g., "192.168.1.100/32") or 128 for IPv6 (e.g., "2001:db8::1/128").

    \sa whitelist(), setBlacklist(), QHostAddress::parseSubnet()
*/
void QHttpServerConfiguration::setWhitelist(const QList<std::pair<QHostAddress, int>> &subnetList)
{
    d.detach();
    d->whitelist = subnetList;
}

/*!
    Returns the whitelist of subnets allowed by QHttpServer.

    \sa setWhitelist()
*/
QList<std::pair<QHostAddress, int>> QHttpServerConfiguration::whitelist() const
{
    return d->whitelist;
}

/*!
    Sets \a subnetList as the blacklist of subnets.

    IP addresses in this list will be denied access by QHttpServer.
    The blacklist is active only when the whitelist is empty.

    \sa blacklist(), setWhitelist(), QHostAddress::parseSubnet()
*/
void QHttpServerConfiguration::setBlacklist(const QList<std::pair<QHostAddress, int>> &subnetList)
{
    d.detach();
    d->blacklist = subnetList;
}

/*!
    Returns the blacklist of subnets that are denied access by QHttpServer.

    \sa setBlacklist()
*/
QList<std::pair<QHostAddress, int>> QHttpServerConfiguration::blacklist() const
{
    return d->blacklist;
}

/*!
    \fn void QHttpServerConfiguration::swap(QHttpServerConfiguration &other)
    \memberswap{configuration}
*/

/*!
    \fn bool QHttpServerConfiguration::operator==(const QHttpServerConfiguration &lhs, const QHttpServerConfiguration &rhs) noexcept
    Returns \c true if \a lhs and \a rhs have the same set of configuration
    parameters.
*/

/*!
    \fn bool QHttpServerConfiguration::operator!=(const QHttpServerConfiguration &lhs, const QHttpServerConfiguration &rhs) noexcept
    Returns \c true if \a lhs and \a rhs do not have the same set of configuration
    parameters.
*/

/*!
    \internal
*/
bool comparesEqual(const QHttpServerConfiguration &lhs, const QHttpServerConfiguration &rhs) noexcept
{
    if (lhs.d == rhs.d)
        return true;

    return lhs.d->rateLimit == rhs.d->rateLimit;
}

QT_END_NAMESPACE
