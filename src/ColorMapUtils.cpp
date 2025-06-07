#include "ColorMapUtils.h"
#include <algorithm>
#include <cmath>

// Helper: interpolate between two colors
static QColor lerp(const QColor& a, const QColor& b, float t) {
    return QColor(
        a.red()   + (b.red()   - a.red())   * t,
        a.green() + (b.green() - a.green()) * t,
        a.blue()  + (b.blue()  - a.blue())  * t
    );
}

QColor getColormapColor(CorrelationBarDelegate::ColorMapType type, float norm) {
    norm = std::clamp(norm, 0.0f, 1.0f);
    switch (type) {
    case CorrelationBarDelegate::ColorMapType::Viridis: // Default
        // Approximate Viridis
        return lerp(QColor(68,1,84), QColor(253,231,37), norm);
    case CorrelationBarDelegate::ColorMapType::Magma:
        return lerp(QColor(0,0,3), QColor(251,252,191), norm);
    case CorrelationBarDelegate::ColorMapType::Plasma:
        return lerp(QColor(13,8,135), QColor(240,249,33), norm);
    case CorrelationBarDelegate::ColorMapType::BrBG:
        return lerp(QColor(140,81,10), QColor(1,102,94), norm);
    case CorrelationBarDelegate::ColorMapType::BuPu:
        return lerp(QColor(247,252,253), QColor(136,65,157), norm);
    case CorrelationBarDelegate::ColorMapType::GnBu:
        return lerp(QColor(240,249,232), QColor(8,64,129), norm);
    case CorrelationBarDelegate::ColorMapType::PiYG:
        return lerp(QColor(197,27,125), QColor(77,146,33), norm);
    case CorrelationBarDelegate::ColorMapType::PuOr:
        return lerp(QColor(241,163,64), QColor(153,142,195), norm);
    case CorrelationBarDelegate::ColorMapType::Q_BlGrRd:
        return lerp(QColor(44,123,182), QColor(215,25,28), norm);
    case CorrelationBarDelegate::ColorMapType::Qualitative:
        // Just cycle a few qualitative colors
        if (norm < 0.2f) return QColor(31,119,180);
        if (norm < 0.4f) return QColor(255,127,14);
        if (norm < 0.6f) return QColor(44,160,44);
        if (norm < 0.8f) return QColor(214,39,40);
        return QColor(148,103,189);
    case CorrelationBarDelegate::ColorMapType::RdBu:
        return lerp(QColor(178,24,43), QColor(33,102,172), norm);
    case CorrelationBarDelegate::ColorMapType::RdPu:
        return lerp(QColor(253,224,221), QColor(134,1,175), norm);
    case CorrelationBarDelegate::ColorMapType::RdYlBu:
        return lerp(QColor(252,141,89), QColor(145,191,219), norm);
    case CorrelationBarDelegate::ColorMapType::RdYlGn:
        return lerp(QColor(252,141,89), QColor(145,207,96), norm);
    case CorrelationBarDelegate::ColorMapType::Reds:
        return lerp(QColor(254,229,217), QColor(165,15,21), norm);
    case CorrelationBarDelegate::ColorMapType::Spectral:
        return lerp(QColor(158,1,66), QColor(94,79,162), norm);
    case CorrelationBarDelegate::ColorMapType::YlGn:
        return lerp(QColor(255,255,229), QColor(0,104,55), norm);
    case CorrelationBarDelegate::ColorMapType::YlGnBu:
        return lerp(QColor(255,255,217), QColor(8,29,88), norm);
    case CorrelationBarDelegate::ColorMapType::YlOrBr:
        return lerp(QColor(255,247,188), QColor(140,81,10), norm);
    default:
        return Qt::white;
    }
}