#ifndef ALGOHIGHLIGHTER_H
#define ALGOHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegExp>
#include <QTextCharFormat>
#include <QVector>

class AlgoHighlighter : public QSyntaxHighlighter
{
public:
    explicit AlgoHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text) override;

public:
    void refreshSettings();

private:
    struct HighlightingRule {
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;
};

#endif // ALGOHIGHLIGHTER_H
