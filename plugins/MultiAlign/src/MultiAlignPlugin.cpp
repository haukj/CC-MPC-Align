#include "MultiAlign.h"

#include <ccMainAppInterface.h>
#include <ccPointCloud.h>
#include <ccHObjectCaster.h>
#include <ccProgressDialog.h>
#include "MultiAlignTool.h"
#include <ccRegistrationTools.h>
#include <FileIOFilter.h>

#include <QProcess>
#include <QTemporaryDir>

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

    MultiAlignTool dlg(m_app->getMainWindow());
    dlg.setClouds(selected);
    if (dlg.exec() != QDialog::Accepted)
        return;

    ccPointCloud* refCloud = dlg.selectedReference();
    if (!refCloud)
    {
        m_app->dispToConsole("Invalid reference cloud", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
        return;
    }
    unsigned maxIter = dlg.maxIterations();

    ccGLMatrix lastTrans;
    bool first = true;

    for (ccHObject* obj : selected)
    {
        ccPointCloud* moving = ccHObjectCaster::ToPointCloud(obj);
        if (!moving || moving == refCloud)
            continue;

        CCCoreLib::PointProjectionTools::Transformation transform;
        ccGLMatrix result;

        if (ccRegistrationTools::ICP(moving, refCloud, transform, nullptr, result, true, maxIter))
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

    MultiAlignTool dlg(m_app->getMainWindow());
    dlg.setClouds(selected);
    if (dlg.exec() != QDialog::Accepted)
        return;

    ccPointCloud* refCloud = dlg.selectedReference();
    if (!refCloud)
    {
        m_app->dispToConsole("Invalid reference cloud", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
        return;
    }

    double voxel = dlg.voxelSize();

    QTemporaryDir tempDir;
    if (!tempDir.isValid())
    {
        m_app->dispToConsole("Failed to create temp directory", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
        return;
    }

    FileIOFilter::SaveParameters sp;
    sp.alwaysDisplaySaveDialog = false;

    QStringList files;
    for (ccHObject* obj : selected)
    {
        ccPointCloud* pc = ccHObjectCaster::ToPointCloud(obj);
        if (!pc)
            continue;
        QString path = tempDir.path() + QLatin1Char('/') + pc->getName() + ".ply";
        FileIOFilter::SaveToFile(pc, path, sp, QStringLiteral("PLY (*.ply)"));
        files << path;
    }

    QString script = QCoreApplication::applicationDirPath() + QLatin1String("/../plugins/MultiAlign/scripts/fgr_multi_align.py");
    QStringList args;
    args << script << QString::number(voxel);
    args.append(files);

    QProcess proc;
    proc.start(QStringLiteral("python3"), args);
    if (!proc.waitForFinished(-1))
    {
        m_app->dispToConsole("Failed to run FGR script", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
        return;
    }

    QList<QByteArray> lines = proc.readAllStandardOutput().split('\n');
    int idx = 0;
    for (ccHObject* obj : selected)
    {
        ccPointCloud* pc = ccHObjectCaster::ToPointCloud(obj);
        if (!pc)
            continue;

        if (pc == refCloud)
        {
            ++idx; // skip reference (identity transform)
            continue;
        }

        if (idx >= lines.size())
            break;

        QStringList vals = QString::fromUtf8(lines[idx]).split(' ', Qt::SkipEmptyParts);
        ++idx;
        if (vals.size() != 16)
            continue;

        double mat[16];
        for (int i = 0; i < 16; ++i)
            mat[i] = vals[i].toDouble();

        ccGLMatrix trans(mat);
        pc->applyGLTransformation_recursive(&trans);
        pc->setDisplay_recursive(refCloud->getDisplay());
    }

    m_app->redrawAll();
}

// EOF
