#ifndef CBOARD_H
#define CBOARD_H

#include "cscene.h"
#include "cfield.h"
#include "csettings.h"

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QHash>

// Виджет доски отображения игры, отвечает за взаимодействие с пользователем
// и управление игрой. Владеет классом CField, непосредственно манипулирующего
// данными игры, и объектом сцены CScene

class CBoard : public QGraphicsView
{
    Q_OBJECT
    friend class CGame;
public:
    explicit CBoard(CScene *scene, CField *field, QWidget *parent = nullptr);

    // Сбросить все параметры игры
    void newGame();

    // Установить состояние игры
    void setGameState(GameState state) { m_game_state = state; }

    // Показать/скрыть сообщение
    void showMessage(const QString &message, bool is_hide_tiles);
    void hideMessage(bool is_show_tiles);

signals:
    void signalUpdateInfo();
    void signalUndoRedo(bool, bool);
    void signalShowResult(int);
    void signalClick();
    void signalVictory();
    void signalNotVariant();

public slots:
    void slotHint();
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
    enum PathState { psNone, psPath, psHint, psDemostration };

    QPixmap m_bg_pixmap;
    GameState m_game_state;
    GameState m_prev_pause_state;
    CField *m_field;
    CScene *m_scene;
    // Текущее состояние пути
    PathState m_path_state{psNone};
    // Текущее сообщение
    QString m_message;
    // Таймер пути
    int m_path_timer;
    // Таймер демонстрации
    int m_demostration_timer;
    // Размеры прямоугольника сообщений
    QRectF m_message_rect;
    // Размер шрифта сообщения в пикселях
    int m_message_font_pixel;
    // Текущий индекс демонстрции
    int m_demostration_index;
    // Старый путь (для демонстрации)
    CoordList m_old_path_coords;
    // Использовались ли хитрости
    bool m_is_cunning;

    // Установить фоновое изображение
    void setBackground(const QString &file_name);

    // Пересчитать view (вызвается при инициализации и каждом изменении размера)
    void recalcView();

    // Обработчики мыши
    void clickLeftButton(QMouseEvent *);
    void clickRightButton(QMouseEvent *);

    // Нарисовать путь
    void paintPath(QPainter *painter);
    void repaintPath();

    // Запустить таймер рисования пути
    void doStartPath(const TilePair &tiles);
    // Окончание таймера пути
    void doFinishPath();

    // Показать hint
    void showHint();

    // Нарисовать окно сообщений
    void paintMessage(QPainter *painter) const;

    // Демонстрация
    void startDemonstration();
    void doDemonstration();
    void clearDemostrationTiles(const TilePair &tiles);
    void stopDemonstration();

private slots:
    void slotStartConnect(const TilePair &tiles);

};

#endif // CBOARD_H
