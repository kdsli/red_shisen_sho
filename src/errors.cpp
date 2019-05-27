#include "errors.h"

#include <QMessageBox>
#include <QApplication>

void CriticalError(const QString &message)
{
    QMessageBox::critical(nullptr, QObject::tr("Error"), message);
    qApp->exit();
}
