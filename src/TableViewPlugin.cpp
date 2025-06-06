#include "TableViewPlugin.h"
#include "HighPerfTableView.h"
#include "FastTableData.h"
#include "TableDataUtils.h" 

#include <event/Event.h>
#include <DatasetsMimeData.h>
#include <QDebug>
#include <QMimeData>

Q_PLUGIN_METADATA(IID "studio.manivault.TableViewPlugin")

using namespace mv;
using namespace mv::gui;

TableViewPlugin::TableViewPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _dropWidget(nullptr),
    _points(),
    _currentDatasetName(),
    _currentDatasetNameLabel(new QLabel()),
    _tableView(new HighPerfTableView())
{
    _currentDatasetNameLabel->setAcceptDrops(true);
    _currentDatasetNameLabel->setAlignment(Qt::AlignCenter);
    getLearningCenterAction().addVideos(QStringList({ "Practitioner", "Developer" }));
}

void TableViewPlugin::init()
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_currentDatasetNameLabel);
    layout->addWidget(_tableView);
    getWidget().setLayout(layout);

    _dropWidget = new DropWidget(_currentDatasetNameLabel);
    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(
        &getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));

    _dropWidget->initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions {
        DropWidget::DropRegions dropRegions;
        const auto datasetsMimeData = dynamic_cast<const DatasetsMimeData*>(mimeData);

        if (datasetsMimeData == nullptr)
            return dropRegions;

        if (datasetsMimeData->getDatasets().count() > 1)
            return dropRegions;

        const auto dataset = datasetsMimeData->getDatasets().first();
        const auto datasetGuiName = dataset->getGuiName();
        const auto datasetId = dataset->getId();
        const auto dataType = dataset->getDataType();
        const auto dataTypes = DataTypes({ PointType });

        if (!dataTypes.contains(dataType)) {
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", "exclamation-circle", false);
        } else {
            auto candidateDataset = mv::data().getDataset<Points>(datasetId);
            if (dataType == PointType) {
                const auto description = QString("Load %1 into table view").arg(datasetGuiName);
                if (_points == candidateDataset) {
                    dropRegions << new DropWidget::DropRegion(this, "Warning", "Data already loaded", "exclamation-circle", false);
                } else {
                    dropRegions << new DropWidget::DropRegion(this, "Points", description, "map-marker-alt", true, [this, candidateDataset]() {
                        _points = candidateDataset;
                        modifyandSetPointData();
                    });
                }
            }
        }
        return dropRegions;
    });

    connect(&_points, &Dataset<Points>::guiNameChanged, this, [this]() {
        auto newDatasetName = _points->getGuiName();
        _currentDatasetNameLabel->setText(QString("Current points dataset: %1").arg(newDatasetName));
        _dropWidget->setShowDropIndicator(newDatasetName.isEmpty());
        modifyandSetPointData();
    });

    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetAdded));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataChanged));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetRemoved));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
    _eventListener.registerDataEventByType(PointType, std::bind(&TableViewPlugin::onDataEvent, this, std::placeholders::_1));

    _tableView->setBarDelegateForNumericalColumns(true);
}

void TableViewPlugin::setShowBarsForNumericalColumns(bool enabled)
{
    if (_tableView)
        _tableView->setBarDelegateForNumericalColumns(enabled);
}

void TableViewPlugin::modifyandSetPointData()
{
    if (_points.isValid()) {
        int numOfDims = _points->getNumDimensions();
        int numOfRows = _points->getNumPoints();
        std::vector<QString> columnNames = _points->getDimensionNames();
        std::vector<int> columnIndices;
        for (int i = 0; i < numOfDims; ++i) {
            columnIndices.push_back(i);
        }
        std::vector<float> xData(numOfRows * numOfDims);
        _points->populateDataForDimensions(xData, columnIndices);

        FastTableData fastData = createTableFromDatasetData(xData, numOfRows, columnNames);
        _tableView->setData(fastData);
        qDebug() << "Table data set with" << numOfRows << "rows and" << numOfDims << "columns.";
    } else {
        _tableView->setData(FastTableData());
    }
}

void TableViewPlugin::onDataEvent(mv::DatasetEvent* dataEvent)
{
    const auto changedDataSet = dataEvent->getDataset();
    const auto datasetGuiName = changedDataSet->getGuiName();

    switch (dataEvent->getType()) {
        case EventType::DatasetAdded:
        {
            const auto dataAddedEvent = static_cast<DatasetAddedEvent*>(dataEvent);
            qDebug() << datasetGuiName << "was added";
            break;
        }
        case EventType::DatasetDataChanged:
        {
            const auto dataChangedEvent = static_cast<DatasetDataChangedEvent*>(dataEvent);
            qDebug() << datasetGuiName << "data changed";
            break;
        }
        case EventType::DatasetRemoved:
        {
            const auto dataRemovedEvent = static_cast<DatasetRemovedEvent*>(dataEvent);
            qDebug() << datasetGuiName << "was removed";
            break;
        }
        case EventType::DatasetDataSelectionChanged:
        {
            const auto dataSelectionChangedEvent = static_cast<DatasetDataSelectionChangedEvent*>(dataEvent);
            const auto& selectionSet = changedDataSet->getSelection<Points>();
            qDebug() << datasetGuiName << "selection has changed";
            break;
        }
        default:
            break;
    }
}

TableViewPluginFactory::TableViewPluginFactory()
{
    getPluginMetadata().setDescription("Table view plugin");
    getPluginMetadata().setSummary("This table shows how to implement a basic view plugin in ManiVault Studio.");
    getPluginMetadata().setCopyrightHolder({ "BioVault (Biomedical Visual Analytics Unit LUMC - TU Delft)" });
    getPluginMetadata().setAuthors({
	});
    getPluginMetadata().setOrganizations({
        { "LUMC", "Leiden University Medical Center", "https://www.lumc.nl/en/" },
        { "TU Delft", "Delft university of technology", "https://www.tudelft.nl/" }
	});
    getPluginMetadata().setLicenseText("This plugin is distributed under the [LGPL v3.0](https://www.gnu.org/licenses/lgpl-3.0.en.html) license.");
}

ViewPlugin* TableViewPluginFactory::produce()
{
    return new TableViewPlugin(this);
}

mv::DataTypes TableViewPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}

mv::gui::PluginTriggerActions TableViewPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getPluginInstance = [this]() -> TableViewPlugin* {
        return dynamic_cast<TableViewPlugin*>(plugins().requestViewPlugin(getKind()));
    };

    const auto numberOfDatasets = datasets.count();

    if (numberOfDatasets == 1) 
    {
        auto pluginTriggerAction = new PluginTriggerAction(const_cast<TableViewPluginFactory*>(this), this, "Table", "View table data", icon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
            for (auto dataset : datasets)
                getPluginInstance();
        });

        pluginTriggerActions << pluginTriggerAction;
    }

    return pluginTriggerActions;
}
