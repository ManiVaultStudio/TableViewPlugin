#include "TableDataUtils.h"
#include <QVariantList>
#include <QString>
#include <vector>
#include <algorithm>
#include "FastTableData.h"
#include "HighPerfTableModel.h"
#include <QStringList>
#include "HighPerfTableView.h"
#include <limits>
#include <QColor>

// Utility function to get contrasting text color (black or white) for a given background color
static QColor getContrastingTextColor(const QColor& bg) {
    // Use luminance formula to determine brightness
    double luminance = 0.299 * bg.red() + 0.587 * bg.green() + 0.114 * bg.blue();
    return (luminance > 186) ? QColor(Qt::black) : QColor(Qt::white);
}


FastTableData createTableFromVariantMap(const QVariantMap& map) {

    std::map<QString, QString> clusterColorMap;
    QVariantMap mapCopy = map;
    if (mapCopy.contains("__clusterColorMap")) {
        QVariant colorMapVar = mapCopy.take("__clusterColorMap");
        QVariantMap colorMapQVar = colorMapVar.toMap();
        for (auto it = colorMapQVar.begin(); it != colorMapQVar.end(); ++it) {
            clusterColorMap[it.key()] = it.value().toString();
        }
    }

    if (mapCopy.isEmpty())
        return FastTableData();

    QStringList colNames = mapCopy.keys();
    int cols = colNames.size();
    int rows = -1;
    std::vector<QVariantList> columns;
    columns.reserve(cols);

    for (const QString& col : colNames) {
        QVariant v = mapCopy.value(col);
        if (!v.canConvert<QVariantList>())
            return FastTableData();
        QVariantList list = v.toList();
        if (rows == -1)
            rows = list.size();
        else if (list.size() != rows)
            return FastTableData();
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

    // Fix: Validate color string before using QColor, skip invalid colors
    for (int c = 0; c < cols; ++c) {
        if (!table.columnIsNumeric(c)) {
            for (int r = 0; r < rows; ++r) {
                QVariant v = columns[c][r];
                QString label = v.toString();
                auto it = clusterColorMap.find(label);
                if (it != clusterColorMap.end()) {
                    QColor color(it->second);
                    if (!color.isValid()) {
                        // Optionally: qWarning("Invalid color string for label '%s': '%s'", qPrintable(label), qPrintable(it->second));
                        continue; // skip invalid color
                    }
                    table.setCellColor(r, c, color);
                    // Use the actual cell background color if set, otherwise fallback to defaultBgColor
                    QColor bgColor = color.isValid() ? color : QColor();
                    table.setCellTextColor(r, c, getContrastingTextColor(bgColor));
                } else {
                    // If no color mapping, use default background color for contrast
                    table.setCellTextColor(r, c, getContrastingTextColor(QColor()));
                }
            }
        }
    }

    return table;
}


FastTableData createTableFromDatasetData(
    const std::vector<float>& pointDataset,
    int numOfRows,
    std::vector<QString> pointColumnNames,
    const std::vector<std::vector<QString>>& clusterDataset,
    const std::vector<QString>& clusterColumnNames,
    const std::vector<std::map<QString, QString>>& clusterColorMap)
{
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

    if (!pointDataset.empty() && pointColumnNames.size() > 0) {
        for (int c = 0; c < pointColumnNames.size(); ++c) {
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

    if (!clusterDataset.empty() && clusterDataset.size() >= static_cast<size_t>(numOfRows)) {
        for (int c = 0; c < clusterDataColumns; ++c) {
            int colIdx = (pointColumnNames.size() > 0 ? pointColumnNames.size() : 0) + c;
            table.setColumnName(colIdx, QString("Cluster %1").arg(c + 1));
            table.setColumnIsNumeric(colIdx, false);
            for (int r = 0; r < numOfRows; ++r) {
                if (static_cast<size_t>(c) < clusterDataset[r].size())
                    table.set(r, colIdx, clusterDataset[r][c]);
                else
                    table.set(r, colIdx, QString());
            }
        }

        // --- Ensure consistent color assignment for each cluster label in a column ---
        for (int c = 0; c < clusterDataColumns; ++c) {
            int colIdx = (pointColumnNames.size() > 0 ? pointColumnNames.size() : 0) + c;
            const std::map<QString, QString>* colorMapPtr = nullptr;
            if (c < static_cast<int>(clusterColorMap.size())) {
                colorMapPtr = &clusterColorMap[c];
            }
            // Build a cache to ensure same label always gets same color in this column
            std::map<QString, QColor> labelColorCache;
            if (colorMapPtr) {
                for (const auto& pair : *colorMapPtr) {
                    QColor color(pair.second);
                    if (color.isValid()) {
                        labelColorCache[pair.first] = color;
                    }
                }
            }
            for (int r = 0; r < numOfRows; ++r) {
                if (static_cast<size_t>(c) < clusterDataset[r].size()) {
                    const QString& clusterLabel = clusterDataset[r][c];
                    auto it = labelColorCache.find(clusterLabel);
                    if (it != labelColorCache.end()) {
                        table.setCellColor(r, colIdx, it->second);
                        QColor bgColor = it->second.isValid() ? it->second : QColor();
                        table.setCellTextColor(r, colIdx, getContrastingTextColor(bgColor));
                    } else {
                        table.setCellTextColor(r, colIdx, getContrastingTextColor(QColor()));
                    }
                }
            }
        }
    }

    for (int c = 0; c < static_cast<int>(clusterColumnNames.size()) && c < clusterDataColumns; ++c) {
        int colIdx = (pointColumnNames.size() > 0 ? pointColumnNames.size() : 0) + c;
        table.setColumnName(colIdx, clusterColumnNames[c]);
    }

    if (table.rowCount() > 0 && table.colCount() > 0)
        return table;

    return FastTableData();
}

FastTableData createVariantMapFromDatasetData(
    const std::vector<float>& pointDataset,
    int numOfRows,
    const std::vector<QString>& pointColumnNames,
    const std::vector<std::vector<QString>>& clusterDataset,
    const std::vector<QString>& clusterColumnNames,
    const std::vector<std::map<QString, QString>>& clusterColorMap)
{
    QVariantMap map;
    int numOfDims = static_cast<int>(pointColumnNames.size());

    if (!pointDataset.empty() && numOfDims > 0 && numOfRows > 0) {
        for (int c = 0; c < numOfDims; ++c) {
            QVariantList col;
            col.reserve(numOfRows);
            for (int r = 0; r < numOfRows; ++r) {
                col.append(pointDataset[r * numOfDims + c]);
            }
            QString colName = (!pointColumnNames[c].isEmpty())
                ? pointColumnNames[c]
                : QString("Dimension %1").arg(c + 1);
            map.insert(colName, col);
        }
    }

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

    if (!clusterColorMap.empty()) {
        QVariantMap colorMapQVar;
        for (int c = 0; c < static_cast<int>(clusterColorMap.size()); ++c) {
            QVariantMap columnColorMap;
            for (const auto& pair : clusterColorMap[c]) {
                columnColorMap.insert(pair.first, pair.second);
            }
            map.insert(QString("__clusterColorMap_%1").arg(c), columnColorMap);
        }
    }

    return createTableFromVariantMap(map);
}