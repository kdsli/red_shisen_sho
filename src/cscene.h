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

    // Выделить или не выделить плитку
    void selectTile(int x, int y, bool selected);

    // Показать/скрыть сообщение
    void showMessage(const QString &message, bool is_hide_tiles = false);
    void hideMessage(bool is_show_tiles);

//    void recalcGlobalCoords();

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

    // Показать/скрыть все плитки
    void setTilesVisible(bool visible);

    void paintMessage(QPainter *painter);
};

#endif // CSCENE_H
