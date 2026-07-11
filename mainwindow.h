#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QProcess>
#include <QProgressBar>
#include "getstarteddlg.h"
#include "algohighlighter.h"
#include "completereventfilter.h"
//#include <QShowEvent>

#include <QCloseEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static void newDocument();
    static void openDocument();
    static void open_doc_from_path(QString path);
    static void getStarted_dlg();

//protected:
//    void showEvent(QShowEvent *event) override;

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    int imgWidth;
    int imgHeight;
    int imgQuality;
    QString imgName;
    QString imgJpegPath;
    void setNewAttributes(int width, int height, int quality, QString name, QString jpegPath);

protected:
    bool isNewDoc;
    bool isOpenDoc;

public slots:
    void newDocument_slot();
    void generate_image();
    bool save_document();
    bool save_document_as();
    void openDocument_slot();
    void getStarted_dlg_slot();
    bool check_existing_jpeg_path(bool withAlert);
    void change_jpeg_path();
    void open_doc_from_path_slot(QString path);

private:
    bool maybeSaveChanges();
    bool writeDocumentToFile(const QString &savePath);
    void updateDocumentActions();
    void showStatusMessage(const QString &message, int timeoutMs = 3000);
    void setGenerationProgressVisible(bool visible);

private slots:
    void readProcessOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_tabWidget_currentChanged(int index);
    void applyEditorSettings();

private:
    void start_jpegen_thread();

private:
    Ui::MainWindow *ui;
    static MainWindow *instance;
    QProcess* jpegenProcess;
    QProgressBar *progressBar;
    QString currentFilePath;
    CompleterEventFilter *redCompleterFilter;
    CompleterEventFilter *greenCompleterFilter;
    CompleterEventFilter *blueCompleterFilter;
    AlgoHighlighter *redHighlighter;
    AlgoHighlighter *greenHighlighter;
    AlgoHighlighter *blueHighlighter;
};

#endif // MAINWINDOW_H
