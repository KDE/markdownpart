#include "markdownvisitor.h"
#include <QRegularExpression>
#include <QProcess>
#include <QSvgRenderer>
#include <QDebug>


#include <QSvgGenerator>
#include <QBuffer>
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>

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
    constexpr int oversample = 8;
    constexpr float baseTextSize = 20.0f; // slightly larger base → heavier strokes at display size
    constexpr float renderTextSize = baseTextSize * oversample; // 160pt
    tex::TeXRender* render = nullptr;
    try {
        render = tex::LaTeX::parse(tex, 0, renderTextSize, renderTextSize, 0xff000000);
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
        
        // Fill with the body background colour (#FAFAFA from markdownpart.css)
        // so the formula blends seamlessly without transparent-edge fading.
        QPixmap pixmap(physicalWidth, physicalHeight);
        pixmap.fill(QColor(QStringLiteral("#FAFAFA")));
        
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        
        tex::Graphics2D_qt g2(&painter);
        render->draw(g2, padding, padding);
        painter.end();
        QImage image = pixmap.toImage();
        
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        // One-step smooth-scale from 8× physical to 1× logical, embedded at
        // natural size (no width/height override). QTextDocument displays at
        // pixel-for-pixel size with no further scaling, avoiding double-blur.
        int logicalWidth = physicalWidth / oversample;
        int logicalHeight = physicalHeight / oversample;
        QImage scaledImage = image.scaled(logicalWidth, logicalHeight,
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
        scaledImage.save(&buffer, "PNG");
        
        QByteArray base64Img = ba.toBase64();
        QString imgTag = QStringLiteral("<img src=\"data:image/png;base64,") + QString::fromUtf8(base64Img) +
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
        int logicalWidth = 0, logicalHeight = 0;
        
        if (syntax == QStringLiteral("mermaid")) {
            QByteArray svgData = runMermaidWeb(c->text());
            if (!svgData.isEmpty()) {
                QString svgStr = QString::fromUtf8(fixMermaidSvgText(svgData));
                
                // Fix QSvgRenderer not supporting rgba() colors which makes label backgrounds solid black
                QRegularExpression rgbaRe(QStringLiteral(R"(rgba\([^)]+\))"));
                svgStr.replace(rgbaRe, QStringLiteral("#E8E8E8"));
                
                // Pass scale=2 for High-DPI
                imgData = svgToHighDpiPng(svgStr.toUtf8(), 2.0f, logicalWidth, logicalHeight);
            }
        } else {
            QByteArray svgData = runPlantUmlWeb(c->text());
            if (!svgData.isEmpty()) {
                QString svgStr = QString::fromUtf8(svgData);
                // PlantUML uses stroke-width:0.5 for lifelines and borders, which become barely visible 
                // faint lines when rasterized and downscaled. Thicken them to 1.0.
                QRegularExpression strokeRe(QStringLiteral(R"(stroke-width:0\.[0-9]+)"));
                svgStr.replace(strokeRe, QStringLiteral("stroke-width:1.0"));
                
                imgData = svgToHighDpiPng(svgStr.toUtf8(), 2.0f, logicalWidth, logicalHeight);
            }
        }

        if (!imgData.isEmpty()) {
            QByteArray base64Img = imgData.toBase64();
            QString imgTag;
            if (logicalWidth > 0 && logicalHeight > 0) {
                imgTag = QStringLiteral("<img src=\"data:image/png;base64,") + QString::fromUtf8(base64Img) +
                         QStringLiteral("\" width=\"%1\" height=\"%2\" />").arg(logicalWidth).arg(logicalHeight);
            } else {
                imgTag = QStringLiteral("<img src=\"data:image/png;base64,") + QString::fromUtf8(base64Img) +
                         QStringLiteral("\" />");
            }
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
    // Request transparent SVG from mermaid.ink. We must use SVG and convert it locally 
    // because their PNG /img/ endpoint does not support transparency (returns JPEG).
    QString config = QStringLiteral("%%{init: {\"flowchart\": {\"htmlLabels\": false}, \"sequence\": {\"htmlLabels\": false}, \"gantt\": {\"htmlLabels\": false}, \"journey\": {\"htmlLabels\": false}, \"class\": {\"htmlLabels\": false}, \"state\": {\"htmlLabels\": false}, \"er\": {\"htmlLabels\": false}, \"pie\": {\"htmlLabels\": false}, \"c4\": {\"htmlLabels\": false}, \"themeVariables\": {\"background\": \"transparent\"}}}%%\n");
    QString fullCode = config + code;
    QByteArray base64 = fullCode.toUtf8().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    QString url = QStringLiteral("https://mermaid.ink/svg/") + QString::fromUtf8(base64);

    QNetworkAccessManager manager;
    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = manager.get(request);
    
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    QByteArray data;
    if (reply->error() == QNetworkReply::NoError) {
        data = reply->readAll();
    }
    reply->deleteLater();
    return data;
}

QByteArray MarkdownVisitor::runPlantUmlWeb(const QString& code)
{
    // Request transparent PNG directly from plantuml.com
    QString modifiedCode = code;
    int startIdx = modifiedCode.indexOf(QStringLiteral("@startuml"));
    if (startIdx != -1) {
        modifiedCode.insert(startIdx + 9, QStringLiteral("\nskinparam backgroundColor transparent\n"));
    } else {
        modifiedCode.prepend(QStringLiteral("skinparam backgroundColor transparent\n"));
    }

    QString hexStr = QStringLiteral("~h") + QString::fromUtf8(modifiedCode.toUtf8().toHex());
    QString url = QStringLiteral("http://www.plantuml.com/plantuml/svg/") + hexStr;

    QNetworkAccessManager manager;
    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = manager.get(request);
    
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    QByteArray data;
    if (reply->error() == QNetworkReply::NoError) {
        data = reply->readAll();
    }
    reply->deleteLater();
    return data;
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
    // Use transparent #FAFAFA (Kate background) instead of transparent black.
    // This prevents a dark fringe/halo when the subpixel antialiasing is composited.
    image.fill(QColor(0xFA, 0xFA, 0xFA, 0));
    
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    renderer.render(&painter);
    painter.end();
    
    // We do NOT scale back to 1x here! We save the high-res image directly,
    // and rely on the HTML <img width="X" height="Y"> attributes to scale it
    // visually. This guarantees crispness on high-DPI screens without QTextDocument double lines!
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    
    return ba;
}
