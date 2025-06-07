#pragma once
#include <QColor>
#include "CorrelationBarDelegate.h"

// Returns a color from the selected colormap, given a normalized value [0,1]
QColor getColormapColor(CorrelationBarDelegate::ColorMapType type, float norm);