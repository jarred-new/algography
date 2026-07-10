#include "getstarteddlg.h"
#include "ui_getstarteddlg.h"
#include <QMessageBox>

GetStartedDlg::GetStartedDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GetStartedDlg)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    connect(ui->pushButton_5, &QPushButton::clicked, this, &GetStartedDlg::close);

    QFile *detector = new QFile(QCoreApplication::applicationDirPath() + "/jpegen/jpegen.exe");

    if (!detector->exists()) {
        QMessageBox::StandardButton errFound = QMessageBox::critical(this,
                                                                     "Cannot Find",
                                                                     "<h1>This app cannot find \"jpegen.exe\"</h1>"
                                                                     "</br>"
                                                                     "<p>Cannot find jpegen.exe and this is important for generating images.</p></br>"
                                                                     "<p>Please reinstall Algography to fix.</p></br>"
                                                                     "<p>This app will quit...</p>",
                                                                     QMessageBox::Ok);

        if (errFound == QMessageBox::Ok) {
            delete detector;
            delete ui;
            exit(0);
        }
    }
    else {
        delete detector;
    }
}

GetStartedDlg::~GetStartedDlg()
{
    delete ui;
}

void GetStartedDlg::on_pushButton_4_clicked()
{
    QMessageBox::StandardButton comfirmQuit = QMessageBox::question(this,
                                                                    "Exit?",
                                                                    "<h1>Do you want to exit Algography?</h1>",
                                                                    QMessageBox::Yes | QMessageBox::No);

    if (comfirmQuit == QMessageBox::Yes) {
        delete ui;
        exit(0);
    }
}

void GetStartedDlg::on_pushButton_clicked()
{
    this->close();
    MainWindow::newDocument();
}

void GetStartedDlg::on_pushButton_2_clicked()
{
    this->close();
    MainWindow::openDocument();
}
