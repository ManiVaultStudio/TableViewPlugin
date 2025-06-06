#pragma once

#include <vector>
#include <QString>
#include <variant>
#include <QColor>
#include <QVariantMap>
#include <QVariantList>

/**
 * @brief FastTableData is a high-performance, flat, row-major table structure for large datasets.
 * Supports per-row bar colors, column meta-data, and primary key designation.
 *
 * You can construct a table from a QVariantMap using createTableFromVariantMap in TableDataUtils.h.
 * The expected structure is:
 *   - Keys: column names (QString)
 *   - Values: QVariantList (each list contains the column's values, all lists must have the same length)
 * Example:
 *   QVariantMap map;
 *   map["col1"] = QVariantList{1,2,3};
 *   map["col2"] = QVariantList{"a","b","c"};
 *   FastTableData table = createTableFromVariantMap(map);
 */
class FastTableData {
public:
    using Value = std::variant<double, int, QString>;

    FastTableData() = default;
    FastTableData(int rows, int cols);

    // Resize the table and all meta-data vectors
    void resize(int rows, int cols);

    // Table dimensions
    int rowCount() const { return _rows; }
    int colCount() const { return _cols; }

    // Value access
    void set(int row, int col, const Value& v);
    const Value& get(int row, int col) const;

    // Column meta-data
    void setColumnName(int col, const QString& name);
    QString columnName(int col) const;

    void setColumnIsNumeric(int col, bool isNumeric);
    bool columnIsNumeric(int col) const;

    void setColumnMinMax(int col, double minVal, double maxVal);
    void getColumnMinMax(int col, double& minVal, double& maxVal) const;

    // Primary key support
    void setPrimaryKeyColumn(int col);
    bool isPrimaryKeyColumn(int col) const;
    int primaryKeyColumn() const;

    // Row/column access
    std::vector<Value> getRow(int row) const;
    std::vector<Value> getColumn(int col) const;
    std::vector<std::vector<Value>> getRows() const;
    std::vector<std::vector<Value>> getColumns() const;

    // Per-row bar color (all columns in a row share the same color)
    void setRowBarColor(int row, const QColor& color);
    QColor rowBarColor(int row) const;
    void setAllRowBarColors(const std::vector<QColor>& colors);
    const std::vector<QColor>& getAllRowBarColors() const;

    // Filtering: set/get row visibility (for filtering/search)
    void setRowVisible(int row, bool visible);
    bool isRowVisible(int row) const;
    void clearRowFilter();

    // Utility: clear all data and meta-data
    void clear();

    /**
     * @deprecated Use createTableFromVariantMap in TableDataUtils.h instead.
     * This method will be removed in a future release. Use createTableFromVariantMap directly.
     */
    [[deprecated("Use createTableFromVariantMap in TableDataUtils.h instead. This method will be removed in a future release.")]]
    static FastTableData fromVariantMap(const QVariantMap& map) = delete; // Deleted, use createTableFromVariantMap

    // Optionally: construct from a row-oriented structure
    // static FastTableData fromVariantRows(const QVariantList& rows);

    // Lazy loading API
    bool canFetchMoreRowsTop(int n) const;
    bool canFetchMoreRowsBottom(int n) const;
    bool canFetchMoreColsLeft(int n) const;
    bool canFetchMoreColsRight(int n) const;
    void fetchMoreRowsTop(int n);
    void fetchMoreRowsBottom(int n);
    void fetchMoreColsLeft(int n);
    void fetchMoreColsRight(int n);

private:
    int _rows = 0, _cols = 0;
    std::vector<Value> _data;
    std::vector<QString> _colNames;
    std::vector<bool> _colIsNumeric;
    std::vector<std::pair<double, double>> _colMinMax;
    int _primaryKeyCol = -1; // -1 means no primary key
    std::vector<QColor> _rowBarColors; // color for each row, used by all columns in that row
    std::vector<bool> _rowVisible;     // row filtering support
};