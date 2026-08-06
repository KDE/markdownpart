#include "markdownvisitor.h"
#include <QRegularExpression>
#include <QProcess>
#include <QDebug>


#include <QSvgGenerator>
#include <QBuffer>
#include <QPainter>
#include <QDebug>

// MicroTeX includes
#include "latex.h"
#include "platform/qt/graphic_qt.h"

MarkdownVisitor::MarkdownVisitor() : MD::details::HtmlVisitor() {
}

MarkdownVisitor::~MarkdownVisitor() {
}

void MarkdownVisitor::onMath(MD::Math *m) {
    if (!m) return;
    
    std::wstring tex = m->text().toStdWString();
    
    // Parse the LaTeX string using MicroTeX
    // width: 800 (or 0 for wrap?), textSize: 12.0f, lineSpace: 12.0f, color: black
    tex::TeXRender* render = nullptr;
    try {
        render = tex::LaTeX::parse(tex, 0, 16.0f, 16.0f, 0xff000000);
    } catch (const std::exception& e) {
        qWarning() << "LaTeX parsing error:" << e.what();
    }
    
    if (render) {
        QBuffer buffer;
        QSvgGenerator svgGen;
        svgGen.setOutputDevice(&buffer);
        
        int width = render->getWidth();
        int height = render->getHeight();
        if (width <= 0) width = 1;
        if (height <= 0) height = 1;
        
        svgGen.setSize(QSize(width, height));
        svgGen.setViewBox(QRectF(0, 0, width, height));
        
        QPainter painter(&svgGen);
        tex::Graphics2D_qt g2(&painter);
        render->draw(g2, 0, 0);
        painter.end();
        
        QString svgString = QString::fromUtf8(buffer.data());
        
        if (m->isInline()) {
            m_html += QStringLiteral("<span class=\"math inline\">") + svgString + QStringLiteral("</span>");
        } else {
            m_html += QStringLiteral("<div class=\"math block\" style=\"text-align: center;\">") + svgString + QStringLiteral("</div>");
        }
        
        delete render;
    } else {
        // Fallback if parsing fails
        if (m->isInline()) {
            m_html += QStringLiteral("<code class=\"math inline\">") + prepareTextForHtml(m->text()) + QStringLiteral("</code>");
        } else {
            m_html += QStringLiteral("<pre class=\"math block\"><code>") + prepareTextForHtml(m->text()) + QStringLiteral("</code></pre>\n");
        }
    }
}


void MarkdownVisitor::onCode(MD::Code *c)
{
    QString syntax = c->syntax().toLower();
    if (c->isFensedCode() && (syntax == QStringLiteral("mermaid") || syntax == QStringLiteral("plantuml") || syntax == QStringLiteral("puml"))) {
        QByteArray svgData;
        if (syntax == QStringLiteral("mermaid")) {
            svgData = runMermaidWeb(c->text());
            svgData = fixMermaidSvgText(svgData);
        } else {
            svgData = runPlantUmlWeb(c->text());
        }

        if (!svgData.isEmpty()) {
            m_html.append(QStringLiteral("<div class=\"%1-diagram\">\n").arg(syntax));
            m_html.append(QString::fromUtf8(svgData));
            m_html.append(QStringLiteral("</div>\n"));
            return;
        }
    }
    // Fallback to original md4qt rendering if not mermaid/plantuml or if fetching failed
    MD::details::HtmlVisitor::onCode(c);
}

QByteArray MarkdownVisitor::runMermaidWeb(const QString& code)
{
    QByteArray base64 = code.toUtf8().toBase64();
    QString url = QStringLiteral("https://mermaid.ink/svg/") + QString::fromUtf8(base64.toPercentEncoding());

    QProcess proc;
    QStringList args;
    args << QStringLiteral("-s") << QStringLiteral("-f") << QStringLiteral("--max-time") << QStringLiteral("15") << url;
    proc.start(QStringLiteral("curl"), args);
    if (proc.waitForStarted() && proc.waitForFinished(15000)) {
        if (proc.exitCode() == 0) {
            return proc.readAllStandardOutput();
        }
    }
    return QByteArray();
}

QByteArray MarkdownVisitor::runPlantUmlWeb(const QString& code)
{
    QString hexStr = QStringLiteral("~h") + QString::fromUtf8(code.toUtf8().toHex());
    QString url = QStringLiteral("http://www.plantuml.com/plantuml/svg/") + hexStr;

    QProcess proc;
    QStringList args;
    args << QStringLiteral("-s") << QStringLiteral("-f") << QStringLiteral("--max-time") << QStringLiteral("15") << url;
    proc.start(QStringLiteral("curl"), args);
    if (proc.waitForStarted() && proc.waitForFinished(15000)) {
        if (proc.exitCode() == 0) {
            return proc.readAllStandardOutput();
        }
    }
    return QByteArray();
}

QByteArray MarkdownVisitor::fixMermaidSvgText(const QByteArray& svgData)
{
    QString svgStr = QString::fromUtf8(svgData);
    
    QRegularExpression textTspanRe(QStringLiteral(R"(<text\b([^>]*)>\s*<tspan\b([^>]*)>)"));
    QRegularExpressionMatchIterator it = textTspanRe.globalMatch(svgStr);
    QList<QRegularExpressionMatch> matches;
    while (it.hasNext()) {
        matches.append(it.next());
    }
    
    QRegularExpression yRe(QStringLiteral(R"(\by\s*=\s*"(-?[0-9]*\.?[0-9]+)em")"));
    QRegularExpression dyRe(QStringLiteral(R"(\bdy\s*=\s*"(-?[0-9]*\.?[0-9]+)em")"));
    QRegularExpression textYRe(QStringLiteral(R"(\by\s*=\s*"[^"]*")"));
    
    for (int i = matches.size() - 1; i >= 0; --i) {
        const QRegularExpressionMatch& match = matches.at(i);
        QString textAttrs = match.captured(1);
        QString tspanAttrs = match.captured(2);
        
        QRegularExpressionMatch yMatch = yRe.match(tspanAttrs);
        QRegularExpressionMatch dyMatch = dyRe.match(tspanAttrs);
        
        if (yMatch.hasMatch() && dyMatch.hasMatch()) {
            double yEm = yMatch.captured(1).toDouble();
            double dyEm = dyMatch.captured(1).toDouble();
            double baselinePx = (yEm + dyEm) * 16.0 - 2.0;
            
            QRegularExpressionMatch textYMatch = textYRe.match(textAttrs);
            if (textYMatch.hasMatch()) {
                textAttrs.replace(textYMatch.capturedStart(0), textYMatch.capturedLength(0), 
                                  QStringLiteral("y=\"%1\"").arg(baselinePx, 0, 'f', 2));
            } else {
                textAttrs = QStringLiteral(" y=\"%1\"").arg(baselinePx, 0, 'f', 2) + textAttrs;
            }
            
            tspanAttrs.remove(yRe);
            tspanAttrs.remove(dyRe);
            
            tspanAttrs = tspanAttrs.simplified();
            if (!tspanAttrs.isEmpty() && !tspanAttrs.startsWith(QStringLiteral(" "))) {
                tspanAttrs.prepend(QLatin1Char(' '));
            }
            
            QString replacement = QStringLiteral("<text%1><tspan%2>").arg(textAttrs).arg(tspanAttrs);
            svgStr.replace(match.capturedStart(0), match.capturedLength(0), replacement);
        }
    }
    
    QRegularExpression emRe(QStringLiteral(R"(\b(y|dy)\s*=\s*"(-?[0-9]*\.?[0-9]+)em")"));
    QRegularExpressionMatchIterator emIt = emRe.globalMatch(svgStr);
    QList<QRegularExpressionMatch> emMatches;
    while (emIt.hasNext()) {
        emMatches.append(emIt.next());
    }
    
    for (int i = emMatches.size() - 1; i >= 0; --i) {
        const QRegularExpressionMatch& match = emMatches.at(i);
        QString attr = match.captured(1);
        double emValue = match.captured(2).toDouble();
        double pxValue = emValue * 16.0;
        
        QString replacement = QStringLiteral("%1=\"%2\"").arg(attr).arg(pxValue, 0, 'f', 2);
        svgStr.replace(match.capturedStart(0), match.capturedLength(0), replacement);
    }
    
    return svgStr.toUtf8();
}
