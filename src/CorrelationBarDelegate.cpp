#include "CorrelationBarDelegate.h"
#include "FastTableData.h"
#include <QAbstractItemModel>

void CorrelationBarDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    // Use UserRole+1 for bar value, but always paint bar if delegate is set
    if (index.data(Qt::UserRole + 1).isValid()) {
        float value = index.data(Qt::UserRole + 1).toFloat();

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

        // Get row bar color if available
        QColor barColor = QColor(255, 140, 0); // Default orange
        // Try to get row color from FastTableData if model supports it
        const QAbstractItemModel* model = index.model();
        if (model) {
            QVariant rowColorVar = model->data(model->index(index.row(), 0), Qt::BackgroundRole);
            if (rowColorVar.canConvert<QColor>()) {
                barColor = rowColorVar.value<QColor>();
            }
        }

        QColor axisColor;
        auto isColorClose = [](const QColor& c1, const QColor& c2, int threshold = 16) {
            return (std::abs(c1.red() - c2.red()) < threshold &&
                std::abs(c1.green() - c2.green()) < threshold &&
                std::abs(c1.blue() - c2.blue()) < threshold);
        };
        bool bgIsBlack = isColorClose(bgColor, QColor(Qt::black));
        bool bgIsWhite = isColorClose(bgColor, QColor(Qt::white));
        if (bgIsBlack) {
            axisColor = QColor(Qt::white);
        }
        else if (bgIsWhite) {
            axisColor = QColor(Qt::black);
        }
        else {
            double luminance = (0.299 * bgColor.red() + 0.587 * bgColor.green() + 0.114 * bgColor.blue()) / 255.0;
            axisColor = (luminance > 0.5) ? QColor(Qt::black) : QColor(Qt::white);
        }

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
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool CorrelationBarDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view,
    const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (index.data(Qt::UserRole + 1).isValid()) {
        float value = index.data(Qt::UserRole + 1).toFloat();
        // Explicitly cast view to QWidget* for QToolTip::showText
        QToolTip::showText(event->globalPos(), QString::number(value, 'g', 4), (QWidget*)view);
        return true;
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}