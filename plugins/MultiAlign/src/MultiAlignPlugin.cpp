#include "MultiAlign.h"

#include <ccMainAppInterface.h>
#include <ccPointCloud.h>
#include <ccHObjectCaster.h>
#include <ccProgressDialog.h>
#include <ccOverlayDialog.h>
#include <ccRegistrationTools.h>

#include <QAction>
#include <QMainWindow>

MultiAlignPlugin::MultiAlignPlugin(QObject* parent)
    : QObject(parent)
    , ccStdPluginInterface(":/CC/plugin/MultiAlign/info.json")
    , m_action(nullptr)
    , m_fgrAction(nullptr)
{
}

QIcon MultiAlignPlugin::getIcon() const
{
    return QIcon();
}

QList<QAction*> MultiAlignPlugin::getActions()
{
    if (!m_action)
    {
        m_action = new QAction(getName(), this);
        m_action->setToolTip(getDescription());
        connect(m_action, &QAction::triggered, this, &MultiAlignPlugin::doAction);
    }
    if (!m_fgrAction)
    {
        m_fgrAction = new QAction(tr("FGR Global Align"), this);
        m_fgrAction->setToolTip(tr("Use Open3D Fast Global Registration"));
        connect(m_fgrAction, &QAction::triggered, this, &MultiAlignPlugin::doFgrAction);
    }
    return { m_action, m_fgrAction };
}

void MultiAlignPlugin::onNewSelection(const ccHObject::Container& selectedEntities)
{
    if (!m_action || !m_fgrAction)
        return;

    unsigned cloudCount = 0;
    for (ccHObject* obj : selectedEntities)
    {
        if (ccHObjectCaster::ToPointCloud(obj))
            ++cloudCount;
    }
    bool enabled = cloudCount > 1;
    m_action->setEnabled(enabled);
    m_fgrAction->setEnabled(enabled);
}

void MultiAlignPlugin::doAction()
{
    if (!m_app)
        return;

    const ccHObject::Container& selected = m_app->getSelectedEntities();
    if (selected.size() < 2)
    {
        m_app->dispToConsole("Select at least two point clouds", ccMainAppInterface::WRN_CONSOLE_MESSAGE);
        return;
    }

    // choose first cloud as reference
    ccPointCloud* refCloud = ccHObjectCaster::ToPointCloud(selected.front());
    if (!refCloud)
    {
        m_app->dispToConsole("First selected entity is not a point cloud", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
        return;
    }

    ccGLMatrix lastTrans;
    bool first = true;

    for (size_t i = 1; i < selected.size(); ++i)
    {
        ccPointCloud* moving = ccHObjectCaster::ToPointCloud(selected[i]);
        if (!moving)
            continue;

        CCCoreLib::PointProjectionTools::Transformation transform;
        ccGLMatrix result;

        if (ccRegistrationTools::ICP(moving, refCloud, transform, nullptr, result, true, 100))
        {
            moving->applyGLTransformation_recursive(&result);
            moving->setDisplay_recursive(refCloud->getDisplay());
            if (first)
            {
                lastTrans = result;
                first = false;
            }
            else
            {
                lastTrans = result * lastTrans;
            }
        }
    }

    m_app->redrawAll();
}

void MultiAlignPlugin::doFgrAction()
{
    if (!m_app)
        return;

    const ccHObject::Container& selected = m_app->getSelectedEntities();
    if (selected.size() < 2)
    {
        m_app->dispToConsole("Select at least two point clouds", ccMainAppInterface::WRN_CONSOLE_MESSAGE);
        return;
    }

    m_app->dispToConsole("FGR alignment requires the external Open3D script", ccMainAppInterface::STD_CONSOLE_MESSAGE);
    // Placeholder: call external script or implement FGR here
}

// EOF
