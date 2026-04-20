#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QQmlContext>   // qmlRegisterType объявлено здесь
#include "calculator.h"    // класс Calculator


static void loadAppFonts()
{
    const QString resPath = QStringLiteral(":/OpenSans.ttf");

    QFile f(resPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Font resource NOT found:" << resPath;
        return;
    }

    QByteArray fontData = f.readAll();
    f.close();

    int id = QFontDatabase::addApplicationFontFromData(fontData);
    if (id == -1) {
        qWarning() << "Font NOT loaded from data.";
    } else {
        qDebug() << "Font loaded OK, families:" << QFontDatabase::applicationFontFamilies(id);
    }
}


int main(int argc, char *argv[])
{

    QGuiApplication app(argc, argv);

    // проверяем, что ресурс виден
    QFile f(":/fonts/OpenSans.ttf");
    qDebug() << "exists:" << f.exists() << "size:" << (f.open(QIODevice::ReadOnly)?f.size():-1);

    // регистрируем шрифт
    int id = QFontDatabase::addApplicationFont(":/fonts/OpenSans.ttf");
    qDebug() << "font id:" << id;

    // регистрируем C++-тип для QML
    qmlRegisterType<Calculator>("Calc", 1, 0, "Calculator");

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/calc2026/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
