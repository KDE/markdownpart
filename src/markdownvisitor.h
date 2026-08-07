#pragma once

#include "html.h"

class MarkdownVisitor : public MD::details::HtmlVisitor {
public:
    MarkdownVisitor();
    ~MarkdownVisitor() override;

protected:
    void onMath(MD::Math *m) override;
    void onCode(MD::Code *c) override;

private:
    QByteArray runMermaidWeb(const QString& code);
    QByteArray runPlantUmlWeb(const QString& code);
    QByteArray fixMermaidSvgText(const QByteArray& svgData);
};
