#include "MultiAlignTool.h"
#include "ui_multiAlignDlg.h"

#include <ccPointCloud.h>

MultiAlignTool::MultiAlignTool(QWidget* parent)
    : ccOverlayDialog(parent, Qt::Tool)
    , ui(new Ui::MultiAlignTool)
    , m_refCloud(nullptr)
{
    ui->setupUi(this);
}

MultiAlignTool::~MultiAlignTool()
{
    delete ui;
}

void MultiAlignTool::setReferenceCloud(ccPointCloud* cloud)
{
    m_refCloud = cloud;
    if (cloud)
        ui->refLabel->setText(cloud->getName());
}

ccPointCloud* MultiAlignTool::getReferenceCloud() const
{
    return m_refCloud;
}

// EOF
