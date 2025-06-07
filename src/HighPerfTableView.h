#pragma once

#include <QTableView>
#include <QAbstractTableModel>
#include <memory>
#include <vector>
#include <QMap>
#include <QKeyEvent>
#include <QMenu>
#include <QClipboard>
#include <QFileDialog>
#include <QAction>
#include <QList>
#include <QVariant>
#include <QTimer>
#include <QColor>
#include "CorrelationBarDelegate.h"
#include "FastTableData.h"
#include "HighPerfTableModel.h"
#include "TableDataUtils.h"

/**
 * @brief HighPerfTableView is a QTableView for FastTableData, supporting bar/value toggle, sorting, selection, and export.
 */
class HighPerfTableView : public QTableView {
    Q_OBJECT
public:
    explicit HighPerfTableView(QWidget* parent = nullptr);

    HighPerfTableModel* model() const;
    void setData(const FastTableData& data);

    void setBarDelegateForNumericalColumns(bool enabled);
    void setBarDelegateForColumn(int column, bool enabled, float minValue = -1.0f, float maxValue = 1.0f);

    void setShowBars(bool show);
    bool showBars() const;
    void setBarDelegateDisplayMode(bool showBars);

    bool exportToFile(QWidget* parent = nullptr, const QString& filePath = QString(), const QString& format = "csv");

    void addColumn(const QString& name, const FastTableData::Value& defaultValue = FastTableData::Value{});
    bool removeColumn(const QString& name);

    QColor currentTableBackgroundColor() const;

signals:
    void selectionChangedWithValues(const QList<QVariantList>& selectedValues);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    HighPerfTableModel* _model;
    QMap<int, CorrelationBarDelegate*> _barDelegates;

    void setupSelectionMode();
    void copySelectedRowsToClipboard(bool asCsv = false);
    QString serializeRows(const QList<int>& rows, const QString& delimiter) const;

    bool _showBars = true;

    void setupLazyLoading();
    void handleVerticalScroll();
    void handleHorizontalScroll();
    QTimer _lazyLoadTimer;
    int _lazyLoadThresholdRows = 100;
    int _lazyLoadThresholdCols = 10;
};