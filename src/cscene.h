#ifndef CSCENE_H
#define CSCENE_H

#include "common_types.h"

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsSvgItem>
#include <QPainter>

class CField;

class CScene : public QGraphicsScene
{
    Q_OBJECT
    friend class CBoard;
public:
    explicit CScene(CField *field, QObject *parent = nullptr);

    // Установить фоновое изображение
    void setBackground(const QString &file_name);

    // Новая игра
    void newGame();

    // Заполнить сцену костяшками
    void fillScene();

    // Показать/скрыть сообщение
    void showMessage(const QString &message, bool is_hide_tiles = false);
    void hideMessage(bool is_show_tiles);

    // Показать hint
    void showHint();

    // Обработка мыши
    void mouseLeft(QPointF point);
    void mouseRight(QPointF point);

    // Начать демонстрацию
    void startDemonstration();

signals:
    // Сигнал перерисовать путь
    void signalPaintPath();
    // Изменился статус программы
    void signalVariantStatus(VariantStatus);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;
    void timerEvent(QTimerEvent *event) override;

private:
    // Тип пути, который показывается в настоящее время
    enum PathType {ptNone, ptPath, ptHint, ptDemostration};

    QPixmap m_bg_pixmap;
    CField *m_field;
    // Rect поля (костяшки + margins)
    QRectF m_field_rect;
    // Список костяшек
    QVector<QGraphicsSvgItem *> m_tiles_list;
    // Сообщение
    QString m_message;
    // Размеры прямоугольника сообщений
    QRectF m_message_rect;
    // Размер шрифта сообщения в пикселях
    int m_message_font_pixel;
    // Выделенные в текущий момент костяшки
    TileList m_selected;
    // Таймер пути
    int m_path_timer;
    // Костяшки, которые нужно будет снять после показа пути
    TilePair m_deleted_tiles;
    // Текущий тип пути
    PathType m_path_type{ptNone};
    // Списоки текущего и прошлого (для демонстрации) пути в координатах сцены
    CoordList m_path_coords;
    CoordList m_old_path_coords;
    // Текущий индекс демонстрции
    int m_demostration_index;
    // Таймер демонстрации
    int m_demostration_timer;

    // Прересчитать логические координаты сцены
    void recalcScene();

    // Вернуть левый верхний угол ячейки
    const QPointF getTilePos(const Tile &tile) const;
    const QPointF getTilePos(const int index) const;
    // Вернуть центр ячейки
    const QPointF getTileCenter(const Tile &tile) const;

    // Показать/скрыть все плитки
    void setTilesVisible(bool visible);
    // Выделить или не выделить плитку
    void selectTile(int x, int y, bool selected) const;
    void selectTile(const Tile &tile, bool selected) const;

    // Нарисовать окно сообщений
    void paintMessage(QPainter *painter) const;

    // Рассчитать костяшку по координатам виджета
    Tile getTileIndex(const QPointF point);

    // Работа со списком выделенных костяшек
    void clearSelected();
    void addSelected(const Tile &tile);

    // Снимаем костяшки
    void removeTiles(const TilePair &tiles);

    // Запустить таймер рисования пути
    void doStartPath(const TilePair &tiles);
    // Окончание таймера пути
    void doFinishPath();

    // Нарисовать путь
    void paintPath(QPainter *painter);

    // Сдвинуть колонку вниз
    void columnMoveDown(const Tile &tile);

    // Демонстрация (по таймеру)
    void doDemonstration();
    void clearDemostrationTiles(const TilePair &tiles);
    void closeDemonstration();



private slots:
    void slotStartConnect(const TilePair &tiles);

};

#endif // CSCENE_H
