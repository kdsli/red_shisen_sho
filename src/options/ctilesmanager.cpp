#include "ctilesmanager.h"

#include "csettings.h"
#include "errors.h"

#include <QDir>
#include <QSvgRenderer>
#include <QGraphicsSvgItem>
#include <QPainter>

CTilesManager *tiles_manager{nullptr};

static const QString tiles_dir = "tilesets";
static const QSize base_size = QSize(96, 116);
static const QSize tile_size = QSize(69, 89);
static const QStringList filters_svg{"*.svg", "*.svgz"};

CTilesManager::CTilesManager(QObject *parent) : QObject(parent)
{
    m_lib_dir = settings->mahjonggLibDir() + tiles_dir + QDir::separator();

    initFiles();
}

QString CTilesManager::currentFile()
{
    return m_lib_dir + settings->tilesetName();
}

int CTilesManager::currentIndex()
{
    auto file_name = settings->tilesetName();
    auto it = std::find_if(m_files.cbegin(), m_files.cend(), [&file_name](const auto record){
        return record.file_name.compare(file_name, FILE_NAME_SENSITIVITY) == 0;
    });

    return it == m_files.cend() ? -1 : static_cast<int>(std::distance(m_files.cbegin(), it));
}

// Признак того, что в текущем наборе правильно выводятся SEASON и FLOWER
// Два файла из поставки KDE в этих элементах не содержат изображения, а
// содержать линк на элементы других элементов. Например, на рамку и цифру.
// Qt::svg не умеет их правильно отрисовывать и рисует предыдущий элемент.
// Например, в наборе alphabet четыре типа SEASON_x имеет одинаковое отображение.
// Решением в лоб яляется просто игнорирование типов 2-4 в этих наборах
// Потом надо будет как-нибудь поправить SVG
bool CTilesManager::isCorrectSVG()
{
    auto file_name = settings->tilesetName();
    return file_name.compare("alphabet.svgz") != 0
            && file_name.compare("egypt.svgz") != 0;
}

void CTilesManager::initFiles()
{
    m_files.clear();

    QDir lib_dir(m_lib_dir);
    if (!lib_dir.exists()) {
        CriticalError(tr("Отсутствует директория tileset"));
        return;
    }

    for (const auto &file : lib_dir.entryList(filters_svg, QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase)) {
        loadSvg(file);
    }
}

void CTilesManager::loadSvg(const QString &file_name)
{
    TilesFile file;
    file.file_name = file_name;

    // SVG item
    auto renderer = new QSvgRenderer(m_lib_dir + file_name, this);
    auto svg = new QGraphicsSvgItem;
    svg->setSharedRenderer(renderer);

    // Рисуем tile Image
    QImage img_tile(tile_size, QImage::Format_ARGB32_Premultiplied);
    img_tile.fill(0);
    QPainter painter_tile(&img_tile);
    painter_tile.save();
    painter_tile.setRenderHint(QPainter::Antialiasing);
    renderer->render(&painter_tile, "CHARACTER_2");
    painter_tile.restore();

    // Рисуем base Image
    QImage img(base_size, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    QPainter painter(&img);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    renderer->render(&painter, "TILE_2");
    // И сверху костяшку
    auto tile_rect = QRectF(QPointF(0, 0), tile_size);
    painter.drawImage(tile_rect, img_tile, tile_rect);
    painter.restore();

    // И сохраним как зависимый Pixmap
    file.pixmap = QPixmap::fromImage(img);
    m_files.append(std::move(file));
}
