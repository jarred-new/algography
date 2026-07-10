#ifndef GETSTARTEDDLG_H
#define GETSTARTEDDLG_H

#include <QDialog>
#include <QFile>
#include "mainwindow.h"

namespace Ui {
class GetStartedDlg;
}

class GetStartedDlg : public QDialog
{
    Q_OBJECT

public:
    explicit GetStartedDlg(QWidget *parent = 0);
    ~GetStartedDlg();

private slots:
    void on_pushButton_4_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::GetStartedDlg *ui;
};

#endif // GETSTARTEDDLG_H
