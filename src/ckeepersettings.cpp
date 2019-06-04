#include "ckeepersettings.h"

#include <QDir>
#include <QMainWindow>
#include <QScreen>
#include <QApplication>

static const QString GEOMETRY_GROUP = "Geometry";

// Тип указателя на метод класса
using ExitProc = void (CKeeperSettings::*)(QObject *);

// Фильтр событий для окон, сохраняющих свою позицию в конфигурации
class CWidgetEventFilter : public QObject
{
public:
    CWidgetEventFilter(CKeeperSettings *keeper, const ExitProc &exit_proc)
        : QObject(),
        m_keeper(keeper),
        m_exit_proc(exit_proc) {}
    ~CWidgetEventFilter() override = default;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    CKeeperSettings *m_keeper;
    ExitProc m_exit_proc;
};

bool CWidgetEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Close || event->type() == QEvent::HideToParent) {
        (m_keeper->*m_exit_proc)(obj);
        event->accept();
    }
    return QObject::eventFilter(obj, event);
}

// ------------------------------------------------------------------------------------------------
// Окно будет закрываться - получено сообщение от фильтра
void CKeeperSettings::doExit(QObject *obj)
{
    auto widget = qobject_cast<QWidget *>(obj);
    beginGroup(GEOMETRY_GROUP);
    auto rect = QRect(widget->pos(), widget->size());
    setValue(widget->objectName(), rect);
    endGroup();
}

// ------------------------------------------------------------------------------------------------
// Зарегистрировать окно как желающего сохранять свой размер
void CKeeperSettings::registerGeometry(QWidget *widget)
{
    // Окну установим фильтр событий и передадим указатель на метод, который
    // будет вызван, если оно попробует закрыться
    widget->installEventFilter(new CWidgetEventFilter(this, &CKeeperSettings::doExit));

    // Прочитаем состояние геометрии окна
    beginGroup(GEOMETRY_GROUP);
    auto rect = value(widget->objectName(), QApplication::primaryScreen()->availableGeometry()).toRect();
    widget->resize(rect.size());
    widget->move(rect.topLeft());
    endGroup();
}

