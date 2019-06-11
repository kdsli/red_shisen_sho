#include "common_types.h"

#include <QTime>

// ------------------------------------------------------------------------------------------------
// Секунды в строку
QString getSecondsString(int seconds)
{
    QTime t(0, 0, 0);
    t = t.addSecs(seconds);

    return t.toString("hh:mm:ss");
}

