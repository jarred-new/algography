#include "completereventfilter.h"

#include <QCompleter>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QRect>
#include <QScrollBar>
#include <QTextCursor>

CompleterEventFilter::CompleterEventFilter(QPlainTextEdit *editor, const QStringList &words, bool enabled)
    : QObject(editor)
    , textEdit(editor)
    , completionEnabled(enabled)
{
    completer = new QCompleter(words, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setWidget(textEdit);
    completer->setWrapAround(false);
    completer->popup()->installEventFilter(this);

    connect(completer,
            (void(QCompleter::*)(const QString&))&QCompleter::activated,
            this,
            &CompleterEventFilter::insertCompletion);
}

void CompleterEventFilter::setCompletionEnabled(bool enabled)
{
    completionEnabled = enabled;
}

bool CompleterEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (!completionEnabled) {
        return QObject::eventFilter(obj, event);
    }

    if (obj == textEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        bool isShortcut = (keyEvent->modifiers() & Qt::ControlModifier) && keyEvent->key() == Qt::Key_Space;

        if (!isShortcut && completer->popup()->isVisible()) {
            switch (keyEvent->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
            {
                QString completion = completer->currentCompletion();
                if (!completion.isEmpty()) {
                    insertCompletion(completion);
                }
                completer->popup()->hide();
                return true;
            }
            case Qt::Key_Space:
            {
                QString completion = completer->currentCompletion();
                if (!completion.isEmpty()) {
                    insertCompletion(completion);
                    textEdit->insertPlainText(" ");
                }
                completer->popup()->hide();
                return true;
            }
            case Qt::Key_Escape:
                completer->popup()->hide();
                return true;
            default:
                break;
            }
        }

        if (!completer) {
            return QObject::eventFilter(obj, event);
        }

        if (!isShortcut && keyEvent->text().isEmpty()) {
            return QObject::eventFilter(obj, event);
        }

        QString completionPrefix = wordUnderCursor();
        if (completionPrefix.isEmpty() && !isShortcut) {
            completer->popup()->hide();
            return QObject::eventFilter(obj, event);
        }

        // For Ctrl+Space show all completions (empty prefix)
        if (isShortcut && completionPrefix.isEmpty()) {
            completer->setCompletionPrefix("");
        } else {
            completer->setCompletionPrefix(completionPrefix);
        }

        QRect cr = textEdit->cursorRect();
        cr.setWidth(completer->popup()->sizeHintForColumn(0) + completer->popup()->verticalScrollBar()->sizeHint().width());
        completer->complete(cr);

        // Only consume the key event for the Ctrl+Space shortcut so
        // normal typing still inserts characters into the editor.
        if (isShortcut) {
            return true;
        }

        return QObject::eventFilter(obj, event);
    }

    return QObject::eventFilter(obj, event);
}

QString CompleterEventFilter::wordUnderCursor() const
{
    QTextCursor tc = textEdit->textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CompleterEventFilter::insertCompletion(const QString &completion)
{
    if (!completer) {
        return;
    }

    QTextCursor tc = textEdit->textCursor();
    tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
    tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    tc.insertText(completion);
    textEdit->setTextCursor(tc);
}

