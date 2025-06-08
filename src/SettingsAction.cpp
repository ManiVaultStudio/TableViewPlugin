#include "SettingsAction.h"

#include <iostream>
#include <set>
#include "TableViewPlugin.h"
#include <string>
#include <QFileDialog>
#include <QPageLayout>
#include <QWebEngineView>
#include <chrono>
#include <typeinfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QSet>
#include <QJsonValue>
using namespace mv;
using namespace mv::gui;

SettingsAction::SettingsAction(TableViewPlugin& TableViewPlugin) :
    WidgetAction(&TableViewPlugin, "TableViewPlugin"),
    _viewerPlugin(TableViewPlugin),
    _datasetOptionsHolder(*this)
{
    setSerializationName("TableViewPlugin:Settings");
    _datasetOptionsHolder.getPointDatasetAction().setSerializationName("LayerSurfer:PointDataset");
    _datasetOptionsHolder.getTableDataVariantAction().setSerializationName("LayerSurfer:TableDataVariant");
    
    _datasetOptionsHolder.getPointDatasetAction().setToolTip("Point Dataset");
    _datasetOptionsHolder.getTableDataVariantAction().setToolTip("Table Data Variant");

    _datasetOptionsHolder.getPointDatasetAction().setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == PointType;
    });

    _datasetOptionsHolder.getPointDatasetAction().setDefaultWidgetFlags(OptionAction::ComboBox);
    _datasetOptionsHolder.getTableDataVariantAction().setDefaultWidgetFlags(VariantAction::TextHeuristicRole);

    _tableViewAction = new HighPerfTableView();
}

inline SettingsAction::DatasetOptionsHolder::DatasetOptionsHolder(SettingsAction& settingsAction) :
    HorizontalGroupAction(&settingsAction, "Dataset Options"),
    _settingsOptions(settingsAction),
    _pointDatasetAction(this, "Point dataset"),
    _tableDataVariant(this, "Table Data Variant")
{
    setText("Dataset1 Options");
    setIcon(mv::util::StyledIcon("database"));
    setPopupSizeHint(QSize(300, 0));
    setConfigurationFlag(WidgetAction::ConfigurationFlag::Default);
    addAction(&_pointDatasetAction);
    addAction(&_tableDataVariant);
}

void SettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);
    _datasetOptionsHolder.getPointDatasetAction().fromParentVariantMap(variantMap);
    _datasetOptionsHolder.getTableDataVariantAction().fromParentVariantMap(variantMap);
}

QVariantMap SettingsAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();
    _datasetOptionsHolder.getPointDatasetAction().insertIntoVariantMap(variantMap);
    _datasetOptionsHolder.getTableDataVariantAction().insertIntoVariantMap(variantMap);
    return variantMap;
}