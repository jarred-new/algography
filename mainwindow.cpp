#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newdialog.h"

#include <QMessageBox>
#include <QPixmap>

#include <QStringList>

#include <QTimer>

#include <QInputDialog>

#include <QFileDialog>
#include <QFile>
#include <QTextStream>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

MainWindow *MainWindow::instance = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::instance = this;

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newDocument_slot);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openDocument_slot);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save_document);

    connect(ui->actionOutput, &QAction::toggled, ui->dockWidget, &QDockWidget::setVisible);
    connect(ui->actionConsole, &QAction::toggled, ui->dockWidget_3, &QDockWidget::setVisible);

    connect(ui->dockWidget, &QDockWidget::visibilityChanged, ui->actionOutput, &QAction::setChecked);
    connect(ui->dockWidget_3, &QDockWidget::visibilityChanged, ui->actionConsole, &QAction::setChecked);

    connect(ui->actionEditName, &QAction::triggered, this, [this]() {
        bool ok;

        QString nameChangeStr = QInputDialog::getText(this,
                                                      "Edit Name",
                                                      "Change your picture name here... (do not add slashes or .jpg)",
                                                      QLineEdit::Normal,
                                                      this->imgName,
                                                      &ok);

        if (ok == true) {
            QString mainPath(this->imgJpegPath + "/" + this->imgName + ".jpg");
            if (QFile::remove(mainPath)) {
                this->imgName = nameChangeStr;
                QMessageBox::StandardButton comfirmGen = QMessageBox::question(this,
                                                                               "Generate?",
                                                                               "<h1>Do you want to regenerate as you changed the picture name?</h1>",
                                                                               QMessageBox::Yes | QMessageBox::No);

                if (comfirmGen == QMessageBox::Yes) {
                    this->generate_image();
                }
            }
            else {
                QMessageBox::critical(this,
                                      "Cannot Change Name",
                                      "<h2>Cannot change the name of your picture because it doesn't delete your old picture name...</h2>",
                                      QMessageBox::Ok);
            }
        }
    });

    connect(ui->actionAboutApp, &QAction::triggered, this, [this]() {
        QMessageBox aboutMsg;

        QString aboutHtml;
        aboutHtml.append("<h1>Algography alpha</h1>");
        aboutHtml.append("<br>");
        aboutHtml.append("<p><strong>Created by: </strong>Jarred");
        aboutHtml.append("<br>");
        aboutHtml.append("<strong>Open Source Libraries:</strong>");
        aboutHtml.append("<br>ExprTk - Copyright (c) 1999-2026 Arash Partow");
        aboutHtml.append("<br>libjpeg-turbo - Copyright (C) 1991-2026, Thomas G. Lane, Guido Vollbeding.");
        aboutHtml.append("<br><br><strong><em>All Rights Reserved</em></strong></p>");

        aboutMsg.setWindowTitle("About Algography");
        aboutMsg.setStandardButtons(QMessageBox::Ok);
        aboutMsg.setIconPixmap(QPixmap(":/mainicon/algography.ico", 0, Qt::AutoColor));
        aboutMsg.setText(aboutHtml);
        aboutMsg.setTextFormat(Qt::RichText);

        aboutMsg.setParent(this, Qt::Dialog);
        aboutMsg.exec();
    });

    connect(ui->actionAboutQt, &QAction::triggered, this, [this]() {
        QMessageBox::aboutQt(this,
                             "About Qt");
    });

    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    jpegenProcess = new QProcess(this);
//    QStringList args;
//    args << "-width" << QString("%1").arg(this->imgWidth);
//    args << "-height" << QString("%1").arg(this->imgHeight);
//    args << "-quality" << QString("%1").arg(this->imgQuality);
//    args << "-r" << ui->redValue->toPlainText();
//    args << "-g" << ui->greenValue->toPlainText();
//    args << "-b" << ui->blueValue->toPlainText();
//    args << "-output" << this->imgName + ".jpg";

//    jpegenProcess->setProgram(QCoreApplication::applicationDirPath() + "/jpegen/jpegen.exe");
//    jpegenProcess->setArguments(args);

    connect(jpegenProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readProcessOutput);
    connect(jpegenProcess, &QProcess::readyReadStandardError, this, &MainWindow::readProcessOutput);
    connect(jpegenProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                this, &MainWindow::onProcessFinished);

    connect(ui->actionGenerate, &QAction::triggered,
            this, &MainWindow::generate_image);

    // only disabled when the document was not created nor open.
    ui->tabWidget->setEnabled(false);
    ui->dockWidget->setEnabled(false);
    ui->dockWidget_3->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete jpegenProcess;
    delete ui;
    exit(0);
}

void MainWindow::newDocument()
{
    if (instance) {
        QMetaObject::invokeMethod(instance,
                                  "newDocument_slot",
                                  Qt::QueuedConnection);
    }
}

void MainWindow::openDocument()
{
    if (instance) {
        QMetaObject::invokeMethod(instance,
                                  "openDocument_slot",
                                  Qt::QueuedConnection);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton comfirmQuit = QMessageBox::question(this,
                                                                    "Exit?",
                                                                    "<h1>Do you want to exit Algography?</h1>",
                                                                    QMessageBox::Yes | QMessageBox::No);

    if (comfirmQuit == QMessageBox::Yes) {
        event->accept();
        delete jpegenProcess;
        delete ui;
        exit(0);
    }
    else if (comfirmQuit == QMessageBox::No) {
        event->ignore();
    }

    //QMainWindow::closeEvent(event);
}

void MainWindow::setNewAttributes(int width, int height, int quality, QString name, QString jpegPath)
{
    this->imgWidth = width;
    this->imgHeight = height;
    this->imgQuality = quality;
    this->imgName = name;
    this->imgJpegPath = jpegPath;

    ui->tabWidget->setEnabled(true);
    ui->dockWidget->setEnabled(true);
    ui->dockWidget_3->setEnabled(true);

    QString msg = QString("Name: %4 \nWidth: %1 \nHeight: %2 \nQuality: %3 \nJPEG Path: %5")
            .arg(this->imgWidth)
            .arg(this->imgHeight)
            .arg(this->imgQuality)
            .arg(this->imgName)
            .arg(this->imgJpegPath);

    QMessageBox::about(this,
                       "Image Information",
                       msg);
}

void MainWindow::newDocument_slot()
{
    NewDialog newdlg(this);

    if (newdlg.exec() == QDialog::Accepted) {
        this->setNewAttributes(newdlg.inputWidth,
                               newdlg.inputHeight,
                               newdlg.inputQuality,
                               newdlg.inputName,
                               newdlg.inputJPGPath);
    }
}

void MainWindow::readProcessOutput() {
    // Read and append data from standard output and standard error streams
    QByteArray output = jpegenProcess->readAllStandardOutput();
    QByteArray error = jpegenProcess->readAllStandardError();

    if (!output.isEmpty()) {
        ui->textBrowser->append(QString::fromLocal8Bit(output));
    }
    if (!error.isEmpty()) {
        ui->textBrowser->setTextColor(Qt::red);
        ui->textBrowser->append(QString::fromLocal8Bit(error));
        ui->textBrowser->setTextColor(Qt::black); // Reset color
    }
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    ui->textBrowser->append("--- Generate Finished ---");
    ui->textBrowser->append(QString("jpegen exit code: %1").arg(exitCode));
    QPixmap pix(this->imgJpegPath + "/" + this->imgName + ".jpg");
    ui->label->setPixmap(pix.scaled(
                             ui->label->size(),
                             Qt::KeepAspectRatio)
                         );
}

void MainWindow::start_jpegen_thread()
{
    QStringList args;
    args << "-width" << QString("%1").arg(this->imgWidth);
    args << "-height" << QString("%1").arg(this->imgHeight);
    args << "-quality" << QString("%1").arg(this->imgQuality);
    args << "-r" << (!ui->redValue->toPlainText().isEmpty() ? ui->redValue->toPlainText() : "0");
    args << "-g" << (!ui->greenValue->toPlainText().isEmpty() ? ui->greenValue->toPlainText() : "0");
    args << "-b" << (!ui->blueValue->toPlainText().isEmpty() ? ui->blueValue->toPlainText() : "0");
    args << "-output" << this->imgJpegPath + "/" + this->imgName + ".jpg";

    QString program(QCoreApplication::applicationDirPath() + "/jpegen/jpegen.exe");
    jpegenProcess->setProgram(program);
    jpegenProcess->setArguments(args);

    jpegenProcess->start();

//    QMetaObject::invokeMethod(jpegenProcess, "start", Qt::QueuedConnection);
}

void MainWindow::generate_image()
{
    ui->textBrowser->clear();
//    QMetaObject::invokeMethod(this, [this]() {
//        this->start_jpegen_thread();
//    }, Qt::QueuedConnection);

    this->start_jpegen_thread();
}

void MainWindow::save_document()
{
    QString savePath = QFileDialog::getSaveFileName(this,
                                                    "Save",
                                                    nullptr,
                                                    "Algo file (*.algo)");

    if (!savePath.isEmpty()) {
        QFile saveFile(savePath);

        if (saveFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonArray jarr;
            QJsonObject jobj;
            jobj["width"] = imgWidth;
            jobj["height"] = imgHeight;
            jobj["quality"] = imgQuality;

            jobj["red"] = ui->redValue->toPlainText();
            jobj["green"] = ui->greenValue->toPlainText();
            jobj["blue"] = ui->blueValue->toPlainText();

            jobj["name"] = imgName;
            jobj["jpgpath"] = imgJpegPath;

            jarr.append(jobj);

            QJsonDocument jdoc(jarr);
            QTextStream stream(&saveFile);
            stream << jdoc.toJson(QJsonDocument::Indented);

            saveFile.close();
        } else {
            QMessageBox::critical(this,
                                  "Cannot Save!",
                                  "<h1>Cannot open file for writing!</h1>",
                                  QMessageBox::Ok);
        }
    }
}

void MainWindow::openDocument_slot()
{
    QString openPath = QFileDialog::getOpenFileName(this,
                                                    "Open Document",
                                                    nullptr,
                                                    "Algo file (*.algo)");

    if (!openPath.isEmpty()) {
        QFile openFile(openPath);

        if (openFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = openFile.readAll();
            openFile.close();

            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (!doc.isArray()) {
                QMessageBox::critical(this,
                                      "Error!",
                                      "<h1>JSON is not an array</h1>",
                                      QMessageBox::Ok);

                return;
            }

            // 3. Extract the array and iterate through the objects
            QJsonArray jsonArray = doc.array();
            for (const QJsonValue &value : jsonArray) {
                QJsonObject obj = value.toObject();

                // 4. Access the specific object fields
                int jw = obj["width"].toInt();
                int jh = obj["height"].toInt();
                int jq = obj["quality"].toInt();

                QString jrv = obj["red"].toString();
                QString jgv = obj["green"].toString();
                QString jbv = obj["blue"].toString();

                QString jname = obj["name"].toString();
                QString jjpgpath = obj["jpgpath"].toString();

                this->setNewAttributes(jw, jh, jq, jname, jjpgpath);

                ui->redValue->setPlainText(jrv);
                ui->greenValue->setPlainText(jgv);
                ui->blueValue->setPlainText(jbv);
            }
        }
        else {
            QMessageBox::critical(this,
                                  "Cannot Open!",
                                  "<h1>Cannot open file for reading!</h1>",
                                  QMessageBox::Ok);
        }
    }
}

//void MainWindow::showEvent(QShowEvent *event)
//{
////    GetStartedDlg *getStarted = new GetStartedDlg(this);
////    getStarted->exec();

//    MainWindow::showEvent(event);
//}
