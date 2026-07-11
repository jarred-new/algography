#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QColorDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    connect(ui->buttonVariableColor, &QPushButton::clicked, this, [this]() {
        pickColor("syntax/variableColor", ui->buttonVariableColor, variableColor);
    });
    connect(ui->buttonNumberColor, &QPushButton::clicked, this, [this]() {
        pickColor("syntax/numberColor", ui->buttonNumberColor, numberColor);
    });
    connect(ui->buttonCommentColor, &QPushButton::clicked, this, [this]() {
        pickColor("syntax/commentColor", ui->buttonCommentColor, commentColor);
    });
    connect(ui->buttonFunctionColor, &QPushButton::clicked, this, [this]() {
        pickColor("syntax/functionColor", ui->buttonFunctionColor, functionColor);
    });
    connect(ui->buttonParenthesesColor, &QPushButton::clicked, this, [this]() {
        pickColor("syntax/parenthesesColor", ui->buttonParenthesesColor, parenthesesColor);
    });
    connect(ui->buttonOperatorColor, &QPushButton::clicked, this, [this]() {
        pickColor("syntax/operatorColor", ui->buttonOperatorColor, operatorColor);
    });

    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::accept()
{
    saveSettings();
    emit settingsChanged();
    QDialog::accept();
}

void SettingsDialog::loadSettings()
{
    QSettings settings("JarredApps", "Algography");

    //ui->checkBoxLineNumbers->setChecked(settings.value("editor/showLineNumbers", true).toBool());
    ui->checkBoxAutocomplete->setChecked(settings.value("editor/autoCompleteEnabled", true).toBool());
    ui->checkBoxShowWhitespace->setChecked(settings.value("editor/showWhitespace", false).toBool());
    ui->checkBoxShowMessages->setChecked(settings.value("notifications/showInfoMessages", true).toBool());
    //ui->checkBoxShowWarnings->setChecked(settings.value("notifications/showWarnings", true).toBool());

    variableColor = settings.value("syntax/variableColor", QColor("#00952d")).value<QColor>();
    numberColor = settings.value("syntax/numberColor", QColor("#b15f00")).value<QColor>();
    commentColor = settings.value("syntax/commentColor", QColor("#621598")).value<QColor>();
    functionColor = settings.value("syntax/functionColor", QColor("#03a3c3")).value<QColor>();
    parenthesesColor = settings.value("syntax/parenthesesColor", QColor("#b5a306")).value<QColor>();
    operatorColor = settings.value("syntax/operatorColor", QColor("#df009c")).value<QColor>();

    applyColorButton(ui->buttonVariableColor, variableColor);
    applyColorButton(ui->buttonNumberColor, numberColor);
    applyColorButton(ui->buttonCommentColor, commentColor);
    applyColorButton(ui->buttonFunctionColor, functionColor);
    applyColorButton(ui->buttonParenthesesColor, parenthesesColor);
    applyColorButton(ui->buttonOperatorColor, operatorColor);
}

void SettingsDialog::saveSettings()
{
    QSettings settings("JarredApps", "Algography");

    //settings.setValue("editor/showLineNumbers", ui->checkBoxLineNumbers->isChecked());
    settings.setValue("editor/autoCompleteEnabled", ui->checkBoxAutocomplete->isChecked());
    settings.setValue("editor/showWhitespace", ui->checkBoxShowWhitespace->isChecked());
    settings.setValue("notifications/showInfoMessages", ui->checkBoxShowMessages->isChecked());
    //settings.setValue("notifications/showWarnings", ui->checkBoxShowWarnings->isChecked());

    settings.setValue("syntax/variableColor", variableColor);
    settings.setValue("syntax/numberColor", numberColor);
    settings.setValue("syntax/commentColor", commentColor);
    settings.setValue("syntax/functionColor", functionColor);
    settings.setValue("syntax/parenthesesColor", parenthesesColor);
    settings.setValue("syntax/operatorColor", operatorColor);
}

void SettingsDialog::pickColor(const QString &settingKey, QPushButton *button, QColor &targetColor)
{
    Q_UNUSED(settingKey);
    QColor color = QColorDialog::getColor(targetColor, this, "Choose color");
    if (color.isValid()) {
        targetColor = color;
        applyColorButton(button, color);
    }
}

void SettingsDialog::applyColorButton(QPushButton *button, const QColor &color)
{
    button->setStyleSheet(QString("background-color: %1;").arg(color.name()));
}

void SettingsDialog::on_pushButtonReset_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Reset Settings", "Are you sure you want to reset all settings to default values?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QSettings settings("JarredApps", "Algography");
        settings.clear();
        loadSettings();
        emit settingsChanged();
    }
}
