#pragma once

#include <QAbstractTableModel>
#include <QColor>
#include "FastTableData.h"

/**
 * @brief HighPerfTableModel provides a Qt model for FastTableData, supporting bar/value toggle and sorting.
 */
class HighPerfTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit HighPerfTableModel(QObject* parent = nullptr);

    void setData(const FastTableData& data);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool isNumericalColumn(int col) const;
    void getColumnMinMax(int col, float& minVal, float& maxVal) const;

    void setShowBars(bool show);
    bool showBars() const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    int primaryKeyColumn() const;

    void requestMoreRowsTop(int n);
    void requestMoreRowsBottom(int n);
    void requestMoreColsLeft(int n);
    void requestMoreColsRight(int n);

    void addColumn(const QString& name, const FastTableData::Value& defaultValue = FastTableData::Value{});
    bool removeColumn(const QString& name);

private:
    FastTableData _data;
    bool _showBars = false;
};