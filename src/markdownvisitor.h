#pragma once

#include "html.h"

class MarkdownVisitor : public MD::details::HtmlVisitor {
public:
    MarkdownVisitor();
    ~MarkdownVisitor() override;

protected:
    void onMath(MD::Math *m) override;
};
