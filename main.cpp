#include "mainwindow.h"
#include "getstarteddlg.h"
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>
#include <QString>
#include <QSplashScreen>
#include <QTimer>

//#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //qDebug() << QStyleFactory::keys();

    QPalette pal = a.palette();

    pal.setColor(QPalette::Window, QColor(255, 243, 212));
    pal.setColor(QPalette::WindowText, Qt::black);
    pal.setColor(QPalette::Base, QColor(240, 226, 192));
    pal.setColor(QPalette::Text, Qt::black);
    pal.setColor(QPalette::Button, QColor(210, 180, 140));
    pal.setColor(QPalette::ButtonText, Qt::black);
    pal.setColor(QPalette::Highlight, QColor(240, 182, 38));
    pal.setColor(QPalette::HighlightedText, Qt::black);

    a.setStyle(QStyleFactory::create("fusion"));
    a.setPalette(pal);

    MainWindow w;
    //GetStartedDlg *gs = new GetStartedDlg(nullptr);
    if (argc > 1) {
        QString argFile = argv[1];
        w.open_doc_from_path(argFile);
    }

    QPixmap splashImage("./algography_splash.png");
    if (splashImage.isNull()) {
        splashImage = QPixmap(480, 480);
        splashImage.fill(Qt::white);
    }

    QPixmap scaledSplash = splashImage.scaled(QSize(480, 480),
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation);

    QSplashScreen splash(scaledSplash);
    splash.show();

    QTimer::singleShot(1000, [&]() {
        splash.finish(&w);
        w.show();

        if (argc > 1) {
            w.open_doc_from_path(QString::fromLocal8Bit(argv[1]));
        }

        w.getStarted_dlg();
    });

    return a.exec();
}
