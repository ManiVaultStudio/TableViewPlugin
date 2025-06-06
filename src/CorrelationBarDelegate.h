#pragma once

#include <QStyledItemDelegate>
#include <QApplication>
#include <QToolTip>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QHelpEvent>

/**
 * @brief CorrelationBarDelegate draws a bar for numerical values, using row color if available.
 */
class CorrelationBarDelegate : public QStyledItemDelegate {
public:
    enum class DisplayMode { Bar, Number };

    CorrelationBarDelegate(float minCorr, float maxCorr, QObject* parent = nullptr, DisplayMode mode = DisplayMode::Bar)
        : QStyledItemDelegate(parent), minValue(minCorr), maxValue(maxCorr), _displayMode(mode) {}

    void setDisplayMode(DisplayMode mode) { _displayMode = mode; }
    DisplayMode displayMode() const { return _displayMode; }

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    bool helpEvent(QHelpEvent* event, QAbstractItemView* view,
        const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    float minValue, maxValue;
    DisplayMode _displayMode;

    bool isColorContrastive(const QColor& c1, const QColor& c2) const;
};