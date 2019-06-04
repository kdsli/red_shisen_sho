#ifndef CKEEPERSETTINGS_H
#define CKEEPERSETTINGS_H

#include <QSettings>
#include <QWidget>
#include <QVector>

// Класс, потомок QSettings, который так же умеет сохранять позицию окна
// Классы с поределениями данных конкретных программ могут создаваться как потомки этого

class CKeeperSettings : public QSettings
{
public:
    void registerGeometry(QWidget *widget);

private:
    void doExit(QObject *obj);

    QString writeGeometry(QWidget);
};

#endif // CKEEPERSETTINGS_H

