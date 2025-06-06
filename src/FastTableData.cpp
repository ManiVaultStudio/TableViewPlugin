#include "FastTableData.h"
#include <cassert>
#include <limits>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>
#include <algorithm>
#include "TableDataUtils.h"

FastTableData::FastTableData(int rows, int cols) {
    resize(rows, cols);
}

void FastTableData::resize(int rows, int cols) {
    _rows = rows;
    _cols = cols;
    _data.resize(rows * cols);
    _colNames.resize(cols);
    _colIsNumeric.assign(cols, true);
    _colMinMax.resize(cols, {0.0, 0.0});
    _rowBarColors.resize(rows);
    _rowVisible.assign(rows, true);
}

void FastTableData::set(int row, int col, const Value& v) {
    assert(row >= 0 && row < _rows && col >= 0 && col < _cols);
    _data[row * _cols + col] = v;
}

const FastTableData::Value& FastTableData::get(int row, int col) const {
    assert(row >= 0 && row < _rows && col >= 0 && col < _cols);
    return _data[row * _cols + col];
}

void FastTableData::setColumnName(int col, const QString& name) {
    if (col >= 0 && col < _cols) _colNames[col] = name;
}

QString FastTableData::columnName(int col) const {
    if (col >= 0 && col < _cols) return _colNames[col];
    return {};
}

void FastTableData::setColumnIsNumeric(int col, bool isNumeric) {
    if (col >= 0 && col < _cols) _colIsNumeric[col] = isNumeric;
}

bool FastTableData::columnIsNumeric(int col) const {
    if (col >= 0 && col < _cols) return _colIsNumeric[col];
    return true;
}

void FastTableData::setColumnMinMax(int col, double minVal, double maxVal) {
    if (col >= 0 && col < _cols) _colMinMax[col] = {minVal, maxVal};
}

void FastTableData::getColumnMinMax(int col, double& minVal, double& maxVal) const {
    if (col >= 0 && col < _cols) {
        minVal = _colMinMax[col].first;
        maxVal = _colMinMax[col].second;
    } else {
        minVal = 0.0;
        maxVal = 0.0;
    }
}

int FastTableData::primaryKeyColumn() const {
    return _primaryKeyCol;
}

void FastTableData::setPrimaryKeyColumn(int col) {
    if (col >= 0 && col < _cols)
        _primaryKeyCol = col;
    else
        _primaryKeyCol = -1;
}

bool FastTableData::isPrimaryKeyColumn(int col) const {
    return col == _primaryKeyCol;
}

std::vector<FastTableData::Value> FastTableData::getRow(int row) const {
    std::vector<Value> result;
    if (row < 0 || row >= _rows) return result;
    result.reserve(_cols);
    for (int c = 0; c < _cols; ++c)
        result.push_back(get(row, c));
    return result;
}

std::vector<FastTableData::Value> FastTableData::getColumn(int col) const {
    std::vector<Value> result;
    if (col < 0 || col >= _cols) return result;
    result.reserve(_rows);
    for (int r = 0; r < _rows; ++r)
        result.push_back(get(r, col));
    return result;
}

std::vector<std::vector<FastTableData::Value>> FastTableData::getRows() const {
    std::vector<std::vector<Value>> result;
    result.reserve(_rows);
    for (int r = 0; r < _rows; ++r)
        result.push_back(getRow(r));
    return result;
}

std::vector<std::vector<FastTableData::Value>> FastTableData::getColumns() const {
    std::vector<std::vector<Value>> result;
    result.reserve(_cols);
    for (int c = 0; c < _cols; ++c)
        result.push_back(getColumn(c));
    return result;
}

// --- Bar color support ---
void FastTableData::setRowBarColor(int row, const QColor& color) {
    if (row >= 0 && row < _rows) _rowBarColors[row] = color;
}

QColor FastTableData::rowBarColor(int row) const {
    if (row >= 0 && row < _rows) return _rowBarColors[row];
    return QColor();
}

void FastTableData::setAllRowBarColors(const std::vector<QColor>& colors) {
    _rowBarColors = colors;
    if ((int)_rowBarColors.size() != _rows) _rowBarColors.resize(_rows);
}

const std::vector<QColor>& FastTableData::getAllRowBarColors() const {
    return _rowBarColors;
}
// --- end bar color support ---

// Filtering support
void FastTableData::setRowVisible(int row, bool visible) {
    if (row >= 0 && row < _rows) _rowVisible[row] = visible;
}

bool FastTableData::isRowVisible(int row) const {
    if (row >= 0 && row < _rows) return _rowVisible[row];
    return true;
}

void FastTableData::clearRowFilter() {
    std::fill(_rowVisible.begin(), _rowVisible.end(), true);
}

// Utility
void FastTableData::clear() {
    _rows = 0;
    _cols = 0;
    _data.clear();
    _colNames.clear();
    _colIsNumeric.clear();
    _colMinMax.clear();
    _rowBarColors.clear();
    _rowVisible.clear();
    _primaryKeyCol = -1;
}

// Lazy loading API stubs (implement as needed for your data source)
bool FastTableData::canFetchMoreRowsTop(int n) const {
    // Return true if there are more rows available above current data
    // TODO: Implement logic based on your data source
    return false;
}

bool FastTableData::canFetchMoreRowsBottom(int n) const {
    // Return true if there are more rows available below current data
    // TODO: Implement logic based on your data source
    return false;
}

bool FastTableData::canFetchMoreColsLeft(int n) const {
    // Return true if there are more columns available to the left
    // TODO: Implement logic based on your data source
    return false;
}

bool FastTableData::canFetchMoreColsRight(int n) const {
    // Return true if there are more columns available to the right
    // TODO: Implement logic based on your data source
    return false;
}

void FastTableData::fetchMoreRowsTop(int n) {
    // Fetch and prepend n rows to the top of the data
    // TODO: Implement logic based on your data source
}

void FastTableData::fetchMoreRowsBottom(int n) {
    // Fetch and append n rows to the bottom of the data
    // TODO: Implement logic based on your data source
}

void FastTableData::fetchMoreColsLeft(int n) {
    // Fetch and prepend n columns to the left of the data
    // TODO: Implement logic based on your data source
}

void FastTableData::fetchMoreColsRight(int n) {
    // Fetch and append n columns to the right of the data
    // TODO: Implement logic based on your data source
}

void FastTableData::addColumn(const QString& name, const Value& defaultValue) {
    // Add column name and metadata
    _colNames.push_back(name);
    _colIsNumeric.push_back(std::holds_alternative<double>(defaultValue) || std::holds_alternative<int>(defaultValue));
    _colMinMax.emplace_back(0.0, 0.0); // Optionally compute min/max later
    _cols += 1;
    // Add default value for each row
    for (int row = 0; row < _rows; ++row) {
        _data.insert(_data.begin() + row * _cols + (_cols - 1), defaultValue);
    }
}

bool FastTableData::removeColumn(const QString& name) {
    auto it = std::find(_colNames.begin(), _colNames.end(), name);
    if (it == _colNames.end())
        return false;
    int col = std::distance(_colNames.begin(), it);
    // Remove data for this column in each row
    for (int row = _rows - 1; row >= 0; --row) {
        _data.erase(_data.begin() + row * _cols + col);
    }
    _colNames.erase(_colNames.begin() + col);
    _colIsNumeric.erase(_colIsNumeric.begin() + col);
    _colMinMax.erase(_colMinMax.begin() + col);
    if (_primaryKeyCol == col) _primaryKeyCol = -1;
    else if (_primaryKeyCol > col) _primaryKeyCol--;
    _cols -= 1;
    return true;
}

// Removed FastTableData::fromVariantMap implementation. Use createTableFromVariantMap in TableDataUtils instead.

// See TableDataUtils.cpp for table creation from QVariantMap.