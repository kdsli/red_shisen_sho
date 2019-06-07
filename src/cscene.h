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

    // Показать/скрыть все плитки
    void setTilesVisible(bool visible);

    // Заполнить список координат пути
    void setPathCoords();

    // Обработка мыши
    void mouseLeft(QPointF point);
    void mouseRight(QPointF point);

signals:
    // Изменился статус программы
    void signalVariantStatus(VariantStatus);
    // Сигнал изменения статусной информации
    void signalUpdateInfo();

public slots:
    void slotUndo();
    void slotRedo();

private:
    QPixmap m_bg_pixmap;
    CField *m_field;
    // Размер поля (только костяшки)
    QSizeF m_tiles_size;
    // Размер поля (только костяшки + тень)
    QRectF m_field_rect;
    // Rect поля (костяшки + тень + margins)
    QRectF m_viewport_rect;
    // Список костяшек
    QVector<QGraphicsSvgItem *> m_tiles_list;
    // Выделенные в текущий момент костяшки
    TileList m_selected;
    // Костяшки, которые нужно будет снять после показа пути
    TilePair m_deleted_tiles;
    // Списки текущего пути в координатах сцены
    CoordList m_path_coords;

    // Прересчитать логические координаты сцены
    void recalcScene();

    // Создать плитку (основа + плитка)
    QGraphicsSvgItem *createTile(int n, QPointF, const QString &);

    // Вернуть левый верхний угол ячейки
    const QPointF getTilePos(const Tile &tile) const;
    const QPointF getTilePos(const int index) const;
    // Вернуть центр ячейки
    const QPointF getTileCenter(const Tile &tile) const;

    // Выделить или не выделить плитку
    void selectTile(int x, int y, bool selected) const;
    void selectTile(const Tile &tile, bool selected) const;

    // Рассчитать костяшку по координатам виджета
    Tile getTileIndex(const QPointF point);

    // Работа со списком выделенных костяшек
    void clearSelected();
    void addSelected(const Tile &tile);

    // Снимаем костяшки
    void removeTiles(const TilePair &tiles);

    // Сдвинуть колонку вниз и вверх
    void columnMoveDown(const Tile &tile);
    void columnMoveUp(const Tile &tile);

    // Шаманство с координатами линий, чтобы не сильно уходили за границу поля
    void correctPoint(QPointF &point) const;

};

#endif // CSCENE_H
