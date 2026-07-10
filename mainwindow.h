#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QProcess>
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

public slots:
    void newDocument_slot();
    void generate_image();
    void save_document();
    void openDocument_slot();

private slots:
    void readProcessOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void start_jpegen_thread();

private:
    Ui::MainWindow *ui;
    static MainWindow *instance;
    QProcess* jpegenProcess;
};

#endif // MAINWINDOW_H
