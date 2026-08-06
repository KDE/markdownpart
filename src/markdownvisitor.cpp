#include "markdownvisitor.h"

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
