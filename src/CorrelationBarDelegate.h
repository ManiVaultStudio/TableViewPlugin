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
    CorrelationBarDelegate(float minCorr, float maxCorr, QObject* parent = nullptr)
        : QStyledItemDelegate(parent), minValue(minCorr), maxValue(maxCorr) {
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    bool helpEvent(QHelpEvent* event, QAbstractItemView* view,
        const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    float minValue, maxValue;
};