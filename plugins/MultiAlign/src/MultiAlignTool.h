#ifndef MULTI_ALIGN_TOOL_H
#define MULTI_ALIGN_TOOL_H

#include <QDialog>
#include <ccHObject.h>

#include <vector>

class ccPointCloud;

namespace Ui { class MultiAlignTool; }

class MultiAlignTool : public QDialog
{
    Q_OBJECT
public:
    explicit MultiAlignTool(QWidget* parent = nullptr);
    ~MultiAlignTool();

    void setClouds(const ccHObject::Container& clouds);
    ccPointCloud* selectedReference() const;
    unsigned maxIterations() const;
    double voxelSize() const;

private:
    Ui::MultiAlignTool* ui;
    std::vector<ccPointCloud*> m_clouds;
};

#endif // MULTI_ALIGN_TOOL_H
