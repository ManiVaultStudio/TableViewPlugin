#include "HighPerfTableModel.h"
#include "FastTableData.h"
#include "TableDataUtils.h"
#include <algorithm>

HighPerfTableModel::HighPerfTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{}

void HighPerfTableModel::setData(const FastTableData& data) {
    beginResetModel();
    _data = data;
    endResetModel();
}

int HighPerfTableModel::rowCount(const QModelIndex&) const {
    return _data.rowCount();
}

int HighPerfTableModel::columnCount(const QModelIndex&) const {
    return _data.colCount();
}

QVariant HighPerfTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();

    if (_showBars && role == Qt::UserRole + 1 && isNumericalColumn(index.column())) {
        const auto& v = _data.get(index.row(), index.column());
        if (std::holds_alternative<double>(v))
            return static_cast<float>(std::get<double>(v));
        if (std::holds_alternative<int>(v))
            return static_cast<float>(std::get<int>(v));
        return {};
    }
    if (!_showBars && role == Qt::DisplayRole) {
        const auto& v = _data.get(index.row(), index.column());
        if (std::holds_alternative<double>(v))
            return std::get<double>(v);
        if (std::holds_alternative<int>(v))
            return std::get<int>(v);
        if (std::holds_alternative<QString>(v))
            return std::get<QString>(v);
        return {};
    }
    if (_showBars && role == Qt::DisplayRole) {
        const auto& v = _data.get(index.row(), index.column());
        if (std::holds_alternative<double>(v))
            return std::get<double>(v);
        if (std::holds_alternative<int>(v))
            return std::get<int>(v);
        if (std::holds_alternative<QString>(v))
            return std::get<QString>(v);
        return {};
    }
    if (role == Qt::ForegroundRole) {
        if (_data.hasCellTextColor(row, col)) {
            return _data.cellTextColor(row, col);
        }
    }
    if (role == Qt::BackgroundRole) {
        if (isNumericalColumn(col) && !_showBars) {
            const auto& val = _data.get(row, col);
            if (std::holds_alternative<double>(val)) {
                float fval = static_cast<float>(std::get<double>(val));
                return colorForValue(col, fval);
            }
            if (std::holds_alternative<int>(val)) {
                float fval = static_cast<float>(std::get<int>(val));
                return colorForValue(col, fval);
            }
        }

        QColor color = _data.cellColor(row, col);
        if (color.isValid())
            return color;
        
        if (!_data.columnIsNumeric(col)) {
            return QColor();
        }
        return _data.rowBarColor(index.row());
    }
    return {};
}

QVariant HighPerfTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal)
        return _data.columnName(section);
    else
        return QString::number(section);
}

bool HighPerfTableModel::isNumericalColumn(int col) const {
    return _data.columnIsNumeric(col);
}

void HighPerfTableModel::getColumnMinMax(int col, float& minVal, float& maxVal) const {
    double minD, maxD;
    _data.getColumnMinMax(col, minD, maxD);
    minVal = static_cast<float>(minD);
    maxVal = static_cast<float>(maxD);
}

void HighPerfTableModel::setShowBars(bool show) {
    if (_showBars != show) {
        _showBars = show;
    }
}

bool HighPerfTableModel::showBars() const {
    return _showBars;
}

int HighPerfTableModel::primaryKeyColumn() const {
    return _data.primaryKeyColumn();
}

void HighPerfTableModel::sort(int column, Qt::SortOrder order) {
    if (column < 0 || column >= _data.colCount())
        return;

    std::vector<int> rowIndices(_data.rowCount());
    std::iota(rowIndices.begin(), rowIndices.end(), 0);

    auto valueLess = [&](int a, int b) {
        const auto& va = _data.get(a, column);
        const auto& vb = _data.get(b, column);
        double da = 0, db = 0;
        if (std::holds_alternative<double>(va)) da = std::get<double>(va);
        else if (std::holds_alternative<int>(va)) da = static_cast<double>(std::get<int>(va));
        else if (std::holds_alternative<QString>(va)) return std::get<QString>(va) < std::get<QString>(vb);
        if (std::holds_alternative<double>(vb)) db = std::get<double>(vb);
        else if (std::holds_alternative<int>(vb)) db = static_cast<double>(std::get<int>(vb));
        else if (std::holds_alternative<QString>(vb)) return std::get<QString>(va) < std::get<QString>(vb);
        return da < db;
    };

    if (order == Qt::AscendingOrder)
        std::stable_sort(rowIndices.begin(), rowIndices.end(), valueLess);
    else
        std::stable_sort(rowIndices.begin(), rowIndices.end(), [&](int a, int b) { return valueLess(b, a); });

    FastTableData newData = _data;
    for (int r = 0; r < _data.rowCount(); ++r) {
        for (int c = 0; c < _data.colCount(); ++c) {
            newData.set(r, c, _data.get(rowIndices[r], c));
            newData.setCellColor(r, c, _data.cellColor(rowIndices[r], c));
            newData.setCellTextColor(r, c, _data.cellTextColor(rowIndices[r], c));
        }
        newData.setRowBarColor(r, _data.rowBarColor(rowIndices[r]));
    }
    beginResetModel();
    _data = newData;
    endResetModel();
}

void HighPerfTableModel::requestMoreRowsTop(int n)
{
    if (_data.canFetchMoreRowsTop(n)) {
        beginInsertRows(QModelIndex(), 0, n - 1);
        _data.fetchMoreRowsTop(n);
        endInsertRows();
    }
}

void HighPerfTableModel::requestMoreRowsBottom(int n)
{
    int oldCount = rowCount();
    if (_data.canFetchMoreRowsBottom(n)) {
        beginInsertRows(QModelIndex(), oldCount, oldCount + n - 1);
        _data.fetchMoreRowsBottom(n);
        endInsertRows();
    }
}

void HighPerfTableModel::requestMoreColsLeft(int n)
{
    if (_data.canFetchMoreColsLeft(n)) {
        beginInsertColumns(QModelIndex(), 0, n - 1);
        _data.fetchMoreColsLeft(n);
        endInsertColumns();
    }
}

void HighPerfTableModel::requestMoreColsRight(int n)
{
    int oldCount = columnCount();
    if (_data.canFetchMoreColsRight(n)) {
        beginInsertColumns(QModelIndex(), oldCount, oldCount + n - 1);
        _data.fetchMoreColsRight(n);
        endInsertColumns();
    }
}

void HighPerfTableModel::addColumn(const QString& name, const FastTableData::Value& defaultValue) {
    beginResetModel();
    _data.addColumn(name, defaultValue);
    endResetModel();
}

bool HighPerfTableModel::removeColumn(const QString& name) {
    beginResetModel();
    bool result = _data.removeColumn(name);
    endResetModel();
    return result;
}

void HighPerfTableModel::setDefaultClusterBackgroundColor(const QColor& color)
{
    m_defaultClusterBgColor = color;
}

QColor HighPerfTableModel::defaultClusterBackgroundColor() const
{
    return m_defaultClusterBgColor;
}

void HighPerfTableModel::setColumnColorMap(int col, ColorMapType cmap) {
    m_columnColorMaps[col] = cmap;
    emit dataChanged(index(0, col), index(rowCount() - 1, col), {Qt::BackgroundRole});
}

HighPerfTableModel::ColorMapType HighPerfTableModel::columnColorMap(int col) const {
    auto it = m_columnColorMaps.find(col);
    if (it != m_columnColorMaps.end())
        return it->second;
    return ColorMapType::Viridis;
}

QColor HighPerfTableModel::colorForValue(int col, float value) const {
    float minVal, maxVal;
    getColumnMinMax(col, minVal, maxVal);
    if (maxVal == minVal) return Qt::white;
    float norm = (value - minVal) / (maxVal - minVal);
    return TableDataUtils::colormapColor(norm, columnColorMap(col));
}