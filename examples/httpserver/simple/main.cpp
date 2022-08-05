// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore>
#include <QtHttpServer>

static inline QString host(const QHttpServerRequest &request)
{
    return QString::fromLatin1(request.value("Host"));
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QHttpServer httpServer;
    httpServer.route("/", []() {
        return "Hello world";
    });

    httpServer.route("/query", [] (const QHttpServerRequest &request) {
        return QString("%1/query/").arg(host(request));
    });

    httpServer.route("/query/", [] (qint32 id, const QHttpServerRequest &request) {
        return QString("%1/query/%2").arg(host(request)).arg(id);
    });

    httpServer.route("/query/<arg>/log", [] (qint32 id, const QHttpServerRequest &request) {
        return QString("%1/query/%2/log").arg(host(request)).arg(id);
    });

    httpServer.route("/query/<arg>/log/", [] (qint32 id, float threshold,
                                              const QHttpServerRequest &request) {
        return QString("%1/query/%2/log/%3").arg(host(request)).arg(id).arg(threshold);
    });

    httpServer.route("/user/", [] (const qint32 id) {
        return QString("User %1").arg(id);
    });

    httpServer.route("/user/<arg>/detail", [] (const qint32 id) {
        return QString("User %1 detail").arg(id);
    });

    httpServer.route("/user/<arg>/detail/", [] (const qint32 id, const qint32 year) {
        return QString("User %1 detail year - %2").arg(id).arg(year);
    });

    httpServer.route("/json/", [] {
        return QJsonObject{
            {
                {"key1", "1"},
                {"key2", "2"},
                {"key3", "3"}
            }
        };
    });

    httpServer.route("/assets/<arg>", [] (const QUrl &url) {
        return QHttpServerResponse::fromFile(QStringLiteral(":/assets/%1").arg(url.path()));
    });

    httpServer.route("/remote_address", [](const QHttpServerRequest &request) {
        return request.remoteAddress().toString();
    });

    // Basic authentication example (RFC 7617)
    httpServer.route("/auth", [](const QHttpServerRequest &request) {
        auto auth = request.value("authorization").simplified();

        if (auth.size() > 6 && auth.first(6).toLower() == "basic ") {
            auto token = auth.sliced(6);
            auto userPass = QByteArray::fromBase64(token);

            if (auto colon = userPass.indexOf(':'); colon > 0) {
                auto userId = userPass.first(colon);
                auto password = userPass.sliced(colon + 1);

                if (userId == "Aladdin" && password == "open sesame")
                    return QHttpServerResponse("text/plain", "Success\n");
            }
        }
        QHttpServerResponse response("text/plain", "Authentication required\n",
                                     QHttpServerResponse::StatusCode::Unauthorized);
        response.setHeader("WWW-Authenticate", R"(Basic realm="Simple example", charset="UTF-8")");
        return response;
    });

    const auto port = httpServer.listen(QHostAddress::Any);
    if (!port) {
        qDebug() << QCoreApplication::translate("QHttpServerExample",
                                                "Server failed to listen on a port.");
        return 0;
    }

    //! [HTTPS Configuration example]
    const auto sslCertificateChain =
            QSslCertificate::fromPath(QStringLiteral(":/assets/certificate.crt"));
    if (sslCertificateChain.empty()) {
        qDebug() << QCoreApplication::translate("QHttpServerExample",
                                                "Couldn't retrive SSL certificate from file.");
        return 0;
    }
    QFile privateKeyFile(QStringLiteral(":/assets/private.key"));
    if (!privateKeyFile.open(QIODevice::ReadOnly)) {
        qDebug() << QCoreApplication::translate("QHttpServerExample",
                                                "Couldn't open file for reading.");
        return 0;
    }
    httpServer.sslSetup(sslCertificateChain.front(), QSslKey(&privateKeyFile, QSsl::Rsa));
    privateKeyFile.close();

    const auto sslPort = httpServer.listen(QHostAddress::Any);
    if (!sslPort) {
        qDebug() << QCoreApplication::translate("QHttpServerExample",
                                                "Server failed to listen on a port.");
        return 0;
    }
    //! [HTTPS Configuration example]

    qDebug() << QCoreApplication::translate("QHttpServerExample",
                                            "Running on http://127.0.0.1:%1/ and "
                                            "https://127.0.0.1:%2/ (Press CTRL+C to quit)")
                        .arg(port)
                        .arg(sslPort);

    return app.exec();
}
