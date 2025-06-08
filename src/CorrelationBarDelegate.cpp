#include "CorrelationBarDelegate.h"
#include "FastTableData.h"
#include <QAbstractItemModel>

void CorrelationBarDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    if (index.data(Qt::UserRole + 1).isValid()) {
        float value = index.data(Qt::UserRole + 1).toFloat();

        if (_displayMode == DisplayMode::Bar) {
            float maxAbs = std::max(std::abs(minValue), std::abs(maxValue));
            if (maxAbs < 1e-6f) maxAbs = 1.0f;

            QRect barRect = option.rect.adjusted(4, 4, -4, -4);
            int centerX = barRect.left() + barRect.width() / 2;
            int barStartX = centerX;
            int barEndX = centerX + static_cast<int>((value / maxAbs) * (barRect.width() / 2));
            if (barEndX < barStartX) std::swap(barStartX, barEndX);

            QRect filledRect = barRect;
            filledRect.setLeft(barStartX);
            filledRect.setRight(barEndX);

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            QColor bgColor = (option.state & QStyle::State_Selected)
                ? option.palette.color(QPalette::Highlight)
                : option.palette.color(QPalette::Base);
            painter->fillRect(option.rect, bgColor);

            // Detect light or dark mode based on background color luminance
            double bgLuminance = (0.299 * bgColor.red() + 0.587 * bgColor.green() + 0.114 * bgColor.blue()) / 255.0;
            bool isDarkMode = (bgLuminance < 0.5);

            // Always use orange bars
            QColor barColor = QColor(255, 140, 0);

            // Axis color: black in light mode, white in dark mode
            QColor axisColor = isDarkMode ? QColor(Qt::white) : QColor(Qt::black);

            QPen axisPen(axisColor);
            axisPen.setWidth(2);
            axisPen.setStyle(Qt::DotLine);

            int axisExtra = std::max(2, barRect.height() / 8);
            int axisTop = barRect.top() - axisExtra;
            int axisBottom = barRect.bottom() + axisExtra;

            painter->setPen(axisPen);
            painter->drawLine(centerX, axisTop, centerX, axisBottom);

            if (std::abs(value) > 1e-6f) {
                painter->setPen(Qt::NoPen);
                painter->setBrush(barColor);
                painter->drawRect(filledRect.normalized());
            }

            if (option.state & QStyle::State_HasFocus) {
                QStyleOptionFocusRect focusOption;
                focusOption.QStyleOption::operator=(option);
                focusOption.rect = option.rect;
                focusOption.state |= QStyle::State_KeyboardFocusChange;
                focusOption.backgroundColor = bgColor;
                QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOption, painter);
            }

            painter->restore();
        } else {
            QColor bgColor = (option.state & QStyle::State_Selected)
                ? option.palette.color(QPalette::Highlight)
                : option.palette.color(QPalette::Base);
            QColor textColor = (option.state & QStyle::State_Selected)
                ? option.palette.color(QPalette::HighlightedText)
                : QColor(Qt::black);

            painter->save();
            painter->fillRect(option.rect, bgColor);
            painter->setPen(textColor);
            QString valueText = QString::number(value, 'g', 4);
            painter->drawText(option.rect.adjusted(4, 4, -4, -4), Qt::AlignCenter, valueText);
            painter->restore();
        }
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool CorrelationBarDelegate::isColorContrastive(const QColor& c1, const QColor& c2) const
{
    auto luminance = [](const QColor& c) {
        auto channel = [](int v) {
            double d = v / 255.0;
            return (d <= 0.03928) ? d / 12.92 : std::pow((d + 0.055) / 1.055, 2.4);
        };
        return 0.2126 * channel(c.red()) + 0.7152 * channel(c.green()) + 0.0722 * channel(c.blue());
    };
    double l1 = luminance(c1) + 0.05;
    double l2 = luminance(c2) + 0.05;
    double ratio = l1 > l2 ? l1 / l2 : l2 / l1;
    return ratio > 2.5;
}

bool CorrelationBarDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view,
    const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (index.data(Qt::UserRole + 1).isValid()) {
        float value = index.data(Qt::UserRole + 1).toFloat();
        QToolTip::showText(event->globalPos(), QString::number(value, 'g', 4), (QWidget*)view);
        return true;
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}