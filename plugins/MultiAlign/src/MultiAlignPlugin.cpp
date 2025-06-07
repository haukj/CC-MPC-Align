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
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>

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
    bool save = dlg.saveTransforms();

    std::vector<ccPointCloud*> clouds;
    clouds.push_back(refCloud);
    for (ccHObject* obj : selected)
    {
        ccPointCloud* pc = ccHObjectCaster::ToPointCloud(obj);
        if (!pc || pc == refCloud)
            continue;
        clouds.push_back(pc);
    }

    QStringList transformLines;
    QString identity;
    for (int i = 0; i < 16; ++i)
        identity += QString::number(i % 5 == 0 ? 1.0 : 0.0, 'f', 6) + ' ';
    transformLines << identity.trimmed();

    for (size_t i = 1; i < clouds.size(); ++i)
    {
        ccPointCloud* moving = clouds[i];

        CCCoreLib::PointProjectionTools::Transformation transform;
        ccGLMatrix result;

        if (ccRegistrationTools::ICP(moving, refCloud, transform, nullptr, result, true, maxIter))
        {
            moving->applyGLTransformation_recursive(&result);
            moving->setDisplay_recursive(refCloud->getDisplay());

            QString line;
            const float* data = result.data();
            for (int j = 0; j < 16; ++j)
                line += QString::number(static_cast<double>(data[j]), 'f', 6) + ' ';
            transformLines << line.trimmed();
        }
    }

    if (save)
    {
        QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        path = QDir(path).filePath(QStringLiteral("alignment_transforms.txt"));
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            for (const QString& l : transformLines)
                out << l << '\n';
        }
        else
        {
            m_app->dispToConsole(QStringLiteral("Failed to write %1").arg(path), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
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
    bool save = dlg.saveTransforms();
    QStringList transformLines;

    std::vector<ccPointCloud*> clouds;
    clouds.push_back(refCloud);
    for (ccHObject* obj : selected)
    {
        ccPointCloud* pc = ccHObjectCaster::ToPointCloud(obj);
        if (!pc || pc == refCloud)
            continue;
        clouds.push_back(pc);
    }

    QTemporaryDir tempDir;
    if (!tempDir.isValid())
    {
        m_app->dispToConsole("Failed to create temp directory", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
        return;
    }

    FileIOFilter::SaveParameters sp;
    sp.alwaysDisplaySaveDialog = false;

    QStringList files;
    for (ccPointCloud* pc : clouds)
    {
        QString path = tempDir.path() + QLatin1Char('/') + pc->getName() + ".ply";
        FileIOFilter::SaveToFile(pc, path, sp, QStringLiteral("PLY (*.ply)"));
        files << path;
    }

    QString script = QCoreApplication::applicationDirPath() + QLatin1String("/../plugins/MultiAlign/scripts/fgr_multi_align.py");
    QStringList args;
    args << script << QString::number(voxel);
    args.append(files);

    QProcess proc;
    QString cmd = QStringLiteral("python3");
    proc.start(cmd, args);
    if (!proc.waitForStarted())
    {
        m_app->dispToConsole(QStringLiteral("Failed to start FGR script: %1 %2").arg(cmd, args.join(' ')), ccMainAppInterface::ERR_CONSOLE_MESSAGE);
        return;
    }
    if (!proc.waitForFinished(-1))
    {
        m_app->dispToConsole(QStringLiteral("FGR script did not finish"), ccMainAppInterface::ERR_CONSOLE_MESSAGE);
        return;
    }
    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0)
    {
        QString err = QString::fromUtf8(proc.readAllStandardError());
        m_app->dispToConsole(QStringLiteral("FGR script failed (code %1): %2\nCommand: %3 %4")
                                 .arg(proc.exitCode())
                                 .arg(err.trimmed())
                                 .arg(cmd)
                                 .arg(args.join(' ')),
                             ccMainAppInterface::ERR_CONSOLE_MESSAGE);
        return;
    }

    QList<QByteArray> rawOutput = proc.readAllStandardOutput().split('\n');
    for (const QByteArray& line : rawOutput)
    {
        QString trimmed = QString::fromUtf8(line).trimmed();
        if (!trimmed.isEmpty())
            transformLines << trimmed;
    }

    for (size_t i = 1; i < clouds.size() && i < static_cast<size_t>(transformLines.size()); ++i)
    {
        QStringList vals = transformLines[int(i)].split(' ', Qt::SkipEmptyParts);
        if (vals.size() != 16)
            continue;

        double mat[16];
        for (int j = 0; j < 16; ++j)
            mat[j] = vals[j].toDouble();

        ccGLMatrix trans(mat);
        clouds[i]->applyGLTransformation_recursive(&trans);
        clouds[i]->setDisplay_recursive(refCloud->getDisplay());
    }

    if (save)
    {
        QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        path = QDir(path).filePath(QStringLiteral("alignment_transforms.txt"));
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            for (const QString& l : transformLines)
                out << l << '\n';
        }
        else
        {
            m_app->dispToConsole(QStringLiteral("Failed to write %1").arg(path), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
        }
    }

    m_app->redrawAll();
}

// EOF
