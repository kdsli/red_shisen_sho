#include "ctilesmanager.h"

#include "csettings.h"
#include "errors.h"

#include <QDir>
#include <QSvgRenderer>
#include <QGraphicsSvgItem>
#include <QPainter>

/*
 * CHARACTER_x 1-9 иероглифы
 * ROD_x 1-9 кружочки
 * BAMBOO_x 1-9 палочки, начиная с дракона
 * DRAGON_x 1-3 пусто, зеленый иероглиф, красное Ф
 * FLOWER_x 1-4 цветы без горшков
 * SEASON_x 1-4 цветы в горшках
 * WIND_x 1-4 N, S, E, W
 * TILE_x 1-4 (северо-восток, юго-восток, юго-запад, северо-запад)
 * TILE_x_SEL 1-4 тоже выделено
 * TILE_SHAD_SEL (67x96)
 * TILE_SHAD_NORM
 *
*/

CTilesManager *tiles_manager{nullptr};

static const QString tiles_dir = "tilesets";
static const QSize base_size = QSize(96, 116);
static const QSize tile_size = QSize(69, 89);
static const QStringList filters_svg{"*.svg", "*.svgz"};
static const QString base_name = "TILE_2";
static const QString base_selected_name = "TILE_2_SEL";

// ------------------------------------------------------------------------------------------------
CTilesManager::CTilesManager(QObject *parent) : QObject(parent),
    m_lib_dir(settings->mahjonggLibDir() + tiles_dir + QDir::separator()),
    m_renderer(nullptr)
{
    addTileSeries("CHARACTER", 9);
    addTileSeries("ROD", 9);
    addTileSeries("BAMBOO", 9);
    addTileSeries("WIND", 4);
    addTileSeries("SEASON", isCorrectSVG() ? 4 : 1);
    addTileSeries("DRAGON", 3);
    addTileSeries("FLOWER", isCorrectSVG() ? 4 : 1);

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &CTilesManager::slotDirectoryChanged);
    m_watcher.addPath(m_lib_dir);

    initFiles();

    initCurrentFile();
}

// ------------------------------------------------------------------------------------------------
QString CTilesManager::currentFile()
{
    return m_lib_dir + settings->currentTilesetName();
}

// ------------------------------------------------------------------------------------------------
int CTilesManager::currentIndex()
{
    auto file_name = settings->currentTilesetName();
    auto it = std::find_if(m_files.cbegin(), m_files.cend(), [&file_name](const auto record){
        return record.file_name.compare(file_name, FILE_NAME_SENSITIVITY) == 0;
    });

    return it == m_files.cend() ? -1 : static_cast<int>(std::distance(m_files.cbegin(), it));
}

// ------------------------------------------------------------------------------------------------
// Признак того, что в текущем наборе правильно выводятся SEASON и FLOWER
// Два файла из поставки KDE в этих элементах не содержат изображения, а
// содержать линк на элементы других элементов. Например, на рамку и цифру.
// Qt::svg не умеет их правильно отрисовывать и рисует предыдущий элемент.
// Например, в наборе alphabet четыре типа SEASON_x имеет одинаковое отображение.
// Решением в лоб яляется просто игнорирование типов 2-4 в этих наборах
// Потом надо будет как-нибудь поправить SVG
bool CTilesManager::isCorrectSVG()
{
    auto file_name = settings->currentTilesetName();
    return file_name.compare("alphabet.svgz") != 0
            && file_name.compare("egypt.svgz") != 0;
}

// ------------------------------------------------------------------------------------------------
// Получить размеры костяшки
void CTilesManager::getTileSize(const QString &tile_name, QSizeF &size) const
{
    QGraphicsSvgItem item;
    item.setSharedRenderer(m_renderer);
    item.setElementId(tile_name);
    size.setWidth(item.boundingRect().toRect().width());
    size.setHeight(item.boundingRect().toRect().height());
}

// ------------------------------------------------------------------------------------------------
// Читаем все файлы в директории
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

// ------------------------------------------------------------------------------------------------
// Инициализация текущего файла
void CTilesManager::initCurrentFile()
{
    if (m_renderer) delete m_renderer;
    m_renderer = new QSvgRenderer(currentFile());

    // Базовый размер основы и костяшки
    getTileSize(base_name, m_base_size);
    getTileSize("CHARACTER_2", m_tile_size);

    // И размеры тени
    m_shadow_width = m_base_size.width() - m_tile_size.width();
    m_shadow_height = m_base_size.height() - m_tile_size.height();

    // Толщина линии в 10% от высоты костяшки
    m_path_line_size = static_cast<int>(m_tile_size.height() / 10);
}

// ------------------------------------------------------------------------------------------------
// Названия обычной и выделенной базовой костяшки
const QString &CTilesManager::getBaseName() const
{
    return base_name;
}

const QString &CTilesManager::getSelectedBaseName() const
{
    return base_selected_name;
}

// ------------------------------------------------------------------------------------------------
// Прочитать и сохранить изображение одной костяшки из файла (для окна настроек)
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
    painter_tile.setRenderHint(QPainter::Antialiasing);
    renderer->render(&painter_tile, "CHARACTER_2");
    painter_tile.end();

    // Рисуем base Image
    QImage img(base_size, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer->render(&painter, base_name);

    // И сверху костяшку
    auto tile_rect = QRectF(QPointF(0, 0), tile_size);
    painter.drawImage(tile_rect, img_tile, tile_rect);
    painter.end();

    // И сохраним как зависимый Pixmap
    file.pixmap = QPixmap::fromImage(img);
    m_files.append(std::move(file));
}

// ------------------------------------------------------------------------------------------------
// Слот обработки изменения директории
void CTilesManager::slotDirectoryChanged(const QString &)
{
    initFiles();
    // Сказать окну настроек, что изменилась директория
    emit signalChangeTilesets();
}

// ------------------------------------------------------------------------------------------------
void CTilesManager::addTileSeries(const QString &series_name, int count)
{
    for (int i  = 1; i < count + 1; ++i) {
        m_tiles_names.append(series_name + "_" + QString::number(i));
    }
}
