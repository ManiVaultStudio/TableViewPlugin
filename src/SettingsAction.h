#pragma once

#include <actions/WidgetAction.h>
#include <actions/IntegralAction.h>
#include <actions/OptionAction.h>
#include <actions/OptionsAction.h>
#include <actions/VariantAction.h>
#include "ClusterData/ClusterData.h"
#include "PointData/PointData.h"
#include <actions/ToggleAction.h>
#include "actions/DatasetPickerAction.h"
#include <PointData/DimensionPickerAction.h>
#include "event/EventListener.h"
#include "actions/Actions.h"
#include "Plugin.h"
#include "DataHierarchyItem.h"
#include "Set.h"
#include <AnalysisPlugin.h>
#include <memory>
#include <algorithm>    
#include <QDebug>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QGridLayout>
#include <QFormLayout>
#include <QString>
#include <string>
#include<QRadioButton>
#include <event/Event.h>
#include <PointData/DimensionPickerAction.h>
#include <QDebug>
#include <QLabel>
#include <string>
#include<actions/ColorMap1DAction.h>
#include <set>
#include <actions/HorizontalToolbarAction.h>
#include <actions/VerticalToolbarAction.h>
#include "QStatusBar"
#include "HighPerfTableView.h"

using namespace mv::gui;
class QMenu;
class TableViewPlugin;

namespace mv
{
    class CoreInterface;
}


class SettingsAction : public WidgetAction
{
public:

    class DatasetOptionsHolder : public VerticalGroupAction
    {
    public:
        DatasetOptionsHolder(SettingsAction& settingsAction);
        

        const DatasetPickerAction& getPointDatasetAction() const { return _pointDatasetAction; }
        DatasetPickerAction& getPointDatasetAction() { return _pointDatasetAction; }

        const VariantAction& getTableDataVariantAction() const { return _tableDataVariant; }
        VariantAction& getTableDataVariantAction() { return _tableDataVariant; }


    protected:
        SettingsAction& _settingsOptions;
        DatasetPickerAction                 _pointDatasetAction;
        VariantAction                       _tableDataVariant;

    };

   /* class SettingsHolder : public HorizontalGroupAction
    {
    public:
        SettingsHolder(SettingsAction& settingsAction);



    protected:
        SettingsAction& _settingsOptions;


    };*/

public:
    SettingsAction(TableViewPlugin& TableViewPlugin);


    DatasetOptionsHolder& getDatasetOptionsHolder() { return _datasetOptionsHolder; }
    //SettingsHolder& getSettingsHolder() { return _settingsHolder; }


public: // Serialization
    void fromVariantMap(const QVariantMap& variantMap) override;
    QVariantMap toVariantMap() const override;

    HighPerfTableView* getTableViewAction() { return _tableViewAction; }


protected:
    TableViewPlugin& _viewerPlugin;
    mv::CoreInterface* _core;
    DatasetOptionsHolder              _datasetOptionsHolder;

    //SettingsHolder                   _settingsHolder;
    HighPerfTableView* _tableViewAction = nullptr;


    friend class ChannelAction;
};