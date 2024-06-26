// Copyright (C) 2019 Tasuku Suzuki <tasuku.suzuki@qbc.io>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtHttpServer/qhttpserverresponse.h>

#include <QtCore/qfile.h>
#include <QtTest/qtest.h>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals;

class tst_QHttpServerResponse : public QObject
{
    Q_OBJECT

private slots:
    void mimeTypeDetection_data();
    void mimeTypeDetection();
    void mimeTypeDetectionFromFile_data();
    void mimeTypeDetectionFromFile();
    void headers();
};

void tst_QHttpServerResponse::mimeTypeDetection_data()
{
    QTest::addColumn<QString>("content");
    QTest::addColumn<QByteArray>("mimeType");

    QTest::addRow("application/x-zerosize")
        << QFINDTESTDATA("data/empty")
        << "application/x-zerosize"_ba;

    QTest::addRow("text/plain")
        << QFINDTESTDATA("data/text.plain")
        << "text/plain"_ba;

    QTest::addRow("text/html")
        << QFINDTESTDATA("data/text.html")
        << "text/html"_ba;

    QTest::addRow("image/png")
        << QFINDTESTDATA("data/image.png")
        << "image/png"_ba;

    QTest::addRow("image/jpeg")
             << QFINDTESTDATA("data/image.jpeg")
             << "image/jpeg"_ba;

    QTest::addRow("image/svg+xml")
             << QFINDTESTDATA("data/image.svg")
             << "image/svg+xml"_ba;
}

void tst_QHttpServerResponse::mimeTypeDetection()
{
    QFETCH(QString, content);
    QFETCH(QByteArray, mimeType);

    QFile file(content);
    file.open(QFile::ReadOnly);
    QHttpServerResponse response(file.readAll());
    file.close();

    QCOMPARE(response.mimeType(), mimeType);
}

void tst_QHttpServerResponse::mimeTypeDetectionFromFile_data()
{
    QTest::addColumn<QString>("content");
    QTest::addColumn<QByteArray>("mimeType");

    QTest::addRow("application/x-zerosize")
            << QFINDTESTDATA("data/empty")
            << "application/x-zerosize"_ba;

    QTest::addRow("text/plain")
            << QFINDTESTDATA("data/text.plain")
            << "text/plain"_ba;

    QTest::addRow("text/html")
            << QFINDTESTDATA("data/text.html")
            << "text/html"_ba;

    QTest::addRow("image/png")
            << QFINDTESTDATA("data/image.png")
            << "image/png"_ba;

    QTest::addRow("image/jpeg")
            << QFINDTESTDATA("data/image.jpeg")
            << "image/jpeg"_ba;

    QTest::addRow("image/svg+xml")
            << QFINDTESTDATA("data/image.svg")
            << "image/svg+xml"_ba;

    QTest::addRow("application/json")
            << QFINDTESTDATA("data/application.json")
            << "application/json"_ba;
}

void tst_QHttpServerResponse::mimeTypeDetectionFromFile()
{
    QFETCH(QString, content);
    QFETCH(QByteArray, mimeType);

    QCOMPARE(QHttpServerResponse::fromFile(content).mimeType(), mimeType);
}

void tst_QHttpServerResponse::headers()
{
    QHttpServerResponse resp("");

    const QByteArray test1 = "test1"_ba;
    const QByteArray test2 = "test2"_ba;
    const QByteArray zero = "application/x-zerosize"_ba;
    const auto contentTypeHeader = "Content-Type"_ba;
    const auto contentLengthHeader = "Content-Length"_ba;

    QVERIFY(!resp.hasHeader(contentLengthHeader));
    QVERIFY(resp.hasHeader(contentTypeHeader, zero));
    QVERIFY(!resp.hasHeader(contentTypeHeader, test1));
    QVERIFY(!resp.hasHeader(contentTypeHeader, test2));

    QHttpHeaders h = resp.headers();
    h.append(contentTypeHeader, test1);
    h.append(contentLengthHeader, test2);
    resp.withHeaders(h);

    QVERIFY(resp.hasHeader(contentLengthHeader, test2));
    QVERIFY(resp.hasHeader(contentTypeHeader, zero));
    QVERIFY(resp.hasHeader(contentTypeHeader, test1));
    QVERIFY(!resp.hasHeader(contentTypeHeader, test2));

    const auto &typeHeaders = resp.headerData(contentTypeHeader);
    QCOMPARE(typeHeaders.size(), 2);
    QVERIFY(typeHeaders.contains(zero));
    QVERIFY(typeHeaders.contains(test1));

    const auto &lengthHeaders = resp.headerData(contentLengthHeader);
    QCOMPARE(lengthHeaders.size(), 1);
    QVERIFY(lengthHeaders.contains(test2));

    h.removeAll(contentTypeHeader);
    h.append(contentTypeHeader, test2);
    resp.withHeaders(h);

    QVERIFY(resp.hasHeader(contentLengthHeader, test2));
    QVERIFY(!resp.hasHeader(contentTypeHeader, zero));
    QVERIFY(!resp.hasHeader(contentTypeHeader, test1));
    QVERIFY(resp.hasHeader(contentTypeHeader, test2));
}

QT_END_NAMESPACE

QTEST_MAIN(tst_QHttpServerResponse)

#include "tst_qhttpserverresponse.moc"
