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
#include <cmath>
#include "CorrelationBarDelegate.h" 

static QColor getContrastingTextColor(const QColor& bg) {
    double luminance = 0.299 * bg.red() + 0.587 * bg.green() + 0.114 * bg.blue();
    return (luminance > 186) ? QColor(Qt::black) : QColor(Qt::white);
}

QColor getNumericCellColor(double value, double minVal, double maxVal) {
    if (minVal == maxVal) return QColor(220, 220, 220);
    double norm = (value - minVal) / (maxVal - minVal);
    int r, g, b;
    if (norm < 0.5) {
        double t = norm * 2.0;
        r = g = 255 * t;
        b = 255;
    } else {
        double t = (norm - 0.5) * 2.0;
        r = 255;
        g = b = 255 * (1.0 - t);
    }
    return QColor(r, g, b);
}

CorrelationBarDelegate::ColorMapType toCorrelationBarColorMapType(HighPerfTableModel::ColorMapType type) {
    switch (type) {
        case HighPerfTableModel::ColorMapType::BrBG:
            return CorrelationBarDelegate::ColorMapType::BrBG;
        case HighPerfTableModel::ColorMapType::BuPu:
            return CorrelationBarDelegate::ColorMapType::BuPu;
        case HighPerfTableModel::ColorMapType::GnBu:
            return CorrelationBarDelegate::ColorMapType::GnBu;
        case HighPerfTableModel::ColorMapType::Magma:
            return CorrelationBarDelegate::ColorMapType::Magma;
        case HighPerfTableModel::ColorMapType::PiYG:
            return CorrelationBarDelegate::ColorMapType::PiYG;
        case HighPerfTableModel::ColorMapType::Plasma:
            return CorrelationBarDelegate::ColorMapType::Plasma;
        case HighPerfTableModel::ColorMapType::PuOr:
            return CorrelationBarDelegate::ColorMapType::PuOr;
        case HighPerfTableModel::ColorMapType::Q_BlGrRd:
            return CorrelationBarDelegate::ColorMapType::Q_BlGrRd;
        case HighPerfTableModel::ColorMapType::Qualitative:
            return CorrelationBarDelegate::ColorMapType::Qualitative;
        case HighPerfTableModel::ColorMapType::RdBu:
            return CorrelationBarDelegate::ColorMapType::RdBu;
        case HighPerfTableModel::ColorMapType::RdPu:
            return CorrelationBarDelegate::ColorMapType::RdPu;
        case HighPerfTableModel::ColorMapType::RdYlBu:
            return CorrelationBarDelegate::ColorMapType::RdYlBu;
        case HighPerfTableModel::ColorMapType::RdYlGn:
            return CorrelationBarDelegate::ColorMapType::RdYlGn;
        case HighPerfTableModel::ColorMapType::Reds:
            return CorrelationBarDelegate::ColorMapType::Reds;
        case HighPerfTableModel::ColorMapType::Spectral:
            return CorrelationBarDelegate::ColorMapType::Spectral;
        case HighPerfTableModel::ColorMapType::Viridis:
            return CorrelationBarDelegate::ColorMapType::Viridis;
        case HighPerfTableModel::ColorMapType::YlGn:
            return CorrelationBarDelegate::ColorMapType::YlGn;
        case HighPerfTableModel::ColorMapType::YlGnBu:
            return CorrelationBarDelegate::ColorMapType::YlGnBu;
        case HighPerfTableModel::ColorMapType::YlOrBr:
            return CorrelationBarDelegate::ColorMapType::YlOrBr;
        default:
            return CorrelationBarDelegate::ColorMapType::Viridis;
    }
}

namespace TableDataUtils {

} // namespace TableDataUtils

FastTableData createTableFromVariantMap(const QVariantMap& map) {
    std::vector<std::map<QString, QString>> clusterColorMaps;
    QVariantMap mapCopy = map;
    int clusterColorMapIdx = 0;
    while (true) {
        QString key = QString("__clusterColorMap_%1").arg(clusterColorMapIdx);
        if (!mapCopy.contains(key))
            break;
        QVariant colorMapVar = mapCopy.take(key);
        QVariantMap colorMapQVar = colorMapVar.toMap();
        std::map<QString, QString> colorMap;
        for (auto it = colorMapQVar.begin(); it != colorMapQVar.end(); ++it) {
            colorMap[it.key()] = it.value().toString();
        }
        clusterColorMaps.push_back(colorMap);
        ++clusterColorMapIdx;
    }
    std::map<QString, QString> legacyClusterColorMap;
    if (mapCopy.contains("__clusterColorMap")) {
        QVariant colorMapVar = mapCopy.take("__clusterColorMap");
        QVariantMap colorMapQVar = colorMapVar.toMap();
        for (auto it = colorMapQVar.begin(); it != colorMapQVar.end(); ++it) {
            legacyClusterColorMap[it.key()] = it.value().toString();
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

    std::vector<bool> isNumericCol(cols, true);
    std::vector<double> minVals(cols, std::numeric_limits<double>::max());
    std::vector<double> maxVals(cols, std::numeric_limits<double>::lowest());

    for (int c = 0; c < cols; ++c) {
        table.setColumnName(c, colNames[c]);
        for (int r = 0; r < rows; ++r) {
            const QVariant& v = columns[c][r];
            FastTableData::Value val;
            if (v.canConvert<double>()) {
                val = v.toDouble();
                double d = v.toDouble();
                minVals[c] = std::min(minVals[c], d);
                maxVals[c] = std::max(maxVals[c], d);
            } else if (v.canConvert<int>()) {
                val = v.toInt();
                double d = v.toInt();
                minVals[c] = std::min(minVals[c], d);
                maxVals[c] = std::max(maxVals[c], d);
            } else if (v.canConvert<QString>()) {
                val = v.toString();
                isNumericCol[c] = false;
            } else {
                val = QString();
                isNumericCol[c] = false;
            }
            table.set(r, c, val);
        }
        table.setColumnIsNumeric(c, isNumericCol[c]);
        if (isNumericCol[c])
            table.setColumnMinMax(c, minVals[c], maxVals[c]);
    }

    for (int c = 0; c < cols; ++c) {
        if (isNumericCol[c]) {
            double minVal = minVals[c], maxVal = maxVals[c];
            for (int r = 0; r < rows; ++r) {
                QVariant v = columns[c][r];
                double d = v.toDouble();
                QColor bg = getNumericCellColor(d, minVal, maxVal);
                table.setCellColor(r, c, bg);
                table.setCellTextColor(r, c, getContrastingTextColor(bg));
            }
        }
    }

    for (int c = 0; c < cols; ++c) {
        if (!isNumericCol[c]) {
            const std::map<QString, QString>* colorMapPtr = nullptr;
            if (c < static_cast<int>(clusterColorMaps.size()))
                colorMapPtr = &clusterColorMaps[c];
            else if (!legacyClusterColorMap.empty())
                colorMapPtr = &legacyClusterColorMap;

            std::map<QString, QColor> labelColorCache;
            if (colorMapPtr) {
                for (const auto& pair : *colorMapPtr) {
                    QColor color(pair.second);
                    if (color.isValid()) {
                        labelColorCache[pair.first] = color;
                    }
                }
            }
            for (int r = 0; r < rows; ++r) {
                QVariant v = columns[c][r];
                QString label = v.toString();
                auto it = labelColorCache.find(label);
                if (it != labelColorCache.end()) {
                    table.setCellColor(r, c, it->second);
                    QColor bgColor = it->second.isValid() ? it->second : QColor();
                    table.setCellTextColor(r, c, getContrastingTextColor(bgColor));
                } else {
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

            for (int r = 0; r < numOfRows; ++r) {
                double value = pointDataset[r * pointColumnNames.size() + c];
                QColor bg = getNumericCellColor(value, minVal, maxVal);
                table.setCellColor(r, c, bg);
                table.setCellTextColor(r, c, getContrastingTextColor(bg));
            }
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

        for (int c = 0; c < clusterDataColumns; ++c) {
            int colIdx = (pointColumnNames.size() > 0 ? pointColumnNames.size() : 0) + c;
            const std::map<QString, QString>* colorMapPtr = nullptr;
            if (c < static_cast<int>(clusterColorMap.size())) {
                colorMapPtr = &clusterColorMap[c];
            }
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
        QString colName = (c < static_cast<int>(clusterColumnNames.size()) && !clusterColumnNames[c].isEmpty())
            ? clusterColumnNames[c]
            : QString("Cluster %1").arg(c + 1);
        map.insert(colName, col);
    }

    if (!clusterColorMap.empty()) {
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