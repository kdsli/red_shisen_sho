#ifndef CBOARD_H
#define CBOARD_H

#include "csettings.h"
#include "csettings.h"
#include "ctileset.h"
#include "cfield.h"

#include <QWidget>
#include <QHash>

QT_BEGIN_NAMESPACE
class QSvgRenderer;
class QGraphicsSvgItem;
QT_END_NAMESPACE

// Класс доски отображения. Отвечает за отображение поля и взимодействие
// с игроком. Оперирует координатами виджета

class CBoard : public QWidget
{
    Q_OBJECT
public:
    explicit CBoard(QWidget *parent = nullptr);

    // Состояние игры
    enum GameState { gsEmpty, gsNormal, gsPause, gsVictory, gsNotVariants, gsDemostration };

    QString gameInfo();

public slots:
    void slotNewGame();
    void slotNewTypeGame();
    void slotRepeatGame();
    void slotHelp();
    void slotPause();
    void slotSetTileset();
    void slotSetBackground();

signals:
    void signalUpdateInfo();
    void signalUndoRedo(bool, bool);
    void signalUndo();
    void signalRedo();
    void signalBackgroundNotFound();
    void signalShowResult(int);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    GameState m_game_state;
    // Старое значение статуса игры
    GameState m_old_game_state;
    // Фон
    QSvgRenderer *m_bg_renderer;
    QGraphicsSvgItem *m_bg_svg;
    QPixmap m_bg_pixmap;
    // Характеристики текущего поля
    QRect m_current_field_rect;
    // Костяшки
    CTileSet *m_ts;
    // Игровое поле
    CField *m_field;
    // Левая верхняя точка отображения костяшек
    QPoint m_left_top;
    // Костяшки, которые будут убираться
    TilePair m_deleted_tile;
    // Признак того, что показан путь
    bool m_is_path = false;
    // Признак подсказки
    bool m_is_help = false;
    // Размеры прямоугольника сообщений
    QRect m_message_rect;
    // Размер шрифта сообщения в пикселях
    int m_font_pixel;
    // ID таймера показа пути
    int m_timer_path;
    // ID таймера учета игрового времени
    int m_timer_game;
    // ID таймера демонстрации
    int m_timer_demostration;
    // Количество секунд с начала игры
    int m_second;
    // Текущий шаг демонстрации
    int m_demostration_index;
    // Предыдущий путь для демонстрации
    TileList m_old_path;

    bool initBackground();

    // Провести расчет доски при изменении размера
    void recalcBoardSize(int width, int height);
    // Общие действия для новой игры
    void doNewGame();

    // Найти координаты в поле по координатам виджета
    Tile getTileIndex(QPointF) const;
    // Найти прямоугольник костяшки в координатах виджета (обратная предудыщей)
    QRectF getTileRect(Tile) const;
    QRectF getBaseRect(Tile) const;

    // Рисование
    void paintBackground(QPainter &painter) const;
    void paintTiles(QPainter &painter, QPaintEvent *event) const;
    void paintPath(QPainter &painter) const;
    void paintMessage(QPainter &painter) const;

    // Обработчики мыши
    void clickLeftButton(QMouseEvent *);
    void clickRightButton(QMouseEvent *);

    // Сформировать регион обновления для пути
    QRegion setPathRegion(const TileList &path);

    QString getMessageText() const;
    QString getNotVariantText() const;

    // Таймеры
    void doTimerPath();
    void doTimerGame();
    void doTimerDemostration();

    // Запустить демонстрацию
    void startDemonstration();
    void closeDemonstration();

    // Шаманство с координатами линий, чтобы не сильно уходили за границу поля
    void correctPoint(QPointF &point) const;

    // Проверим результаты
    void checkResult();

private slots:
    void slotRepaint();
    void slotStartConnect(const TilePair &);
    void slotNotVariants();
    void slotVictory();
};

#endif // CBOARD_H
