#include <QTextBlock>
#include <QTextDocument>
#include <QtTest>

#include "markdownhighlighter.h"

class MarkdownHighlighterTest : public QObject {
    Q_OBJECT

   private slots:
    void closingFence_data();
    void closingFence();
    void inlineBackticks();
};

void MarkdownHighlighterTest::closingFence_data() {
    QTest::addColumn<QString>("marker");
    QTest::addColumn<QString>("language");
    QTest::addColumn<QString>("ending");
    QTest::addColumn<bool>("closes");

    for (const QString &marker :
         {QStringLiteral("```"), QStringLiteral("~~~")}) {
        for (const QString &language :
             {QString(), QStringLiteral("python"), QStringLiteral("cpp")}) {
            const QStringList endings = {QStringLiteral("text"),
                                         QStringLiteral(" text"),
                                         QStringLiteral(" text```"),
                                         QString(QChar(0x00a0)),
                                         QString(),
                                         QStringLiteral(" \t"),
                                         QString(marker.at(0)),
                                         marker};
            for (int i = 0; i < endings.size(); ++i) {
                const QByteArray name =
                    (marker + "/" + language + "/" + QString::number(i))
                        .toUtf8();
                QTest::newRow(name.constData())
                    << marker << language << endings.at(i) << (i >= 4);
            }
        }
    }
}

void MarkdownHighlighterTest::closingFence() {
    QFETCH(QString, marker);
    QFETCH(QString, language);
    QFETCH(QString, ending);
    QFETCH(bool, closes);

    QTextDocument document;
    MarkdownHighlighter highlighter(&document);
    QString text =
        marker + language + "\n" + marker + ending + "\n# After candidate\n";
    if (!closes) text += marker + "\n# After closing\n";
    document.setPlainText(text);
    highlighter.rehighlight();

    QVERIFY(MarkdownHighlighter::isCodeBlock(document.begin().userState()));
    const QTextBlock candidate = document.findBlockByNumber(1);
    const QTextBlock following = document.findBlockByNumber(2);
    if (closes) {
        QVERIFY(MarkdownHighlighter::isCodeBlockEnd(candidate.userState()));
        QVERIFY(MarkdownHighlighter::isHeading(following.userState()));
    } else {
        QVERIFY(MarkdownHighlighter::isCodeBlock(candidate.userState()));
        QVERIFY(MarkdownHighlighter::isCodeBlock(following.userState()));
        QVERIFY(MarkdownHighlighter::isCodeBlockEnd(
            document.findBlockByNumber(3).userState()));
        QVERIFY(MarkdownHighlighter::isHeading(
            document.findBlockByNumber(4).userState()));
    }
}

void MarkdownHighlighterTest::inlineBackticks() {
    QTextDocument document;
    MarkdownHighlighter highlighter(&document);
    document.setPlainText("```inline```\n# Heading\n");
    highlighter.rehighlight();
    QVERIFY(!MarkdownHighlighter::isCodeBlock(document.begin().userState()));
    QVERIFY(MarkdownHighlighter::isHeading(
        document.findBlockByNumber(1).userState()));
}

QTEST_MAIN(MarkdownHighlighterTest)
#include "tst_markdownhighlighter.moc"
