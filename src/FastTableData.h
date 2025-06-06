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
 */
class FastTableData {
public:
    using Value = std::variant<double, int, QString>;

    FastTableData() = default;
    FastTableData(int rows, int cols);

    void resize(int rows, int cols);

    int rowCount() const { return _rows; }
    int colCount() const { return _cols; }

    void set(int row, int col, const Value& v);
    const Value& get(int row, int col) const;

    void setColumnName(int col, const QString& name);
    QString columnName(int col) const;

    void setColumnIsNumeric(int col, bool isNumeric);
    bool columnIsNumeric(int col) const;

    void setColumnMinMax(int col, double minVal, double maxVal);
    void getColumnMinMax(int col, double& minVal, double& maxVal) const;

    void setPrimaryKeyColumn(int col);
    bool isPrimaryKeyColumn(int col) const;
    int primaryKeyColumn() const;

    std::vector<Value> getRow(int row) const;
    std::vector<Value> getColumn(int col) const;
    std::vector<std::vector<Value>> getRows() const;
    std::vector<std::vector<Value>> getColumns() const;

    void setRowBarColor(int row, const QColor& color);
    QColor rowBarColor(int row) const;
    void setAllRowBarColors(const std::vector<QColor>& colors);
    const std::vector<QColor>& getAllRowBarColors() const;

    void setRowVisible(int row, bool visible);
    bool isRowVisible(int row) const;
    void clearRowFilter();

    void clear();

    [[deprecated("Use createTableFromVariantMap in TableDataUtils.h instead. This method will be removed in a future release.")]]
    static FastTableData fromVariantMap(const QVariantMap& map) = delete;

    bool canFetchMoreRowsTop(int n) const;
    bool canFetchMoreRowsBottom(int n) const;
    bool canFetchMoreColsLeft(int n) const;
    bool canFetchMoreColsRight(int n) const;
    void fetchMoreRowsTop(int n);
    void fetchMoreRowsBottom(int n);
    void fetchMoreColsLeft(int n);
    void fetchMoreColsRight(int n);

    void addColumn(const QString& name, const Value& defaultValue = Value{});
    bool removeColumn(const QString& name);

private:
    int _rows = 0, _cols = 0;
    std::vector<Value> _data;
    std::vector<QString> _colNames;
    std::vector<bool> _colIsNumeric;
    std::vector<std::pair<double, double>> _colMinMax;
    int _primaryKeyCol = -1;
    std::vector<QColor> _rowBarColors;
    std::vector<bool> _rowVisible;
};