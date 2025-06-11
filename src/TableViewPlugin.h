#pragma once

#include <ViewPlugin.h>
#include <Dataset.h>
#include <widgets/DropWidget.h>
#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>
#include <QWidget>
#include "HighPerfTableView.h"
#include "FastTableData.h"
#include "SettingsAction.h"

using namespace mv::plugin;
using namespace mv::gui;
using namespace mv::util;

class QLabel;

class TableViewPlugin : public ViewPlugin
{
    Q_OBJECT

public:
    TableViewPlugin(const PluginFactory* factory);
    ~TableViewPlugin() override = default;

    void init() override;
    void onDataEvent(mv::DatasetEvent* dataEvent);
    void setShowBarsForNumericalColumns(bool enabled);
    void modifyandSetNewPointData();

public:
    void fromVariantMap(const QVariantMap& variantMap) override;
    QVariantMap toVariantMap() const override;
    
protected:
    DropWidget*             _dropWidget;
    mv::Dataset<Points>     _points;
    QString                 _currentDatasetName;
    //QLabel*                 _currentDatasetNameLabel;
    SettingsAction          _settingsAction;

};

class TableViewPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(mv::plugin::ViewPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "studio.manivault.TableViewPlugin"
                      FILE  "PluginInfo.json")

public:
    TableViewPluginFactory();
    ViewPlugin* produce() override;
    mv::DataTypes supportedDataTypes() const override;
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};
