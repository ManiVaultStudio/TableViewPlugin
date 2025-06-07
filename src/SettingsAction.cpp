#include "SettingsAction.h"

#include <iostream>
#include <set>
#include "TableViewPlugin.h"
#include<string>  
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
    _datasetOptionsHolder(*this)//,
    //_settingsHolder(*this)
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
    /*

    _tableView1Action->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _tableView1Action->setSelectionBehavior(QAbstractItemView::SelectRows);
    _tableView1Action->setSelectionMode(QAbstractItemView::SingleSelection);
    _tableView1Action->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _tableView1Action->setAlternatingRowColors(false);
    _tableView1Action->setSortingEnabled(true);
    _tableView1Action->setShowGrid(true);
    _tableView1Action->setGridStyle(Qt::SolidLine);
    _tableView1Action->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    _tableView1Action->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _tableView1Action->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _tableView1Action->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _tableView1Action->setCornerButtonEnabled(false);
    _tableView1Action->setWordWrap(false);
    _tableView1Action->setTabKeyNavigation(false);
    _tableView1Action->setAcceptDrops(false);
    _tableView1Action->setDropIndicatorShown(false);
    _tableView1Action->setDragEnabled(false);
    _tableView1Action->setDragDropMode(QAbstractItemView::NoDragDrop);
    _tableView1Action->setDragDropOverwriteMode(false);
    _tableView1Action->setAutoScroll(false);
    _tableView1Action->setAutoScrollMargin(16);
    _tableView1Action->setAutoFillBackground(true);
    _tableView1Action->setFrameShape(QFrame::NoFrame);
    _tableView1Action->setFrameShadow(QFrame::Plain);
    _tableView1Action->setLineWidth(0);
    _tableView1Action->setMidLineWidth(0);
    _tableView1Action->setFocusPolicy(Qt::NoFocus);
    _tableView1Action->setContextMenuPolicy(Qt::NoContextMenu);
    _tableView1Action->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _tableView1Action->setMinimumSize(QSize(0, 0));
    _tableView1Action->setMaximumSize(QSize(16777215, 16777215));
    _tableView1Action->setBaseSize(QSize(0, 0));
    _tableView1Action->setFocusPolicy(Qt::StrongFocus);
    _tableView1Action->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    //only highlight multiple rows if shiuft is pressed
    _tableView1Action->setSelectionBehavior(QAbstractItemView::SelectRows);

    */


}

inline SettingsAction::DatasetOptionsHolder::DatasetOptionsHolder(SettingsAction& settingsAction) :
    VerticalGroupAction(&settingsAction, "Dataset Options"),
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


/*
inline SettingsAction::SettingsHolder::SettingsHolder(SettingsAction& settingsAction) :
    VerticalGroupAction(&settingsAction, "Settings Options"),
    _settingsOptions(settingsAction)
{
    setText("Shared Options");
    setIcon(mv::util::StyledIcon("gear"));
    setPopupSizeHint(QSize(300, 0));
    setConfigurationFlag(WidgetAction::ConfigurationFlag::Default);


}
*/


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