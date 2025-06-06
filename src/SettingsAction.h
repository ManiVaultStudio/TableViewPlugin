#pragma once

#include <QVariantMap>
#include "FastTableData.h"


// Use this function to create FastTableData from a QVariantMap
FastTableData createTableFromVariantMap(const QVariantMap& map);