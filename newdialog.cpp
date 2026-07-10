#include "newdialog.h"
#include "ui_newdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

NewDialog::NewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewDialog)
{
    ui->setupUi(this);
}

NewDialog::~NewDialog()
{
    delete ui;
}

void NewDialog::on_buttonBox_accepted()
{
    inputWidth = ui->spinWidth->value();
    inputHeight = ui->spinHeight->value();
    inputQuality = ui->qualitySlider->value();

    if (!ui->lineEdit->text().isEmpty()) {
        inputName = ui->lineEdit->text();
    }
    else {
        inputName = "untitled";
    }

    if (!ui->lineEdit_2->text().isEmpty()) {

        inputJPGPath = ui->lineEdit_2->text();

        QDir jpgdir(this->inputJPGPath);
        if (!jpgdir.exists()) {
            QDir dirG;
            if (!dirG.mkdir(this->inputJPGPath)) {
                QMessageBox::critical(this,
                                      "Cannot create path...",
                                      "<h1>Cannot create path, moving to the app's current path</h1>",
                                      QMessageBox::Ok);
                inputJPGPath = ".";
            }
        }

    }
    else {
        inputJPGPath = ".";
    }
}

void NewDialog::on_pushButton_clicked()
{
    QString jpegpathDlg = QFileDialog::getExistingDirectory(this,
                                                            "Set JPEG Path",
                                                            nullptr);

    if (!jpegpathDlg.isEmpty()) {
        ui->lineEdit_2->setText(jpegpathDlg);
    }
}
