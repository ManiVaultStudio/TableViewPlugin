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

/** All plugin related classes are in the ManiVault plugin namespace */
using namespace mv::plugin;

/** Drop widget used in this plugin is located in the ManiVault gui namespace */
using namespace mv::gui;

/** Dataset reference used in this plugin is located in the ManiVault util namespace */
using namespace mv::util;

class QLabel;

class TableViewPlugin : public ViewPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param factory Pointer to the plugin factory
     */
    TableViewPlugin(const PluginFactory* factory);

    /** Destructor */
    ~TableViewPlugin() override = default;
    
    /** This function is called by the core after the view plugin has been created */
    void init() override;

    /**
     * Invoked when a data event occurs
     * @param dataEvent Data event which occurred
     */
    void onDataEvent(mv::DatasetEvent* dataEvent);

    /**
     * Enable or disable bar rendering for numerical columns
     * @param enabled True to enable, false to disable
     */
    void setShowBarsForNumericalColumns(bool enabled);
    void modifyandSetPointData();
protected:
    DropWidget*             _dropWidget;                /** Widget for drag and drop behavior */
    mv::Dataset<Points>     _points;                 /** Points smart pointer */
    QString                 _currentDatasetName;        /** Name of the current dataset */
    QLabel*                 _currentDatasetNameLabel;   /** Label that show the current dataset name */
    HighPerfTableView*      _tableView = nullptr;       /** High-performance table view for large datasets */
};

/**
 * Table view plugin factory class
 *
 * Note: Factory does not need to be altered (merely responsible for generating new plugins when requested)
 */
class TableViewPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(mv::plugin::ViewPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "studio.manivault.TableViewPlugin"
                      FILE  "PluginInfo.json")

public:

    /** Default constructor */
    TableViewPluginFactory();

    /** Creates an instance of the table view plugin */
    ViewPlugin* produce() override;

    /** Returns the data types that are supported by the table view plugin */
    mv::DataTypes supportedDataTypes() const override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};
