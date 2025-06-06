#pragma once

#include <ViewPlugin.h>

#include <Dataset.h>
#include <widgets/DropWidget.h>

#include <PointData/PointData.h>

#include <QWidget>

/** All plugin related classes are in the ManiVault plugin namespace */
using namespace mv::plugin;

/** Drop widget used in this plugin is located in the ManiVault gui namespace */
using namespace mv::gui;

/** Dataset reference used in this plugin is located in the ManiVault util namespace */
using namespace mv::util;

class QLabel;

/**
 * Table view plugin class
 *
 * This view plugin class provides skeleton code that shows how to develop
 * a view plugin in ManiVault. It shows how to use the built-in drag and drop
 * behavior.
 *
 * To see the plugin in action, please follow the steps below:
 *
 * 1. Go to the visualization menu in ManiVault
 * 2. Choose the Table view menu item, the view will be added to the layout
 *
 * @authors J. Thijssen & T. Kroes
 */
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

protected:
    DropWidget*             _dropWidget;                /** Widget for drag and drop behavior */
    mv::Dataset<Points>     _points;                    /** Points smart pointer */
    QString                 _currentDatasetName;        /** Name of the current dataset */
    QLabel*                 _currentDatasetNameLabel;   /** Label that show the current dataset name */
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
