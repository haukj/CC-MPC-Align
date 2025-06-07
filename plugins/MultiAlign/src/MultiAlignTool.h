#ifndef MULTI_ALIGN_TOOL_H
#define MULTI_ALIGN_TOOL_H

#include <ccOverlayDialog.h>

namespace Ui { class MultiAlignTool; }

class MultiAlignTool : public ccOverlayDialog
{
    Q_OBJECT
public:
    explicit MultiAlignTool(QWidget* parent = nullptr);
    ~MultiAlignTool();

    void setReferenceCloud(ccPointCloud* cloud);
    ccPointCloud* getReferenceCloud() const;

private:
    Ui::MultiAlignTool* ui;
    ccPointCloud* m_refCloud;
};

#endif // MULTI_ALIGN_TOOL_H
