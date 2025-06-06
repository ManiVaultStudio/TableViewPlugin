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

    // Toggle between showing bars and numerical values for all cells
    void setShowBars(bool show);
    bool showBars() const;

    // Add this
    void setBarDelegateDisplayMode(bool showBars);

    // Export the whole table to a file (CSV or other formats)
    bool exportToFile(QWidget* parent = nullptr, const QString& filePath = QString(), const QString& format = "csv");

signals:
    // Emits a list of lists, each inner list contains the first column value and, if different, the primary key value for each selected row.
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

    // Copy selected rows to clipboard as tab-separated or CSV
    void copySelectedRowsToClipboard(bool asCsv = false);

    // Helper to serialize rows
    QString serializeRows(const QList<int>& rows, const QString& delimiter) const;

    // Track bar/value mode
    bool _showBars = true;

    // Lazy loading setup and handling
    void setupLazyLoading();
    void handleVerticalScroll();
    void handleHorizontalScroll();
    QTimer _lazyLoadTimer;
    int _lazyLoadThresholdRows = 100; // Number of rows from edge to trigger load
    int _lazyLoadThresholdCols = 10;  // Number of cols from edge to trigger load
};