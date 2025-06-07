#ifndef MULTI_ALIGN_PLUGIN_H
#define MULTI_ALIGN_PLUGIN_H

#include <ccStdPluginInterface.h>

class QAction;

class MultiAlignPlugin : public QObject, public ccStdPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(ccPluginInterface ccStdPluginInterface)
    Q_PLUGIN_METADATA(IID "cccorp.cloudcompare.plugin.MultiAlign" FILE "../info.json")

public:
    explicit MultiAlignPlugin(QObject* parent = nullptr);

    // ccPluginInterface
    QString getName() const override { return "MultiCloud Alignment"; }
    QIcon getIcon() const override;
    QString getDescription() const override { return "Align multiple point clouds sequentially"; }
    QList<QAction *> getActions() override;

    void onNewSelection(const ccHObject::Container& selectedEntities) override;

private slots:
    void doAction();
    void doFgrAction();

private:
    QAction* m_action;
    QAction* m_fgrAction;
};

#endif // MULTI_ALIGN_PLUGIN_H
