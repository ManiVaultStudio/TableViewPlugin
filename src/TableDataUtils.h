#pragma once

#include <QVariantMap>
#include "FastTableData.h"

// Use this function to create FastTableData from a QVariantMap
FastTableData createTableFromVariantMap(const QVariantMap& map);

// Ensure default arguments are only here, not in the .cpp
FastTableData createTableFromDatasetData(
    const std::vector<float>& pointDataset = std::vector<float>(),
    int numOfRows = 0,
    std::vector<QString> pointColumnNames= std::vector<QString>(),
    const std::vector<std::vector<QString>>& clusterDataset = std::vector<std::vector<QString>>(),
    const std::vector<QString>& clusterColumnNames = std::vector<QString>()
);

FastTableData createVariantMapFromDatasetData(
    const std::vector<float>& pointDataset,
    int numOfRows,
    const std::vector<QString>& pointColumnNames,
    const std::vector<std::vector<QString>>& clusterDataset,
    const std::vector<QString>& clusterColumnNames);