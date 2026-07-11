#include "algohighlighter.h"
#include <QColor>
#include <QFont>
#include <QSettings>

AlgoHighlighter::AlgoHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    refreshSettings();
}

void AlgoHighlighter::refreshSettings()
{
    highlightingRules.clear();

    QSettings settings("JarredApps", "Algography");

    QTextCharFormat variableFormat;
    variableFormat.setForeground(settings.value("syntax/variableColor", QColor("#00952d")).value<QColor>());
    variableFormat.setFontWeight(QFont::Bold);

    QTextCharFormat numberFormat;
    numberFormat.setForeground(settings.value("syntax/numberColor", QColor("#b15f00")).value<QColor>());

    QTextCharFormat commentFormat;
    commentFormat.setForeground(settings.value("syntax/commentColor", QColor("#621598")).value<QColor>());
    commentFormat.setFontItalic(true);

    HighlightingRule rule;
    rule.pattern = QRegExp("\\b(x|y|width|height)\\b");
    rule.format = variableFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegExp("\\b[0-9]+(\\.[0-9]+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegExp("//[^\\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    QTextCharFormat functionFormat;
    functionFormat.setForeground(settings.value("syntax/functionColor", QColor("#03a3c3")).value<QColor>());
    functionFormat.setFontWeight(QFont::Bold);

    rule.pattern = QRegExp("\\b(abs|acos|acosh|asin|asinh|atan|atan2|atanh|avg|ceil|clamp|cos|cosh|cot|csc|deg2grad|deg2rad|exp|expm1|erf|erfc|floor|frac|grad2deg|hypot|log|log1p|log10|log2|logn|max|min|mul|ncdf|not_equal|pow|rad2deg|root|round|roundn|sec|sgn|sin|sinc|sinh|sqrt|sum|swap|trunc|tan|tanh)\\b");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    QTextCharFormat parenFormat;
    parenFormat.setForeground(settings.value("syntax/parenthesesColor", QColor("#b5a306")).value<QColor>());
    rule.pattern = QRegExp("[\\(\\)\\{\\}\\[\\]]");
    rule.format = parenFormat;
    highlightingRules.append(rule);

    QTextCharFormat operatorFormat;
    operatorFormat.setForeground(settings.value("syntax/operatorColor", QColor("#df009c")).value<QColor>());
    rule.pattern = QRegExp("[+\\-*/%\\^=<>!&|]+");
    rule.format = operatorFormat;
    highlightingRules.append(rule);
}

void AlgoHighlighter::highlightBlock(const QString &text)
{
    for (int i = 0; i < highlightingRules.size(); ++i) {
        const HighlightingRule &rule = highlightingRules.at(i);
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}

