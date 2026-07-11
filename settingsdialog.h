#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QColor>

class QPushButton;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

signals:
    void settingsChanged();

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

protected:
    void accept() override;

private slots:
    void on_pushButtonReset_clicked();

private:
    void loadSettings();
    void saveSettings();
    void pickColor(const QString &settingKey, QPushButton *button, QColor &targetColor);
    void applyColorButton(QPushButton *button, const QColor &color);

private:
    Ui::SettingsDialog *ui;
    QColor variableColor;
    QColor numberColor;
    QColor commentColor;
    QColor functionColor;
    QColor parenthesesColor;
    QColor operatorColor;
};

#endif // SETTINGSDIALOG_H
