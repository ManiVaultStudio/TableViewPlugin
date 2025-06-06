#include "TableDataUtils.h"
#include <QVariantList>
#include <QString>
#include <vector>
#include <algorithm>
#include "FastTableData.h"
#include "HighPerfTableModel.h"
#include <QStringList>
#include "HighPerfTableView.h" // If mv::Points is defined elsewhere, include its header
#include <limits>

FastTableData createTableFromVariantMap(const QVariantMap& map) {
    if (map.isEmpty())
        return FastTableData();

    QStringList colNames = map.keys();
    int cols = colNames.size();
    int rows = -1;
    std::vector<QVariantList> columns;
    columns.reserve(cols);

    for (const QString& col : colNames) {
        QVariant v = map.value(col);
        if (!v.canConvert<QVariantList>())
            return FastTableData();
        QVariantList list = v.toList();
        if (rows == -1)
            rows = list.size();
        else if (list.size() != rows)
            return FastTableData(); // inconsistent column sizes
        columns.push_back(list);
    }

    FastTableData table(rows, cols);
    for (int c = 0; c < cols; ++c) {
        table.setColumnName(c, colNames[c]);
        bool isNumeric = true;
        double minVal = std::numeric_limits<double>::max();
        double maxVal = std::numeric_limits<double>::lowest();
        for (int r = 0; r < rows; ++r) {
            const QVariant& v = columns[c][r];
            FastTableData::Value val;
            if (v.canConvert<double>()) {
                val = v.toDouble();
                double d = v.toDouble();
                minVal = std::min(minVal, d);
                maxVal = std::max(maxVal, d);
            } else if (v.canConvert<int>()) {
                val = v.toInt();
                double d = v.toInt();
                minVal = std::min(minVal, d);
                maxVal = std::max(maxVal, d);
            } else if (v.canConvert<QString>()) {
                val = v.toString();
                isNumeric = false;
            } else {
                val = QString();
                isNumeric = false;
            }
            table.set(r, c, val);
        }
        table.setColumnIsNumeric(c, isNumeric);
        if (isNumeric)
            table.setColumnMinMax(c, minVal, maxVal);
    }
    return table;
}

FastTableData createTableFromDatasetData(
    const std::vector<float>& pointDataset,
    int numOfRows,
    std::vector<QString> pointColumnNames,
    const std::vector<std::vector<QString>>& clusterDataset,
    const std::vector<QString>& clusterColumnNames)
{

    // If neither point nor cluster data is present, return empty
    if ((pointDataset.empty() || pointColumnNames.size() <= 0) && clusterDataset.empty())
        return FastTableData();
    if (numOfRows <= 0)
        return FastTableData();

    int clusterDataColumns = 0;
    if (!clusterDataset.empty()) {
        clusterDataColumns = static_cast<int>(clusterDataset[0].size());
    }
    int totalColumns = (pointColumnNames.size() > 0 ? pointColumnNames.size() : 0) + clusterDataColumns;
    if (totalColumns == 0)
        return FastTableData();

    FastTableData table(numOfRows, totalColumns);

    // Fill point data columns if present
    if (!pointDataset.empty() && pointColumnNames.size() > 0) {
        for (int c = 0; c < pointColumnNames.size(); ++c) {
            // Use provided point column name if available, otherwise fallback to default
            if (c < static_cast<int>(pointColumnNames.size()) && !pointColumnNames[c].isEmpty()) {
                table.setColumnName(c, pointColumnNames[c]);
            } else {
                table.setColumnName(c, QString("Dimension %1").arg(c + 1));
            }
            table.setColumnIsNumeric(c, true);
            double minVal = std::numeric_limits<double>::max();
            double maxVal = std::numeric_limits<double>::lowest();
            for (int r = 0; r < numOfRows; ++r) {
                double value = pointDataset[r * pointColumnNames.size() + c];
                table.set(r, c, value);
                minVal = std::min(minVal, value);
                maxVal = std::max(maxVal, value);
            }
            table.setColumnMinMax(c, minVal, maxVal);
        }
    }

    // Fill cluster data columns if present
    if (!clusterDataset.empty() && clusterDataset.size() >= static_cast<size_t>(numOfRows)) {
        for (int c = 0; c < clusterDataColumns; ++c) {
            int colIdx = (pointColumnNames.size() > 0 ? pointColumnNames.size() : 0) + c;
            // Default cluster column name
            table.setColumnName(colIdx, QString("Cluster %1").arg(c + 1));
            table.setColumnIsNumeric(colIdx, false);
            for (int r = 0; r < numOfRows; ++r) {
                if (static_cast<size_t>(c) < clusterDataset[r].size())
                    table.set(r, colIdx, clusterDataset[r][c]);
                else
                    table.set(r, colIdx, QString());
            }
        }
    }

    // Set the column names for the cluster data if provided
    for (int c = 0; c < static_cast<int>(clusterColumnNames.size()) && c < clusterDataColumns; ++c) {
        int colIdx = (pointColumnNames.size() > 0 ? pointColumnNames.size() : 0) + c;
        table.setColumnName(colIdx, clusterColumnNames[c]);
    }

    // Ensure the table is valid
    if (table.rowCount() > 0 && table.colCount() > 0)
        return table;

    return FastTableData();
}

FastTableData createVariantMapFromDatasetData(
    const std::vector<float>& pointDataset,
    int numOfRows,
    const std::vector<QString>& pointColumnNames,
    const std::vector<std::vector<QString>>& clusterDataset,
    const std::vector<QString>& clusterColumnNames)
{
    QVariantMap map;

    int numOfDims = static_cast<int>(pointColumnNames.size());

    // Add point data as columns using provided pointColumnNames
    if (!pointDataset.empty() && numOfDims > 0 && numOfRows > 0) {
        for (int c = 0; c < numOfDims; ++c) {
            QVariantList col;
            col.reserve(numOfRows);
            for (int r = 0; r < numOfRows; ++r) {
                col.append(pointDataset[r * numOfDims + c]);
            }
            // Use provided point column name if available, otherwise fallback to default
            QString colName = (!pointColumnNames[c].isEmpty())
                ? pointColumnNames[c]
                : QString("Dimension %1").arg(c + 1);
            map.insert(colName, col);
        }
    }

    // Add cluster data as columns
    int clusterDataColumns = (!clusterDataset.empty() && !clusterDataset[0].empty()) ? static_cast<int>(clusterDataset[0].size()) : 0;
    for (int c = 0; c < clusterDataColumns; ++c) {
        QVariantList col;
        col.reserve(numOfRows);
        for (int r = 0; r < numOfRows; ++r) {
            if (static_cast<size_t>(c) < clusterDataset[r].size())
                col.append(clusterDataset[r][c]);
            else
                col.append(QString());
        }
        QString colName = (c < static_cast<int>(clusterColumnNames.size()))
            ? clusterColumnNames[c]
            : QString("Cluster %1").arg(c + 1);
        map.insert(colName, col);
    }

    return createTableFromVariantMap(map);
}