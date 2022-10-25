// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore>
#include <QtHttpServer>

#define API_KEY "SecretKey"

struct ContactEntry
{
    qint64 id;
    QString name;
    QString address;

    ContactEntry(const QString &name, const QString &address)
        : id(ContactEntry::nextId()), name(name), address(address)
    {
    }

    QJsonObject toJson() const
    {
        return QJsonObject{ { "id", id }, { "name", name }, { "address", address } };
    }

private:
    static qint64 nextId();
};

qint64 ContactEntry::nextId()
{
    static qint64 lastId = 0;
    return lastId++;
}

static bool checkApiKeyHeader(const QList<QPair<QByteArray, QByteArray>> &headers)
{
    for (const auto &[key, value] : headers) {
        if (key == "api_key" && value == API_KEY) {
            return true;
        }
    }
    return false;
}

static QJsonObject insertAddress(QMap<qint64, ContactEntry> &contacts, const QString &name,
                                 const QString &address)
{
    ContactEntry entry(name, address);
    const auto it = contacts.insert(entry.id, std::move(entry));
    return it->toJson();
}

static std::optional<QJsonObject> byteArrayToJsonObject(const QByteArray &arr)
{
    QJsonParseError err;
    const auto json = QJsonDocument::fromJson(arr, &err);
    if (err.error || !json.isObject())
        return std::nullopt;
    return json.object();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QMap<qint64, ContactEntry> contacts;
    // Setup QHttpServer
    QHttpServer httpServer;
    //! [GET example]
    httpServer.route("/v2/contact", QHttpServerRequest::Method::Get,
                     [&contacts](const QHttpServerRequest &) {
                         QJsonArray array;
                         std::transform(contacts.cbegin(), contacts.cend(),
                                        std::inserter(array, array.begin()),
                                        [](const auto &it) { return it.toJson(); });

                         return QHttpServerResponse(array);
                     });
    //! [GET example]

    httpServer.route("/v2/contact/<arg>", QHttpServerRequest::Method::Get,
                     [&contacts](qint64 contactId, const QHttpServerRequest &) {
                         const auto address = contacts.find(contactId);
                         return address != contacts.end()
                                 ? QHttpServerResponse(address->toJson())
                                 : QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
                     });

    //! [POST example]
    httpServer.route(
            "/v2/contact", QHttpServerRequest::Method::Post,
            [&contacts](const QHttpServerRequest &request) {
                if (!checkApiKeyHeader(request.headers())) {
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::Unauthorized);
                }
                const auto json = byteArrayToJsonObject(request.body());
                if (!json || !json->contains("address") || !json->contains("name"))
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
                const auto entry = insertAddress(contacts, json->value("name").toString(),
                                                 json->value("address").toString());
                return QHttpServerResponse(entry, QHttpServerResponder::StatusCode::Created);
            });
    //! [POST example]

    httpServer.route(
            "/v2/contact/<arg>", QHttpServerRequest::Method::Put,
            [&contacts](qint64 contactId, const QHttpServerRequest &request) {
                if (!checkApiKeyHeader(request.headers())) {
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::Unauthorized);
                }
                const auto json = byteArrayToJsonObject(request.body());
                if (!json || !json->contains("address") || !json->contains("name")) {
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
                }
                auto address = contacts.find(contactId);
                if (address == contacts.end())
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
                address->name = json->value("name").toString();
                address->address = json->value("address").toString();
                return QHttpServerResponse(address->toJson());
            });

    httpServer.route(
            "/v2/contact/<arg>", QHttpServerRequest::Method::Patch,
            [&contacts](qint64 contactId, const QHttpServerRequest &request) {
                if (!checkApiKeyHeader(request.headers())) {
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::Unauthorized);
                }
                const auto json = byteArrayToJsonObject(request.body());
                if (!json) {
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
                }
                auto address = contacts.find(contactId);
                if (address == contacts.end())
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
                if (json->contains("name"))
                    address->name = json->value("name").toString();
                if (json->contains("address"))
                    address->address = json->value("address").toString();
                return QHttpServerResponse(address->toJson());
            });

    httpServer.route(
            "/v2/contact/<arg>", QHttpServerRequest::Method::Delete,
            [&contacts](qint64 contactId, const QHttpServerRequest &request) {
                if (!checkApiKeyHeader(request.headers())) {
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::Unauthorized);
                }
                if (!contacts.remove(contactId))
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
                return QHttpServerResponse(QHttpServerResponder::StatusCode::Ok);
            });

    const auto port = httpServer.listen(QHostAddress::Any);
    if (!port) {
        qDebug() << QCoreApplication::translate("QHttpServerExample",
                                                "Server failed to listen on a port.");
        return 0;
    }

    qDebug() << QCoreApplication::translate(
                        "QHttpServerExample",
                        "Running on http://127.0.0.1:%1/ (Press CTRL+C to quit)")
                        .arg(port);

    return app.exec();
}
