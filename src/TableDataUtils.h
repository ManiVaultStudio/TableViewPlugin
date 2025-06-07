#pragma once

#include <QVariantMap>
#include <QColor>
#include <map>
#include <vector>
#include "FastTableData.h"
#include "HighPerfTableModel.h"

/**
 * Creates a FastTableData from a QVariantMap and applies clusterColorMap to color cluster cells.
 * If the QVariantMap contains a key "__clusterColorMap" (QVariantMap of label->color), it will be used for coloring.
 * @param map The QVariantMap. May contain a "__clusterColorMap" entry.
 * @param defaultBgColor The default background color to use for contrast calculations.
 * @return FastTableData with colored cluster cells.
 */
FastTableData createTableFromVariantMap(const QVariantMap& map);

/**
 * Creates a FastTableData from the given datasets and applies clusterColorMap to color cluster cells.
 * @param pointDataset The point data.
 * @param numOfRows Number of rows.
 * @param pointColumnNames Names of point columns.
 * @param clusterDataset The cluster data.
 * @param clusterColumnNames Names of cluster columns.
 * @param clusterColorMap Vector of maps from cluster label to color (as string, e.g. "#RRGGBB"), one map per cluster column.
 * @param defaultBgColor The default background color to use for contrast calculations.
 * @return FastTableData with colored cluster cells.
 */
FastTableData createTableFromDatasetData(
    const std::vector<float>& pointDataset = std::vector<float>(),
    int numOfRows = 0,
    std::vector<QString> pointColumnNames = std::vector<QString>(),
    const std::vector<std::vector<QString>>& clusterDataset = std::vector<std::vector<QString>>(),
    const std::vector<QString>& clusterColumnNames = std::vector<QString>(),
    const std::vector<std::map<QString, QString>>& clusterColorMap = std::vector<std::map<QString, QString>>()
);

/**
 * Creates a FastTableData from the given datasets and applies clusterColorMap to color cluster cells.
 * The clusterColorMap will be included in the QVariantMap under the key "__clusterColorMap".
 * @param pointDataset The point data.
 * @param numOfRows Number of rows.
 * @param pointColumnNames Names of point columns.
 * @param clusterDataset The cluster data.
 * @param clusterColumnNames Names of cluster columns.
 * @param clusterColorMap Vector of maps from cluster label to color (as string, e.g. "#RRGGBB"), one map per cluster column.
 * @param defaultBgColor The default background color to use for contrast calculations.
 * @return FastTableData with colored cluster cells.
 */
FastTableData createVariantMapFromDatasetData(
    const std::vector<float>& pointDataset,
    int numOfRows,
    const std::vector<QString>& pointColumnNames,
    const std::vector<std::vector<QString>>& clusterDataset,
    const std::vector<QString>& clusterColumnNames,
    const std::vector<std::map<QString, QString>>& clusterColorMap
);

/**
 * Utility function to get the background color for numeric cells based on their value, minimum, and maximum.
 * @param value The numeric value of the cell.
 * @param minVal The minimum value in the column.
 * @param maxVal The maximum value in the column.
 * @return QColor representing the background color for the cell.
 */
QColor getNumericCellColor(double value, double minVal, double maxVal);

namespace TableDataUtils {

/**
 * Map normalized value [0,1] to QColor using the specified colormap.
 * @param norm The normalized value in the range [0,1].
 * @param cmap The colormap type to use for mapping.
 * @return QColor representing the mapped color.
 */
QColor colormapColor(float norm, HighPerfTableModel::ColorMapType cmap);

} // namespace TableDataUtils