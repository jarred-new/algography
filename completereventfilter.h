#ifndef COMPLETEREVENTFILTER_H
#define COMPLETEREVENTFILTER_H

#include <QObject>
#include <QStringList>

class QCompleter;
class QPlainTextEdit;
class QEvent;

class CompleterEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit CompleterEventFilter(QPlainTextEdit *editor, const QStringList &words, bool enabled = true);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void insertCompletion(const QString &completion);

public:
    void setCompletionEnabled(bool enabled);

private:
    QString wordUnderCursor() const;

    QPlainTextEdit *textEdit;
    QCompleter *completer;
    bool completionEnabled;
};

#endif // COMPLETEREVENTFILTER_H
