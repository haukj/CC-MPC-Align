#include "MultiAlignTool.h"
#include "ui_MultiAlignDlg.h"

#include <ccPointCloud.h>
#include <ccHObjectCaster.h>

MultiAlignTool::MultiAlignTool(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::MultiAlignTool)
{
    ui->setupUi(this);
}

MultiAlignTool::~MultiAlignTool()
{
    delete ui;
}

void MultiAlignTool::setClouds(const ccHObject::Container& clouds)
{
    ui->refCombo->clear();
    m_clouds.clear();
    for (ccHObject* obj : clouds)
    {
        ccPointCloud* pc = ccHObjectCaster::ToPointCloud(obj);
        if (pc)
        {
            ui->refCombo->addItem(pc->getName());
            m_clouds.push_back(pc);
        }
    }
}

ccPointCloud* MultiAlignTool::selectedReference() const
{
    int idx = ui->refCombo->currentIndex();
    if (idx >= 0 && idx < static_cast<int>(m_clouds.size()))
        return m_clouds[idx];
    return nullptr;
}

unsigned MultiAlignTool::maxIterations() const
{
    return static_cast<unsigned>(ui->iterSpinBox->value());
}

double MultiAlignTool::voxelSize() const
{
    return ui->voxelSpinBox->value();
}

// EOF
