#include "mainwindow.h"
#include <iostream>
#include <QDebug>
#include <QApplication>
#include <QTextCodec>
#include <QProcess>
#include <QFileInfo>
#include <QMessageBox>
#include <QDateTime>

#include "version.h"
#include "installwizard.h"
#include "launchframe.h"

extern QString WORKDIR;

#include "datathread.h"
#include "qrcode/QrCode.hpp"

#define VERSION "0.1"


extern QString REDISSERVER; // "127.0.0.1"
extern int REDISPORT; // 6379
extern QString REDISAUTH; //
extern int gRID;

extern QString LOCALWEBSERVER; // "https://cnshop.aiwaiter.net:443"
extern QApplication* gApp;


int launchMainWnd(int argc, char *argv[]) {
    QApplication app(argc, argv);

    gApp = &app;
//    int ret = app.exec();

    static LaunchFrame lf;
    lf.setMinimumSize(800, 600);
    lf.show();

    return app.exec();
}


int main(int argc, char *argv[])
{
    if(argc > 1) {
        if("--version" == QString(argv[1])) {
            std::cout << AMPVersion.toStdString().c_str() << "."
                      << AMPBuild.toStdString().c_str()
                      << std::endl;
            return 0;
        }
    }
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    WORKDIR = "/opt/pos";
#ifdef MPOS_CLOUD

#ifdef Q_OS_ANDROID
    WORKDIR = "/sdcard";
#elif defined(Q_OS_WIN32)
    WORKDIR = "c:/pos";
#else
    WORKDIR = "/opt/pos";
#endif

#endif


    QDir dir(QString("%1/plugins").arg(WORKDIR));
    if(dir.exists() == false) {
        dir.mkdir(QString("%1/plugins").arg(WORKDIR));
    }

    char* env = (char*)QString("QT_QPA_PLATFORM_PLUGIN_PATH=%1/plugins/platforms").arg(WORKDIR).toStdString().c_str();
    ::putenv(env);

#ifdef Q_OS_WIN32
    QApplication::addLibraryPath( QString("%1").arg(WORKDIR) ); //"/opt/bin/plugins");
#else
    QApplication::addLibraryPath( QString("%1/plugins").arg(WORKDIR) ); //"/opt/bin/plugins");
#endif


    QString installflag = QString("%1/%2").arg(WORKDIR).arg(".installed");

    qDebug() << "install path: " << installflag;

    QFileInfo fi( installflag );

    if(! fi.isFile() ) {

        QApplication app(argc, argv);

        InstallWizard w;

        w.setMinimumSize(400, 300);
        w.setMaximumSize(1024, 768);
        w.showFullScreen();

        int ret = app.exec();
        if(ret != 0) {
            QMessageBox::warning(nullptr, QObject::tr("出错"), QObject::tr("安装过程中已被用户取消"));
            return ret;
        }

#ifndef MPOS_CLOUD
        QFile file(installflag);
        file.open(QIODevice::WriteOnly);
        file.write( QDateTime::currentDateTime().toString().toLocal8Bit());
        file.close();
#endif
    }

    QFile file(installflag);
    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);
    while(!stream.atEnd()) {

        QString line = stream.readLine();
        line = line.trimmed();
        if(line.startsWith("redis-server=")) {
            REDISSERVER = line.mid( QString("redis-server=").length() );

        } else if(line.startsWith("redis-port=")) {
            REDISPORT = line.mid( QString("redis-port=").length() ).toInt();


        } else if(line.startsWith("redis-auth=")) {
            REDISAUTH = line.mid( QString("redis-auth=").length() );

        } else if(line.startsWith("local-web-server=")) {
            LOCALWEBSERVER = line.mid( QString("local-web-server=").length() );

        } else if(line.startsWith("shop-id=")) {
            gRID = line.mid( QString("shop-id=").length() ).toInt();
        }
    }

    qDebug() << "redis-server: " << REDISSERVER
             << "redis-port: " << REDISPORT
             << "redis-auth: " << REDISAUTH
             << "local-web-server: " << LOCALWEBSERVER;

    return launchMainWnd(argc, argv);
}
