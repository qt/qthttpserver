// Copyright (C) 2019 Tasuku Suzuki <tasuku.suzuki@qbc.io>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtHttpServer/qhttpserverresponse.h>

#include <QtCore/qfile.h>
#include <QtTest/qtest.h>

#if QT_CONFIG(mimetype)
#include <QtCore/qmimedatabase.h>
#endif

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

    QTest::addRow("application/x-zerosize")
        << QFINDTESTDATA("data/empty");

    QTest::addRow("text/plain")
        << QFINDTESTDATA("data/text.plain");

    QTest::addRow("text/html")
        << QFINDTESTDATA("data/text.html");

    QTest::addRow("image/png")
        << QFINDTESTDATA("data/image.png");

    QTest::addRow("image/jpeg")
             << QFINDTESTDATA("data/image.jpeg");

    QTest::addRow("image/svg+xml")
             << QFINDTESTDATA("data/image.svg");
}

void tst_QHttpServerResponse::mimeTypeDetection()
{
#if !QT_CONFIG(mimetype)
    QSKIP("Test requires QMimeDatabase");
#else
    QFETCH(QString, content);

    QFile file(content);
    file.open(QFile::ReadOnly);
    QByteArray data = file.readAll();
    QHttpServerResponse response(data);
    file.close();

    const QMimeType mimeType = QMimeDatabase().mimeTypeForData(data);
    QCOMPARE(response.mimeType(), mimeType.name());
#endif
}

void tst_QHttpServerResponse::mimeTypeDetectionFromFile_data()
{
    QTest::addColumn<QString>("content");

    QTest::addRow("application/x-zerosize")
            << QFINDTESTDATA("data/empty");

    QTest::addRow("text/plain")
            << QFINDTESTDATA("data/text.plain");

    QTest::addRow("text/html")
            << QFINDTESTDATA("data/text.html");

    QTest::addRow("image/png")
            << QFINDTESTDATA("data/image.png");

    QTest::addRow("image/jpeg")
            << QFINDTESTDATA("data/image.jpeg");

    QTest::addRow("image/svg+xml")
            << QFINDTESTDATA("data/image.svg");

    QTest::addRow("application/json")
            << QFINDTESTDATA("data/application.json");
}

void tst_QHttpServerResponse::mimeTypeDetectionFromFile()
{
#if !QT_CONFIG(mimetype)
    QSKIP("Test requires QMimeDatabase");
#else
    QFETCH(QString, content);
    const QMimeType mimeType = QMimeDatabase().mimeTypeForFile(content);

    const QByteArray responseMimeType = QHttpServerResponse::fromFile(content).mimeType();
    QCOMPARE(responseMimeType, mimeType.name());
#endif
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
