#include "HighPerfTableView.h"
#include <QHeaderView>
#include <QScrollBar>
#include <QDebug>
#include "CorrelationBarDelegate.h"
#include <QKeyEvent>
#include <QItemSelectionModel>
#include <QContextMenuEvent>
#include <QClipboard>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QAction>
#include <QMenu>
#include <QFileDialog>
#include "TableDataUtils.h"

HighPerfTableView::HighPerfTableView(QWidget* parent)
    : QTableView(parent)
    , _model(new HighPerfTableModel(this))
    , _showBars(true)
{
    setModel(_model);
    setupSelectionMode();
    setSortingEnabled(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(20);
    setAlternatingRowColors(true);
    _model->setShowBars(true);

    if (selectionModel()) {
        connect(selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &HighPerfTableView::onSelectionChanged);
    }

    setupLazyLoading();
}

void HighPerfTableView::setupSelectionMode()
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
}

HighPerfTableModel* HighPerfTableView::model() const {
    return _model;
}

void HighPerfTableView::setData(const FastTableData& data) {
    _model->setData(data);
    setBarDelegateForNumericalColumns(_model->showBars());
}

void HighPerfTableView::setBarDelegateForNumericalColumns(bool enabled)
{
    for (auto it = _barDelegates.begin(); it != _barDelegates.end(); ++it) {
        this->setItemDelegateForColumn(it.key(), nullptr);
        delete it.value();
    }
    _barDelegates.clear();

    if (!enabled || !_model)
        return;

    for (int col = 0; col < _model->columnCount(); ++col) {
        if (_model->isNumericalColumn(col)) {
            float minVal, maxVal;
            _model->getColumnMinMax(col, minVal, maxVal);
            auto* delegate = new CorrelationBarDelegate(
                minVal, maxVal, this,
                _showBars ? CorrelationBarDelegate::DisplayMode::Bar : CorrelationBarDelegate::DisplayMode::Number
            );
            this->setItemDelegateForColumn(col, delegate);
            _barDelegates[col] = delegate;
        }
    }
}

void HighPerfTableView::setBarDelegateForColumn(int column, bool enabled, float minValue, float maxValue)
{
    if (_barDelegates.contains(column)) {
        this->setItemDelegateForColumn(column, nullptr);
        delete _barDelegates[column];
        _barDelegates.remove(column);
    }
    if (enabled) {
        auto* delegate = new CorrelationBarDelegate(minValue, maxValue, this);
        this->setItemDelegateForColumn(column, delegate);
        _barDelegates[column] = delegate;
    }
}

void HighPerfTableView::setBarDelegateDisplayMode(bool showBars)
{
    _showBars = showBars;
    for (auto it = _barDelegates.begin(); it != _barDelegates.end(); ++it) {
        if (it.value()) {
            it.value()->setDisplayMode(
                showBars ? CorrelationBarDelegate::DisplayMode::Bar : CorrelationBarDelegate::DisplayMode::Number
            );
        }
    }
    viewport()->update();
}

void HighPerfTableView::setShowBars(bool show)
{
    auto* m = qobject_cast<HighPerfTableModel*>(model());
    if (m) {
        m->setShowBars(show);
        if (m->showBars() == show) {
            setBarDelegateDisplayMode(show);
            setBarDelegateForNumericalColumns(show);
            viewport()->update();
        }
    }
}

bool HighPerfTableView::showBars() const
{
    auto* m = qobject_cast<HighPerfTableModel*>(model());
    return m ? m->showBars() : false;
}

void HighPerfTableView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_R && !event->modifiers()) {
        selectionModel()->clearSelection();
        event->accept();
        return;
    }
    QTableView::keyPressEvent(event);
}

void HighPerfTableView::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    QAction* copyAction = menu.addAction(tr("Copy Selected Rows"));
    QAction* exportAction = menu.addAction(tr("Export Table..."));
    QAction* toggleBarsAction = menu.addAction(showBars() ? tr("Show Values") : tr("Show Bars"));

    QAction* chosen = menu.exec(event->globalPos());
    if (chosen == copyAction) {
        copySelectedRowsToClipboard(true);
    } else if (chosen == exportAction) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Export Table"), QString(), tr("CSV Files (*.csv);;TSV Files (*.tsv);;All Files (*)"));
        if (!fileName.isEmpty()) {
            QString format = "csv";
            if (fileName.endsWith(".tsv", Qt::CaseInsensitive)) format = "tsv";
            if (!exportToFile(this, fileName, format)) {
                QMessageBox::warning(this, tr("Export Failed"), tr("Failed to export table to file."));
            }
        }
    } else if (chosen == toggleBarsAction) {
        setShowBars(!showBars());
    }
}

void HighPerfTableView::copySelectedRowsToClipboard(bool asCsv)
{
    auto selModel = selectionModel();
    if (!selModel) return;
    auto selRows = selModel->selectedRows();
    if (selRows.isEmpty()) return;

    QList<int> rowIndices;
    for (const QModelIndex& idx : selRows)
        rowIndices << idx.row();

    QString delimiter = asCsv ? "," : "\t";
    QString text = serializeRows(rowIndices, delimiter);

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

QString HighPerfTableView::serializeRows(const QList<int>& rows, const QString& delimiter) const
{
    QStringList lines;
    QStringList header;
    for (int c = 0; c < _model->columnCount(); ++c)
        header << _model->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString();
    lines << header.join(delimiter);

    for (int r : rows) {
        QStringList fields;
        for (int c = 0; c < _model->columnCount(); ++c) {
            QVariant v = _model->data(_model->index(r, c), Qt::DisplayRole);
            QString s = v.toString();
            if (delimiter == "," && s.contains(',')) s = "\"" + s + "\"";
            fields << s;
        }
        lines << fields.join(delimiter);
    }
    return lines.join("\n");
}

bool HighPerfTableView::exportToFile(QWidget* parent, const QString& filePath, const QString& format)
{
    QString outPath = filePath;
    if (outPath.isEmpty()) {
        outPath = QFileDialog::getSaveFileName(parent ? parent : this, tr("Export Table"), QString(), tr("CSV Files (*.csv);;TSV Files (*.tsv);;All Files (*)"));
        if (outPath.isEmpty()) return false;
    }
    QString delimiter = (format == "tsv" || outPath.endsWith(".tsv", Qt::CaseInsensitive)) ? "\t" : ",";
    QList<int> allRows;
    for (int r = 0; r < _model->rowCount(); ++r) allRows << r;
    QString text = serializeRows(allRows, delimiter);

    QFile file(outPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream ts(&file);
    ts << text;
    file.close();
    return true;
}

void HighPerfTableView::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    auto selModel = selectionModel();
    if (!selModel) return;
    auto selRows = selModel->selectedRows();
    QList<QVariantList> selectedValues;

    int firstCol = 0;
    int pkCol = _model->primaryKeyColumn();

    for (const QModelIndex& idx : selRows) {
        QVariantList rowValues;
        rowValues << _model->data(_model->index(idx.row(), firstCol), Qt::DisplayRole);
        if (pkCol != -1 && pkCol != firstCol) {
            rowValues << _model->data(_model->index(idx.row(), pkCol), Qt::DisplayRole);
        }
        selectedValues << rowValues;
    }
    emit selectionChangedWithValues(selectedValues);
}

void HighPerfTableView::setupLazyLoading()
{
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int) {
        _lazyLoadTimer.start(50);
    });
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, [this](int) {
        _lazyLoadTimer.start(50);
    });
    _lazyLoadTimer.setSingleShot(true);
    connect(&_lazyLoadTimer, &QTimer::timeout, this, [this]() {
        handleVerticalScroll();
        handleHorizontalScroll();
    });
}

void HighPerfTableView::handleVerticalScroll()
{
    int firstVisible = rowAt(0);
    int lastVisible = rowAt(viewport()->height() - 1);
    if (firstVisible < 0 || lastVisible < 0) return;

    int totalRows = _model->rowCount();
    if (firstVisible < _lazyLoadThresholdRows) {
        _model->requestMoreRowsTop(_lazyLoadThresholdRows);
    }
    if (lastVisible > totalRows - _lazyLoadThresholdRows) {
        _model->requestMoreRowsBottom(_lazyLoadThresholdRows);
    }
}

void HighPerfTableView::handleHorizontalScroll()
{
    int firstVisible = columnAt(0);
    int lastVisible = columnAt(viewport()->width() - 1);
    if (firstVisible < 0 || lastVisible < 0) return;

    int totalCols = _model->columnCount();
    if (firstVisible < _lazyLoadThresholdCols) {
        _model->requestMoreColsLeft(_lazyLoadThresholdCols);
    }
    if (lastVisible > totalCols - _lazyLoadThresholdCols) {
        _model->requestMoreColsRight(_lazyLoadThresholdCols);
    }
}

void HighPerfTableView::addColumn(const QString& name, const FastTableData::Value& defaultValue) {
    if (_model) {
        _model->addColumn(name, defaultValue);
    }
}

bool HighPerfTableView::removeColumn(const QString& name) {
    if (_model) {
        bool result = _model->removeColumn(name);
        return result;
    }
    return false;
}

QColor HighPerfTableView::currentTableBackgroundColor() const
{
    // Use palette to get the current background color for the table
    return palette().color(QPalette::Base);
}