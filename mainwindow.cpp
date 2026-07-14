#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newdialog.h"
#include "settingsdialog.h"

#include <QCloseEvent>
#include <QColor>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPalette>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QRegExp>
#include <QSettings>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>
#include <QStringList>
#include <QTimer>

static void setupCodeEditor(QPlainTextEdit *editor, CompleterEventFilter **filterOut, AlgoHighlighter **highlighterOut, const bool completionEnabled)
{
    QPalette pal = editor->palette();
    // pal.setColor(QPalette::Base, baseColor);
    pal.setColor(QPalette::Text, Qt::white);
    pal.setColor(QPalette::Highlight, QColor("#44475a"));
    pal.setColor(QPalette::HighlightedText, Qt::white);
    editor->setPalette(pal);
    editor->setTabStopWidth(editor->fontMetrics().width(' ') * 4);
    AlgoHighlighter *highlighter = new AlgoHighlighter(editor->document());
    if (highlighterOut)
    {
        *highlighterOut = highlighter;
    }

    QStringList keywords;
    keywords << "x" << "y" << "width" << "height"
             // basic math + utility
             << "abs" << "avg" << "ceil" << "clamp" << "equal" << "erf" << "erfc"
             << "exp" << "expm1" << "floor" << "frac" << "log" << "log10" << "log1p" << "log2" << "logn"
             << "max" << "min" << "mul" << "ncdf" << "not_equal" << "root" << "round" << "roundn" << "sgn"
             << "sqrt" << "sum" << "swap" << "trunc"
             // trig + hyperbolic + other
             << "acos" << "acosh" << "asin" << "asinh" << "atan" << "atanh" << "atan2"
             << "cos" << "cosh" << "cot" << "csc" << "sec" << "sin" << "sinc" << "sinh"
             << "tan" << "tanh" << "hypot"
             // degree/radian conversions
             << "rad2deg" << "deg2grad" << "deg2rad" << "grad2deg";

    CompleterEventFilter *filter = new CompleterEventFilter(editor, keywords, completionEnabled);
    if (filterOut)
    {
        *filterOut = filter;
    }
    editor->installEventFilter(filter);
}

MainWindow *MainWindow::instance = nullptr;

void MainWindow::showStatusMessage(const QString &message, int timeoutMs)
{
    QSettings settings("JarredApps", "Algography");
    if (!settings.value("notifications/showInfoMessages", true).toBool())
    {
        return;
    }

    statusBar()->showMessage(message, timeoutMs);
}

void MainWindow::setGenerationProgressVisible(bool visible)
{
    progressBar->setVisible(visible);
    if (visible)
    {
        progressBar->setRange(0, 0);
    }
    else
    {
        progressBar->setValue(0);
    }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::instance = this;

    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setMaximumWidth(180);
    progressBar->setVisible(false);
    statusBar()->addPermanentWidget(progressBar);
    showStatusMessage("Ready", 2000);

    QSettings settings("JarredApps", "Algography");
    const bool autoCompleteEnabled = settings.value("editor/autoCompleteEnabled", true).toBool();
    const bool showWhitespace = settings.value("editor/showWhitespace", false).toBool();

    setupCodeEditor(ui->redValue, &redCompleterFilter, &redHighlighter, autoCompleteEnabled);
    setupCodeEditor(ui->greenValue, &greenCompleterFilter, &greenHighlighter, autoCompleteEnabled);
    setupCodeEditor(ui->blueValue, &blueCompleterFilter, &blueHighlighter, autoCompleteEnabled);

    auto applyWhitespace = [this, showWhitespace](QPlainTextEdit *editor)
    {
        if (!editor) {
            return;
        }

        QTextOption option = editor->document()->defaultTextOption();
        option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces);
        if (showWhitespace) {
            option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
        }
        editor->document()->setDefaultTextOption(option);
        editor->viewport()->update();
    };

    applyWhitespace(ui->redValue);
    applyWhitespace(ui->greenValue);
    applyWhitespace(ui->blueValue);

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newDocument_slot);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openDocument_slot);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save_document);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::save_document_as);
    connect(ui->actionGetStarted, &QAction::triggered, this, &MainWindow::getStarted_dlg_slot);

    connect(ui->actionOutput, &QAction::toggled, ui->dockWidget, &QDockWidget::setVisible);
    connect(ui->actionConsole, &QAction::toggled, ui->dockWidget_3, &QDockWidget::setVisible);

    connect(ui->dockWidget, &QDockWidget::visibilityChanged, ui->actionOutput, &QAction::setChecked);
    connect(ui->dockWidget_3, &QDockWidget::visibilityChanged, ui->actionConsole, &QAction::setChecked);

    connect(ui->actionEditJPEGPath, &QAction::triggered, this, &MainWindow::change_jpeg_path);
    connect(ui->actionCheck_JPEG_Path, &QAction::triggered, this, [this]()
            { check_existing_jpeg_path(true); });

    connect(ui->actionClearConsole, &QAction::triggered, this, [this]()
            {
        ui->textBrowser->clear();
        showStatusMessage("Console cleared.", 2000); });

    connect(ui->actionSettings, &QAction::triggered, this, [this]()
            {
        showStatusMessage("Opening settings...");
        SettingsDialog s(this);
        s.setWindowFlags(Qt::Dialog);
        connect(&s, &SettingsDialog::settingsChanged, this, &MainWindow::applyEditorSettings);
        s.exec(); });

    connect(ui->actionEditName, &QAction::triggered, this, [this]()
            {
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
        } });

    connect(ui->actionAboutApp, &QAction::triggered, this, [this]()
            {
        showStatusMessage("Opening about dialog...");
        QMessageBox aboutMsg;

        QString aboutHtml;
        aboutHtml.append("<h1>Algography v1.0</h1>");
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
        aboutMsg.exec(); });

    connect(ui->actionAboutQt, &QAction::triggered, this, [this]()
            {
        showStatusMessage("Opening Qt information...");
        QMessageBox::aboutQt(this,
                             "About Qt"); });

    connect(ui->actionExit, &QAction::triggered, this, [this]()
            {
        showStatusMessage("Closing application...");
        this->close(); });

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
    connect(jpegenProcess, &QProcess::started, this, [this]()
            {
        showStatusMessage("Generating image...");
        setGenerationProgressVisible(true); });
    connect(jpegenProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &MainWindow::onProcessFinished);

    connect(ui->actionGenerate, &QAction::triggered,
            this, &MainWindow::generate_image);

    this->isNewDoc = false;
    this->isOpenDoc = false;
    this->currentFilePath.clear();
    updateDocumentActions();
}

MainWindow::~MainWindow()
{
    delete jpegenProcess;
    delete ui;
}

void MainWindow::newDocument()
{
    if (instance)
    {
        QMetaObject::invokeMethod(instance,
                                  "newDocument_slot",
                                  Qt::QueuedConnection);
    }
}

void MainWindow::openDocument()
{
    if (instance)
    {
        QMetaObject::invokeMethod(instance,
                                  "openDocument_slot",
                                  Qt::QueuedConnection);
    }
}

void MainWindow::open_doc_from_path(QString path)
{
    if (instance)
    {
        QMetaObject::invokeMethod(instance,
                                  "open_doc_from_path_slot",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, path));
    }
}

void MainWindow::getStarted_dlg()
{
    if (instance)
    {
        QMetaObject::invokeMethod(instance,
                                  "getStarted_dlg_slot",
                                  Qt::QueuedConnection);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSaveChanges())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::setNewAttributes(int width, int height, int quality, QString name, QString jpegPath)
{
    QSettings settings("JarredApps", "Algography");

    this->imgWidth = width;
    this->imgHeight = height;
    this->imgQuality = quality;
    this->imgName = name;
    this->imgJpegPath = jpegPath;

    updateDocumentActions();

    if (settings.value("editor/autoCompleteEnabled", true).toBool())
    {
        redCompleterFilter->setCompletionEnabled(true);
        greenCompleterFilter->setCompletionEnabled(true);
        blueCompleterFilter->setCompletionEnabled(true);
    }
    else
    {
        redCompleterFilter->setCompletionEnabled(false);
        greenCompleterFilter->setCompletionEnabled(false);
        blueCompleterFilter->setCompletionEnabled(false);
    }

    if (settings.value("editor/syntaxHighlightingEnabled", true).toBool())
    {
        redHighlighter->setDocument(ui->redValue->document());
        greenHighlighter->setDocument(ui->greenValue->document());
        blueHighlighter->setDocument(ui->blueValue->document());
    }
    else
    {
        redHighlighter->setDocument(nullptr);
        greenHighlighter->setDocument(nullptr);
        blueHighlighter->setDocument(nullptr);
    }

    if (settings.value("notifications/showInfoMessages", true).toBool())
    {
        showStatusMessage("New document created.", 3000);
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
    else
    {
        showStatusMessage("New document created.", 3000);
    }
}

void MainWindow::newDocument_slot()
{
    showStatusMessage("Creating a new document...");

    if (!maybeSaveChanges())
    {
        return;
    }

    // Clear the current document state
    ui->redValue->clear();
    ui->greenValue->clear();
    ui->blueValue->clear();
    ui->textBrowser->clear();
    //ui->label->clear();
    ui->label->setPixmap(QPixmap());

    NewDialog newdlg(this);

    if (newdlg.exec() == QDialog::Accepted)
    {
        this->setNewAttributes(newdlg.inputWidth,
                               newdlg.inputHeight,
                               newdlg.inputQuality,
                               newdlg.inputName,
                               newdlg.inputJPGPath);
        this->currentFilePath.clear();
        this->isNewDoc = true;
        this->isOpenDoc = false;
        updateDocumentActions();
        showStatusMessage("New document ready.", 3000);
    }
}

void MainWindow::readProcessOutput()
{
    // Read and append data from standard output and standard error streams
    QByteArray output = jpegenProcess->readAllStandardOutput();
    QByteArray error = jpegenProcess->readAllStandardError();

    if (!output.isEmpty())
    {
        ui->textBrowser->append(QString::fromLocal8Bit(output));
    }
    if (!error.isEmpty())
    {
        ui->textBrowser->setTextColor(Qt::red);
        ui->textBrowser->append(QString::fromLocal8Bit(error));
        ui->textBrowser->setTextColor(Qt::black); // Reset color
    }
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    ui->textBrowser->append("--- Generate Finished ---");
    ui->textBrowser->append(QString("jpegen exit code: %1").arg(exitCode));
    setGenerationProgressVisible(false);
    showStatusMessage("Image generation finished.", 4000);
    QPixmap pix(this->imgJpegPath + "/" + this->imgName + ".jpg");
    ui->label->setPixmap(pix.scaled(
        ui->label->size(),
        Qt::KeepAspectRatio));
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
    showStatusMessage("Starting image generation...");
    setGenerationProgressVisible(true);
    //    QMetaObject::invokeMethod(this, [this]() {
    //        this->start_jpegen_thread();
    //    }, Qt::QueuedConnection);

    this->start_jpegen_thread();
}

bool MainWindow::save_document()
{
    if (this->currentFilePath.isEmpty())
    {
        return save_document_as();
    }

    if (writeDocumentToFile(this->currentFilePath))
    {
        this->isNewDoc = false;
        this->isOpenDoc = true;
        updateDocumentActions();
        showStatusMessage("Document saved.", 3000);
        return true;
    }

    return false;
}

bool MainWindow::save_document_as()
{
    showStatusMessage("Choosing a save location...");

    QString savePath = QFileDialog::getSaveFileName(this,
                                                    "Save As",
                                                    nullptr,
                                                    "Algo file (*.algo)");

    if (savePath.isEmpty())
    {
        showStatusMessage("Save canceled.", 2000);
        return false;
    }

    if (writeDocumentToFile(savePath))
    {
        this->currentFilePath = savePath;
        this->isNewDoc = false;
        this->isOpenDoc = true;
        updateDocumentActions();
        showStatusMessage("Document saved as a new file.", 3000);
        return true;
    }

    return false;
}

bool MainWindow::writeDocumentToFile(const QString &savePath)
{
    QFile saveFile(savePath);
    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this,
                              "Cannot Save!",
                              "<h1>Cannot open file for writing!</h1>",
                              QMessageBox::Ok);
        return false;
    }

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
    return true;
}

bool MainWindow::maybeSaveChanges()
{
    if (!this->isNewDoc && !this->isOpenDoc)
    {
        return true;
    }

    QMessageBox::StandardButton choice = QMessageBox::warning(this,
                                                              "Save Changes?",
                                                              "The current document has unsaved changes. Do you want to save them before continuing?",
                                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (choice == QMessageBox::Save)
    {
        return save_document();
    }
    else if (choice == QMessageBox::Discard)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void MainWindow::applyEditorSettings()
{
    QSettings settings("JarredApps", "Algography");
    const bool autoCompleteEnabled = settings.value("editor/autoCompleteEnabled", true).toBool();
    const bool showWhitespace = settings.value("editor/showWhitespace", false).toBool();

    if (redCompleterFilter)
    {
        redCompleterFilter->setCompletionEnabled(autoCompleteEnabled);
    }
    if (greenCompleterFilter)
    {
        greenCompleterFilter->setCompletionEnabled(autoCompleteEnabled);
    }
    if (blueCompleterFilter)
    {
        blueCompleterFilter->setCompletionEnabled(autoCompleteEnabled);
    }

    auto applyWhitespace = [this, showWhitespace](QPlainTextEdit *editor)
    {
        if (!editor) {
            return;
        }

        QTextOption option = editor->document()->defaultTextOption();
        option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces);
        if (showWhitespace) {
            option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
        }
        editor->document()->setDefaultTextOption(option);
        editor->viewport()->update();
    };

    applyWhitespace(ui->redValue);
    applyWhitespace(ui->greenValue);
    applyWhitespace(ui->blueValue);

    if (redHighlighter)
    {
        redHighlighter->refreshSettings();
        ui->redValue->document()->setModified(true);
        redHighlighter->rehighlight();
    }
    if (greenHighlighter)
    {
        greenHighlighter->refreshSettings();
        ui->greenValue->document()->setModified(true);
        greenHighlighter->rehighlight();
    }
    if (blueHighlighter)
    {
        blueHighlighter->refreshSettings();
        ui->blueValue->document()->setModified(true);
        blueHighlighter->rehighlight();
    }
}

void MainWindow::updateDocumentActions()
{
    bool hasDocument = this->isNewDoc || this->isOpenDoc;
    ui->actionSave->setEnabled(hasDocument);
    ui->actionSaveAs->setEnabled(hasDocument);
    ui->actionGenerate->setEnabled(hasDocument);
    ui->actionEditJPEGPath->setEnabled(hasDocument);
    ui->actionCheck_JPEG_Path->setEnabled(hasDocument);
    ui->actionEditName->setEnabled(hasDocument);
    ui->actionCopy->setEnabled(hasDocument);
    ui->actionCut->setEnabled(hasDocument);
    ui->actionPaste->setEnabled(hasDocument);
    ui->tabWidget->setEnabled(hasDocument);
    ui->dockWidget->setEnabled(hasDocument);
    ui->dockWidget_3->setEnabled(hasDocument);
}

void MainWindow::openDocument_slot()
{
    showStatusMessage("Opening document...");

    if (!maybeSaveChanges())
    {
        return;
    }

    // Clear the current document state
    ui->redValue->clear();
    ui->greenValue->clear();
    ui->blueValue->clear();
    ui->textBrowser->clear();
    //ui->label->clear();
    ui->label->setPixmap(QPixmap());

    QString openPath = QFileDialog::getOpenFileName(this,
                                                    "Open Document",
                                                    nullptr,
                                                    "Algo file (*.algo)");

    if (openPath.isEmpty())
    {
        showStatusMessage("Open canceled.", 2000);
        return;
    }

    if (!openPath.isEmpty())
    {
        QFile openFile(openPath);

        if (openFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QByteArray data = openFile.readAll();
            openFile.close();

            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (!doc.isArray())
            {
                QMessageBox::critical(this,
                                      "Error!",
                                      "<h1>JSON is not an array</h1>",
                                      QMessageBox::Ok);

                return;
            }

            QJsonArray jsonArray = doc.array();
            for (const QJsonValue &value : jsonArray)
            {
                QJsonObject obj = value.toObject();

                int jw = obj["width"].toInt();
                int jh = obj["height"].toInt();
                int jq = obj["quality"].toInt();

                QString jrv = obj["red"].toString();
                QString jgv = obj["green"].toString();
                QString jbv = obj["blue"].toString();

                QString jname = obj["name"].toString();
                QString jjpgpath = obj["jpgpath"].toString();

                this->setNewAttributes(jw, jh, jq, jname, jjpgpath);
                this->currentFilePath = openPath;
                this->isOpenDoc = true;
                this->isNewDoc = false;
                updateDocumentActions();
                showStatusMessage("Document opened.", 3000);

                ui->redValue->setPlainText(jrv);
                ui->greenValue->setPlainText(jgv);
                ui->blueValue->setPlainText(jbv);
            }
        }
        else
        {
            QMessageBox::critical(this,
                                  "Cannot Open!",
                                  "<h1>Cannot open file for reading!</h1>",
                                  QMessageBox::Ok);
            showStatusMessage("Document cannot be opened.", 3000);
        }
    }
}

void MainWindow::getStarted_dlg_slot()
{
    showStatusMessage("Opening getting started guide...");
    GetStartedDlg getStarted(this);
    getStarted.exec();
}

bool MainWindow::check_existing_jpeg_path(bool withAlert)
{
    QDir *dirE = new QDir(this->imgJpegPath);
    if (withAlert == true)
    {
        if (dirE->exists())
        {
            QMessageBox::information(this,
                                     "JPEG Path exists!",
                                     "The document's JPEG path has been <i><strong>existed</strong></i> in your computer...");
            delete dirE;
            return true;
        }
        else
        {
            showStatusMessage("JPEG path is missing.", 3000);
            QMessageBox::critical(this,
                                  "It doesn't exist",
                                  "The document's JPEG path was from <i><strong>another computer</strong></i> or on <i><strong>a removable storage</strong></i><br>Please change your JPEG path...");
            delete dirE;
            return false;
        }
    }
    else if (withAlert == false)
    {
        if (dirE->exists())
        {
            delete dirE;
            return true;
        }
        else
        {
            delete dirE;
            return false;
        }
    }
    return true;
}

void MainWindow::change_jpeg_path()
{
    QString newJPEGPath = QFileDialog::getExistingDirectory(this,
                                                            "Change JPEG Path",
                                                            nullptr);

    if (!newJPEGPath.isEmpty())
    {
        this->imgJpegPath = newJPEGPath;
        showStatusMessage("JPEG path updated.", 3000);
    }
}

void MainWindow::open_doc_from_path_slot(QString path)
{
    showStatusMessage("Opening document from startup path...");
    QFile openFile(path);

    if (openFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray data = openFile.readAll();
        openFile.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isArray())
        {
            QMessageBox::critical(this,
                                  "Error!",
                                  "<h1>JSON is not an array</h1>",
                                  QMessageBox::Ok);

            return;
        }

        QJsonArray jsonArray = doc.array();
        for (const QJsonValue &value : jsonArray)
        {
            QJsonObject obj = value.toObject();

            int jw = obj["width"].toInt();
            int jh = obj["height"].toInt();
            int jq = obj["quality"].toInt();

            QString jrv = obj["red"].toString();
            QString jgv = obj["green"].toString();
            QString jbv = obj["blue"].toString();

            QString jname = obj["name"].toString();
            QString jjpgpath = obj["jpgpath"].toString();

            this->setNewAttributes(jw, jh, jq, jname, jjpgpath);
            this->currentFilePath = path;
            this->isOpenDoc = true;
            this->isNewDoc = false;
            updateDocumentActions();
            showStatusMessage("Document opened from startup path.", 3000);

            ui->redValue->setPlainText(jrv);
            ui->greenValue->setPlainText(jgv);
            ui->blueValue->setPlainText(jbv);
        }
    }
    else
    {
        QMessageBox::critical(this,
                              "Cannot Open!",
                              "<h1>Cannot open file for reading!</h1>",
                              QMessageBox::Ok);
    }
}

// void MainWindow::showEvent(QShowEvent *event)
//{
////    GetStartedDlg *getStarted = new GetStartedDlg(this);
////    getStarted->exec();

//    MainWindow::showEvent(event);
//}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    // copy
    if (index == 0)
    {
        connect(ui->actionCopy, &QAction::triggered, ui->redValue, &QPlainTextEdit::copy);
    }
    else
    {
        disconnect(ui->actionCopy, &QAction::triggered, ui->redValue, &QPlainTextEdit::copy);
    }
    if (index == 1)
    {
        connect(ui->actionCopy, &QAction::triggered, ui->greenValue, &QPlainTextEdit::copy);
    }
    else
    {
        disconnect(ui->actionCopy, &QAction::triggered, ui->greenValue, &QPlainTextEdit::copy);
    }
    if (index == 2)
    {
        connect(ui->actionCopy, &QAction::triggered, ui->blueValue, &QPlainTextEdit::copy);
    }
    else
    {
        disconnect(ui->actionCopy, &QAction::triggered, ui->blueValue, &QPlainTextEdit::copy);
    }

    // cut
    if (index == 0)
    {
        connect(ui->actionCut, &QAction::triggered, ui->redValue, &QPlainTextEdit::cut);
    }
    else
    {
        disconnect(ui->actionCut, &QAction::triggered, ui->redValue, &QPlainTextEdit::cut);
    }
    if (index == 1)
    {
        connect(ui->actionCut, &QAction::triggered, ui->greenValue, &QPlainTextEdit::cut);
    }
    else
    {
        disconnect(ui->actionCut, &QAction::triggered, ui->greenValue, &QPlainTextEdit::cut);
    }
    if (index == 2)
    {
        connect(ui->actionCut, &QAction::triggered, ui->blueValue, &QPlainTextEdit::cut);
    }
    else
    {
        disconnect(ui->actionCut, &QAction::triggered, ui->blueValue, &QPlainTextEdit::cut);
    }

    // paste
    if (index == 0)
    {
        connect(ui->actionPaste, &QAction::triggered, ui->redValue, &QPlainTextEdit::paste);
    }
    else
    {
        disconnect(ui->actionPaste, &QAction::triggered, ui->redValue, &QPlainTextEdit::paste);
    }
    if (index == 1)
    {
        connect(ui->actionPaste, &QAction::triggered, ui->greenValue, &QPlainTextEdit::paste);
    }
    else
    {
        disconnect(ui->actionPaste, &QAction::triggered, ui->greenValue, &QPlainTextEdit::paste);
    }
    if (index == 2)
    {
        connect(ui->actionPaste, &QAction::triggered, ui->blueValue, &QPlainTextEdit::paste);
    }
    else
    {
        disconnect(ui->actionPaste, &QAction::triggered, ui->blueValue, &QPlainTextEdit::paste);
    }
}
