#pragma once

#include <QVariantMap>
#include <QColor>
#include <map>
#include <vector>
#include "FastTableData.h"
#include "HighPerfTableModel.h"

// Creates a FastTableData from a QVariantMap and applies clusterColorMap to color cluster cells.
FastTableData createTableFromVariantMap(const QVariantMap& map);

// Creates a FastTableData from the given datasets and applies clusterColorMap to color cluster cells.
FastTableData createTableFromDatasetData(
    const std::vector<float>& pointDataset = std::vector<float>(),
    int numOfRows = 0,
    std::vector<QString> pointColumnNames = std::vector<QString>(),
    const std::vector<std::vector<QString>>& clusterDataset = std::vector<std::vector<QString>>(),
    const std::vector<QString>& clusterColumnNames = std::vector<QString>(),
    const std::vector<std::map<QString, QString>>& clusterColorMap = std::vector<std::map<QString, QString>>()
);

// Creates a FastTableData from the given datasets and applies clusterColorMap to color cluster cells.
// The clusterColorMap will be included in the QVariantMap under the key "__clusterColorMap".
FastTableData createVariantMapFromDatasetData(
    const std::vector<float>& pointDataset,
    int numOfRows,
    const std::vector<QString>& pointColumnNames,
    const std::vector<std::vector<QString>>& clusterDataset,
    const std::vector<QString>& clusterColumnNames,
    const std::vector<std::map<QString, QString>>& clusterColorMap
);

// Utility function to get the background color for numeric cells based on their value, minimum, and maximum.
QColor getNumericCellColor(double value, double minVal, double maxVal);

namespace TableDataUtils {

// Map normalized value [0,1] to QColor using the specified colormap.
QColor colormapColor(float norm, HighPerfTableModel::ColorMapType cmap);

} // namespace TableDataUtils