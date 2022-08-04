// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore>
#include <QtHttpServer>

struct AddressEntry
{
    QString address;
    QString name;

    QJsonObject toJson(qint64 id) const
    {
        return QJsonObject{ { "id", id }, { "address", address }, { "name", name } };
    }

    static qint64 nextId();
};

qint64 AddressEntry::nextId()
{
    static qint64 lastId = 0;
    return lastId++;
}

static QJsonObject insertAddress(QMap<qint64, AddressEntry> &addresses, const QString &address,
                                 const QString &name)
{
    const auto entry = AddressEntry{ address, name };
    const auto it = addresses.insert(AddressEntry::nextId(), entry);
    return it->toJson(it.key());
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

    QMap<qint64, AddressEntry> addresses;
    // Setup QHttpServer
    QHttpServer httpServer;
    //! [GET example]
    httpServer.route("/v2/contact", QHttpServerRequest::Method::Get,
                     [&addresses](const QHttpServerRequest &request) {
                         QJsonArray array;
                         std::transform(addresses.constKeyValueBegin(),
                                        addresses.constKeyValueEnd(),
                                        std::inserter(array, array.begin()),
                                        [](const auto &it) { return it.second.toJson(it.first); });

                         return QHttpServerResponse(array);
                     });
    //! [GET example]

    httpServer.route("/v2/contact/<arg>", QHttpServerRequest::Method::Get,
                     [&addresses](qint64 contactId, const QHttpServerRequest &request) {
                         const auto address = addresses.find(contactId);
                         return address != addresses.end()
                                 ? QHttpServerResponse(address->toJson(address.key()))
                                 : QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
                     });

    //! [POST example]
    httpServer.route(
            "/v2/contact", QHttpServerRequest::Method::Post,
            [&addresses](const QHttpServerRequest &request) {
                const auto json = byteArrayToJsonObject(request.body());
                if (!json || !json->contains("address") || !json->contains("name"))
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

                const auto entry = insertAddress(addresses, json->value("address").toString(),
                                                 json->value("name").toString());
                return QHttpServerResponse(entry, QHttpServerResponder::StatusCode::Created);
            });
    //! [POST example]

    httpServer.route(
            "/v2/contact/<arg>", QHttpServerRequest::Method::Put,
            [&addresses](qint64 contactId, const QHttpServerRequest &request) {
                const auto json = byteArrayToJsonObject(request.body());
                if (!json || !json->contains("address") || !json->contains("name")) {
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
                }
                auto address = addresses.find(contactId);
                if (address == addresses.end())
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
                address->address = json->value("address").toString();
                address->name = json->value("name").toString();
                return QHttpServerResponse(address->toJson(address.key()));
            });

    httpServer.route(
            "/v2/contact/<arg>", QHttpServerRequest::Method::Patch,
            [&addresses](qint64 contactId, const QHttpServerRequest &request) {
                const auto json = byteArrayToJsonObject(request.body());
                if (!json) {
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
                }
                auto address = addresses.find(contactId);
                if (address == addresses.end())
                    return QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
                if (json->contains("address"))
                    address->address = json->value("address").toString();
                if (json->contains("name"))
                    address->name = json->value("name").toString();
                return QHttpServerResponse(address->toJson(address.key()));
            });

    httpServer.route("/v2/contact/<arg>", QHttpServerRequest::Method::Delete,
                     [&addresses](qint64 contactId, const QHttpServerRequest &request) {
                         if (!addresses.remove(contactId))
                             return QHttpServerResponse(
                                     QHttpServerResponder::StatusCode::NoContent);
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
