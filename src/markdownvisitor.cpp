#include "markdownvisitor.h"
#include <QRegularExpression>
#include <QProcess>
#include <QSvgRenderer>
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
    
    // Parse the LaTeX string using MicroTeX.
    // textSize is in points; we oversample for smooth downscaling.
    // PIXELS_PER_POINT=1.0, so getWidth()/getHeight() return pixels at textSize.
    constexpr int oversample = 6;
    constexpr float baseTextSize = 16.0f; // logical display size in points/px
    constexpr float renderTextSize = baseTextSize * oversample; // 96pt
    tex::TeXRender* render = nullptr;
    try {
        render = tex::LaTeX::parse(tex, 0, renderTextSize, renderTextSize, 0xff1a2b3c);
    } catch (const std::exception& e) {
        qWarning() << "LaTeX parsing error:" << e.what();
    }

    if (render) {
        // getWidth()/getHeight() already incorporate renderTextSize.
        // Add padding scaled to the oversample factor.
        int padding = 4 * oversample;
        int physicalWidth = render->getWidth() + padding * 2;
        int physicalHeight = render->getHeight() + render->getDepth() + padding * 2;
        
        if (physicalWidth <= 0) physicalWidth = 1;
        if (physicalHeight <= 0) physicalHeight = 1;
        
        QImage image(physicalWidth, physicalHeight, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        
        tex::Graphics2D_qt g2(&painter);
        render->draw(g2, padding, padding);
        painter.end();
        
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        // Smooth-scale from the 6× render to 2× before encoding, giving
        // QTextDocument a high-quality source image to display at 1× size.
        int logicalWidth = physicalWidth / oversample;
        int logicalHeight = physicalHeight / oversample;
        QImage scaledImage = image.scaled(logicalWidth * 2, logicalHeight * 2,
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
        scaledImage.save(&buffer, "PNG");
        
        QByteArray base64Img = ba.toBase64();
        // Embed at 2× pixels, display at 1× via width/height.
        QString imgTag = QStringLiteral("<img src=\"data:image/png;base64,") + QString::fromUtf8(base64Img) +
                         QStringLiteral("\" width=\"") + QString::number(logicalWidth) +
                         QStringLiteral("\" height=\"") + QString::number(logicalHeight) +
                         QStringLiteral("\" />");
        
        if (m->isInline()) {
            m_html += QStringLiteral("<span class=\"math inline\">") + imgTag + QStringLiteral("</span>");
        } else {
            m_html += QStringLiteral("<p align=\"center\">") + imgTag + QStringLiteral("</p>");
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
        QByteArray imgData;
        if (syntax == QStringLiteral("mermaid")) {
            imgData = runMermaidWeb(c->text());
            if (!imgData.isEmpty()) {
                imgData = fixMermaidSvgText(imgData);
            }
        } else {
            imgData = runPlantUmlWeb(c->text());
        }

        if (!imgData.isEmpty()) {
            // Pre-render the SVG to a 4× PNG with antialiasing enabled, then
            // embed it with explicit 1× display dimensions. QTextDocument will
            // smooth-scale the high-res source down for crisp display.
            QSvgRenderer renderer(imgData);
            QSize svgSize = renderer.defaultSize();
            if (!svgSize.isValid() || svgSize.isEmpty()) {
                svgSize = QSize(600, 400);
            }
            constexpr int scale = 4;
            QImage svgImage(svgSize * scale, QImage::Format_ARGB32_Premultiplied);
            svgImage.fill(Qt::white);
            QPainter svgPainter(&svgImage);
            svgPainter.setRenderHint(QPainter::Antialiasing);
            svgPainter.setRenderHint(QPainter::SmoothPixmapTransform);
            svgPainter.setRenderHint(QPainter::TextAntialiasing);
            renderer.render(&svgPainter);
            svgPainter.end();
            // Embed the 4× PNG directly — QTextDocument's QPainter uses
            // SmoothPixmapTransform when drawing images, so the 4:1 downscale
            // to the display dimensions is handled with high quality.
            QByteArray pngData;
            QBuffer pngBuf(&pngData);
            pngBuf.open(QIODevice::WriteOnly);
            svgImage.save(&pngBuf, "PNG");
            QByteArray base64Img = pngData.toBase64();
            // Embed at 2× pixels, display at 1× via width/height.
            QString imgTag = QStringLiteral("<img src=\"data:image/png;base64,") + QString::fromUtf8(base64Img) +
                             QStringLiteral("\" width=\"") + QString::number(svgSize.width()) +
                             QStringLiteral("\" height=\"") + QString::number(svgSize.height()) +
                             QStringLiteral("\" />");
            m_html.append(QStringLiteral("<p align=\"center\">\n"));
            m_html.append(imgTag);
            m_html.append(QStringLiteral("</p>\n"));
            return;
        }
    }
    // Fallback to original md4qt rendering if not mermaid/plantuml or if fetching failed
    MD::details::HtmlVisitor::onCode(c);
}

QByteArray MarkdownVisitor::runMermaidWeb(const QString& code)
{
    QString config = QStringLiteral("%%{init: {\"flowchart\": {\"htmlLabels\": false}, \"sequence\": {\"htmlLabels\": false}, \"gantt\": {\"htmlLabels\": false}, \"journey\": {\"htmlLabels\": false}, \"class\": {\"htmlLabels\": false}, \"state\": {\"htmlLabels\": false}, \"er\": {\"htmlLabels\": false}, \"pie\": {\"htmlLabels\": false}, \"c4\": {\"htmlLabels\": false}}}%%\n");
    QString fullCode = config + code;
    QByteArray base64 = fullCode.toUtf8().toBase64();
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
    
    QRegularExpression foreignObjRe(QStringLiteral("<foreignObject\\s+width=\"([^\"]+)\"\\s+height=\"([^\"]+)\"[^>]*>.*?<span[^>]*>(?:<p>)?(.*?)(?:</p>)?</span>.*?</foreignObject>"));
    QRegularExpressionMatchIterator foreignIt = foreignObjRe.globalMatch(svgStr);
    QList<QRegularExpressionMatch> foreignMatches;
    while (foreignIt.hasNext()) {
        foreignMatches.append(foreignIt.next());
    }
    for (int i = foreignMatches.size() - 1; i >= 0; --i) {
        const QRegularExpressionMatch& match = foreignMatches.at(i);
        double w = match.captured(1).toDouble();
        double h = match.captured(2).toDouble();
        QString text = match.captured(3);
        
        QString replacement = QStringLiteral(R"(<text x="%1" y="%2" dominant-baseline="middle" text-anchor="middle" font-family="sans-serif" font-size="14px" fill="#333">%3</text>)").arg(w / 2.0).arg(h / 2.0 + 2.0).arg(text);
        svgStr.replace(match.capturedStart(0), match.capturedLength(0), replacement);
    }
    
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

QByteArray MarkdownVisitor::svgToHighDpiPng(const QByteArray& svgData, float scale, int& logicalWidth, int& logicalHeight)
{
    QSvgRenderer renderer(svgData);
    if (!renderer.isValid()) return QByteArray();
    
    QSize defaultSize = renderer.defaultSize();
    if (defaultSize.isEmpty()) {
        QRectF viewBox = renderer.viewBoxF();
        if (!viewBox.isEmpty()) {
            defaultSize = viewBox.size().toSize();
        } else {
            defaultSize = QSize(800, 600);
        }
    }
    
    logicalWidth = defaultSize.width();
    logicalHeight = defaultSize.height();
    
    QSize scaledSize = defaultSize * scale;
    QImage image(scaledSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    renderer.render(&painter);
    painter.end();
    
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    
    return ba;
}
