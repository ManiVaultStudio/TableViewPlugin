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
    _settingsAction(*this)
{
    _currentDatasetNameLabel->setAcceptDrops(true);
    _currentDatasetNameLabel->setAlignment(Qt::AlignCenter);

    _currentDatasetNameLabel->setMinimumHeight(40); 
    _currentDatasetNameLabel->setWordWrap(true);
    _currentDatasetNameLabel->setStyleSheet(
        "QLabel {"
        "  border: 1.5px dashed #6c6cff;"
        "  background: #f5f7ff;"
        "  border-radius: 8px;"
        "  color: #333;"
        "  font-style: italic;"
        "  padding: 4px 8px;"
        "  font-size: 13px;"
        "}"
        "QLabel:disabled {"
        "  color: #aaa;"
        "}"
    );

    getLearningCenterAction().addVideos(QStringList({ "Practitioner", "Developer" }));
}

void TableViewPlugin::init()
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    auto settings = new QHBoxLayout();
    settings->setContentsMargins(0, 0, 0, 0);
    settings->setSpacing(0);
    settings->addWidget(_settingsAction.getDatasetOptionsHolder().createWidget(&getWidget()));
    layout->addLayout(settings);
    layout->addWidget(_settingsAction.getTableViewAction());
    layout->addWidget(_currentDatasetNameLabel);
    getWidget().setLayout(layout);

    _dropWidget = new DropWidget(_currentDatasetNameLabel);

    auto* dropIndicator = new DropWidget::DropIndicatorWidget(
        &getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data...");
    dropIndicator->setMinimumHeight(64);
    dropIndicator->setStyleSheet(
        "QWidget {"
        "  padding: 14px 12px 20px 12px;"
        "  font-size: 13px;"
        "  color: #333;"
        "  background: transparent;"
        "}"
    );
    _dropWidget->setDropIndicatorWidget(dropIndicator);

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

                if (_points.isValid() && candidateDataset.isValid() && _points == candidateDataset) {
                    qDebug() << "[DropWidget] Same dataset detected, drop not allowed.";
                    dropRegions << new DropWidget::DropRegion(
                        this,
                        "Drop not allowed",
                        "This dataset is already loaded.",
                        "exclamation-circle",
                        false 
                    );
                }
                else {
                    dropRegions << new DropWidget::DropRegion(this, "Points", description, "map-marker-alt", true, [this, candidateDataset]() {
                        
                        _points = candidateDataset;
                        qDebug() << "[DropWidget] _points.isValid() after assignment:" << _points.isValid();
                        //_settingsAction.getDatasetOptionsHolder().getPointDatasetAction().setCurrentText("");
                        //_settingsAction.getDatasetOptionsHolder().getPointDatasetAction().setCurrentIndex(-1);
                        _settingsAction.getDatasetOptionsHolder().getPointDatasetAction().setCurrentDataset(_points);
                        //modifyandSetNewPointData();

                        if (_points.isValid()) {
                            auto newDatasetName = _points->getGuiName();
                            _currentDatasetNameLabel->setText(QString("Current points dataset: %1").arg(newDatasetName));
                            _dropWidget->setShowDropIndicator(newDatasetName.isEmpty());
                        }
                       
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
        //_settingsAction.getDatasetOptionsHolder().getPointDatasetAction().setCurrentText("");
        //_settingsAction.getDatasetOptionsHolder().getPointDatasetAction().setCurrentIndex(-1);
        //_settingsAction.getDatasetOptionsHolder().getPointDatasetAction().setCurrentDataset(_points);

 
    });


    connect(&_points, &Dataset<Points>::dataChanged, this, [this]() {

        if(_points.isValid())
        { auto newDatasetName = _points->getGuiName();
        _currentDatasetNameLabel->setText(QString("Current points dataset: %1").arg(newDatasetName));
        //_dropWidget->setShowDropIndicator(newDatasetName.isEmpty());

        modifyandSetNewPointData();

        qDebug() << "[dataChanged] Exiting. _isUpdatingPoints reset to false.";
        }
    });

    connect(&_settingsAction.getDatasetOptionsHolder().getPointDatasetAction(), &DatasetPickerAction::currentIndexChanged, this, [this]() {


        if(_settingsAction.getDatasetOptionsHolder().getPointDatasetAction().getCurrentDataset().isValid())
        {
            _points = _settingsAction.getDatasetOptionsHolder().getPointDatasetAction().getCurrentDataset();
            mv::events().notifyDatasetDataChanged(_points);
            qDebug() << "[currentIndexChanged] _points set to valid dataset.";
        }
        else
        {
            _points = Dataset<Points>();
            qDebug() << "[currentIndexChanged] _points set to invalid dataset.";
        }

    });

    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetAdded));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataChanged));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetRemoved));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
    _eventListener.registerDataEventByType(PointType, std::bind(&TableViewPlugin::onDataEvent, this, std::placeholders::_1));

    _settingsAction.getTableViewAction()->setBarDelegateForNumericalColumns(true);
}

void TableViewPlugin::setShowBarsForNumericalColumns(bool enabled)
{
    if (_settingsAction.getTableViewAction())
        _settingsAction.getTableViewAction()->setBarDelegateForNumericalColumns(enabled);
}

void TableViewPlugin::modifyandSetNewPointData()
{
    qDebug() << "[modifyandSetPointData] _points.isValid():" << _points.isValid();
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

        auto children = _points->getChildren();
        std::vector<std::vector<QString>> clusterDataset;
        std::vector<QString> clusterColumnNames;
        std::vector<std::map<QString, QString>> clusterColorMap;

        for (const Dataset<Clusters>& child : children) {
            if (child->getDataType() == ClusterType) {
                std::vector<QString> clusterInfo(numOfRows, QString());
                std::map<QString, QString> colorMapForThisColumn;
                clusterColumnNames.push_back(child->getGuiName());
                auto clusterValues = child->getClusters();

                for (const auto& clusterValue : clusterValues) {
                    auto clusterName = clusterValue.getName();
                    auto clusterIndices = clusterValue.getIndices();
                    QString clusterColor = clusterValue.getColor().name();

                    if (!clusterName.isEmpty() && !clusterColor.isEmpty()) {
                        if (colorMapForThisColumn.find(clusterName) == colorMapForThisColumn.end()) {
                            colorMapForThisColumn[clusterName] = clusterColor;
                        }
                    }

                    for (const auto& index : clusterIndices) {
                        if (index < numOfRows) {
                            clusterInfo[index] = clusterName;
                        }
                    }
                }
                clusterDataset.push_back(clusterInfo);
                clusterColorMap.push_back(colorMapForThisColumn);
            }
        }

        std::vector<std::vector<QString>> clusterDatasetRows(numOfRows);
        for (int r = 0; r < numOfRows; ++r) {
            clusterDatasetRows[r].resize(clusterDataset.size());
            for (size_t c = 0; c < clusterDataset.size(); ++c) {
                clusterDatasetRows[r][c] = clusterDataset[c][r];
            }
        }

        for (const Dataset<Points>& child : children)
        {
            if (child->getDataType() == PointType) {
                qDebug() << "[modifyandSetPointData] Found child dataset of type PointType:" << child->getGuiName();
                auto childColumnNames = child->getDimensionNames();
                int childNumOfDims = child->getNumDimensions();
                int childNumOfRows = child->getNumPoints();
                std::vector<int> childColumnIndices;
                for (int i = 0; i < childNumOfDims; ++i) {
                    childColumnIndices.push_back(i);
                }
                std::vector<float> childXData(childNumOfRows * childNumOfDims);
                child->populateDataForDimensions(childXData, childColumnIndices);

                if (childNumOfRows == numOfRows && childNumOfDims > 0) {
                    int oldNumCols = numOfDims;
                    int newNumCols = numOfDims + childNumOfDims;
                    std::vector<float> mergedXData(numOfRows * newNumCols);

                    for (int row = 0; row < numOfRows; ++row) {
                        for (int col = 0; col < numOfDims; ++col) {
                            mergedXData[row * newNumCols + col] = xData[row * numOfDims + col];
                        }
                    }
                    for (int row = 0; row < numOfRows; ++row) {
                        for (int col = 0; col < childNumOfDims; ++col) {
                            mergedXData[row * newNumCols + (numOfDims + col)] = childXData[row * childNumOfDims + col];
                        }
                    }
                    xData = std::move(mergedXData);
                    columnNames.insert(columnNames.end(), childColumnNames.begin(), childColumnNames.end());
                    numOfDims = newNumCols;
                }
            }
        }

        FastTableData fastData = createTableFromDatasetData(
            xData, numOfRows, columnNames, clusterDatasetRows, clusterColumnNames);
        _settingsAction.getTableViewAction()->setData(fastData);

        qDebug() << "[modifyandSetPointData] Table data set with" << numOfRows << "rows and" << numOfDims << "columns.";
    }
    else {
        qDebug() << "[modifyandSetPointData] No valid points dataset, clearing table.";
        _settingsAction.getTableViewAction()->setData(FastTableData());
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
    return pluginTriggerActions;
}

void TableViewPlugin::fromVariantMap(const QVariantMap& variantMap)
{
    ViewPlugin::fromVariantMap(variantMap);
    mv::util::variantMapMustContain(variantMap, "TableViewPlugin:Settings");
    _settingsAction.fromVariantMap(variantMap["TableViewPlugin:Settings"].toMap());
}

QVariantMap TableViewPlugin::toVariantMap() const
{
    QVariantMap variantMap = ViewPlugin::toVariantMap();

    _settingsAction.insertIntoVariantMap(variantMap);
    return variantMap;
}