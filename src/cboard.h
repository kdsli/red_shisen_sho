#ifndef CBOARD_H
#define CBOARD_H

#include "cscene.h"
#include "cfield.h"
#include "csettings.h"

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QHash>

// Запись за тип поля
struct FieldRec
{
    // Размеры поля
    quint16 x;
    quint16 y;
    // Количество одинаковых костяшек для каждого типа
    quint16 count;
};

// Виджет доски отображения игры, отвечает за взаимодействие с пользователем
// и управление игрой. Владеет классом CField, непосредственно манипулирующего
// данными игры, и объектом сцены CScene

class CBoard : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CBoard(QWidget *parent = nullptr);

    // Состояние игры
    enum GameState { gsEmpty, gsNormal, gsPause, gsVictory, gsNotVariants, gsDemostration };

    QString gameInfo();

signals:
    void signalUpdateInfo();
    void signalUndoRedo(bool, bool);
    void signalShowResult(int);

public slots:
    void slotNewGame();
    void slotRepeatGame();
    void slotHint();
    void slotPause();
    void slotNewTypeGame();
    void slotSetTileset();
    void slotSetBackground();
    void slotUndo();
    void slotRedo();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    // Тип пути, который показывается в настоящее время
    enum PathType {ptNone, ptPath, ptHint, ptDemostration};

    QPixmap m_bg_pixmap;
    GameState m_game_state;
    GameState m_prev_pause_state;
    CField *m_field;
    CScene *m_scene;
    // Данные типов игр
    QHash<GameType, FieldRec> m_field_types;
    // Таймер игры
    int m_game_timer;
    // Таймер пути
    int m_path_timer;
    // Таймер демонстрации
    int m_demostration_timer;
    // Секунды игры
    int m_seconds;
    // Текущий тип пути
    PathType m_path_type{ptNone};
    // Использовались ли хитрости
    bool m_is_cunning;
    // Сообщение
    QString m_message;
    // Размеры прямоугольника сообщений
    QRectF m_message_rect;
    // Размер шрифта сообщения в пикселях
    int m_message_font_pixel;
    // Текущий индекс демонстрции
    int m_demostration_index;
    // Старый путь (для демонстрации)
    CoordList m_old_path_coords;

    // Установить фоновое изображение
    void setBackground(const QString &file_name);

    // Пересчитать view (вызвается при инициализации и каждом изменении размера)
    void recalcView();

    // Обработчики мыши
    void clickLeftButton(QMouseEvent *);
    void clickRightButton(QMouseEvent *);

    // Ситуации в игре
    void doNewGame();
    void doVictory();
    void doNotVariant();

    // Нарисовать путь
    void paintPath(QPainter *painter);
    void repaintPath();

    // Запустить таймер рисования пути
    void doStartPath(const TilePair &tiles);
    // Окончание таймера пути
    void doFinishPath();

    // Показать hint
    void showHint();

    // Проверить и показать результаты
    void checkResult();

    // Остановить таймер игры
    void stopGameTimer();

    // Секунды в строку
    QString getSecondsString();

    // Нарисовать окно сообщений
    void paintMessage(QPainter *painter) const;

    // Показать/скрыть сообщение
    void showMessage(const QString &message, bool is_hide_tiles = false);
    void hideMessage(bool is_show_tiles);

    // Демонстрация
    void startDemonstration();
    void doDemonstration();
    void clearDemostrationTiles(const TilePair &tiles);
    void closeDemonstration();

private slots:
    void slotStartConnect(const TilePair &tiles);

};

#endif // CBOARD_H
