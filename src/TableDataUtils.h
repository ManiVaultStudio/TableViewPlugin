#pragma once

#include <QVariantMap>
#include <QColor>
#include <map>
#include <vector>
#include "FastTableData.h"
#include "HighPerfTableModel.h"
#include "ColorMapUtils.h"
#include "CorrelationBarDelegate.h" 

FastTableData createTableFromVariantMap(const QVariantMap& map);

FastTableData createTableFromDatasetData(
    const std::vector<float>& pointDataset = std::vector<float>(),
    int numOfRows = 0,
    std::vector<QString> pointColumnNames = std::vector<QString>(),
    const std::vector<std::vector<QString>>& clusterDataset = std::vector<std::vector<QString>>(),
    const std::vector<QString>& clusterColumnNames = std::vector<QString>(),
    const std::vector<std::map<QString, QString>>& clusterColorMap = std::vector<std::map<QString, QString>>()
);

FastTableData createVariantMapFromDatasetData(
    const std::vector<float>& pointDataset,
    int numOfRows,
    const std::vector<QString>& pointColumnNames,
    const std::vector<std::vector<QString>>& clusterDataset,
    const std::vector<QString>& clusterColumnNames,
    const std::vector<std::map<QString, QString>>& clusterColorMap
);

QColor getNumericCellColor(double value, double minVal, double maxVal);

CorrelationBarDelegate::ColorMapType toCorrelationBarColorMapType(HighPerfTableModel::ColorMapType type);

namespace TableDataUtils {

inline QColor colormapColor(float norm, HighPerfTableModel::ColorMapType cmap) {
    return getColormapColor(toCorrelationBarColorMapType(cmap), norm);
}

}