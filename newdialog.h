#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <QDialog>

#include <QString>

namespace Ui {
class NewDialog;
}

class NewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewDialog(QWidget *parent = 0);
    ~NewDialog();

private:
    Ui::NewDialog *ui;

public:
    int inputWidth;
    int inputHeight;
    int inputQuality;
    QString inputName;
    QString inputJPGPath;

private slots:
    void on_buttonBox_accepted();
    void on_pushButton_clicked();
};

#endif // NEWDIALOG_H
