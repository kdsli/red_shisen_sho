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
public:
    explicit CScene(CField *field, QObject *parent = nullptr);

    const QRectF &fieldRect() const { return m_field_rect; }

    // Установить фоновое изображение
    void setBackground(const QString &file_name);

    // Новая игра
    void newGame();

    // Показать/скрыть сообщение
    void showMessage(const QString &message, bool is_hide_tiles = false);
    void hideMessage(bool is_show_tiles);

    // Обработка мыши
    void mouseLeft(QPointF point);
    void mouseRight(QPointF point);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    QPixmap m_bg_pixmap;
    CField *m_field;
    // Rect поля (костяшки + margins)
    QRectF m_field_rect;
    // Отдельные списки для костяшек и основ
    QVector<QGraphicsSvgItem *> m_tiles_list, m_bases_list;
    // Сообщение
    QString m_message;
    // Размеры прямоугольника сообщений
    QRectF m_message_rect;
    // Размер шрифта сообщения в пикселях
    int m_message_font_pixel;
    // Выделенные в текущий момент костяшки
    TileList m_selected;

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
};

#endif // CSCENE_H
