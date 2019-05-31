#include "cbackgroundmanager.h"

#include "csettings.h"

#include <QDir>
#include <QApplication>
#include <QSvgRenderer>
#include <QGraphicsSvgItem>
#include <QPainter>
#include <QFileInfo>
#include <QFileInfo>

CBackgroundManager *bg_manager = nullptr;

static const QString backgrounds_dir = "backgrounds";
static const QSize thumbnail_size = QSize(165, 124); // 4:3
static const QStringList filters_svg{"*.svg", "*.svgz"};
static const QStringList filters_images{"*.jpg", "*.png"};
static const QString user_dir_name = "UserBackgroungs";

CBackgroundManager::CBackgroundManager(QObject *parent) : QObject(parent),
    m_lib_dir(settings->mahjonggLibDir() + backgrounds_dir + QDir::separator()),
    m_user_dir(qApp->applicationDirPath() + QDir::separator() + user_dir_name + QDir::separator())
{

    // Проверим существование пользовательской директории и создадим ее если надо
    QDir dir(m_user_dir);
    if (!dir.exists()) dir.mkdir(m_user_dir);


    connect(&m_lib_watcher, &QFileSystemWatcher::directoryChanged, this, &CBackgroundManager::slotDirectoryChanged);
    m_lib_watcher.addPath(m_lib_dir);
    connect(&m_user_watcher, &QFileSystemWatcher::directoryChanged, this, &CBackgroundManager::slotDirectoryChanged);
    m_lib_watcher.addPath(m_user_dir);

    if (!QFileInfo::exists(currentFile()))
        settings->setDefaultBackground();

    initFiles();
}

QString CBackgroundManager::currentFile()
{
    // Из какой директории брать файл определим по расширению
    auto file_name = settings->bgName();

    if (isCurrentSvg())
        return m_lib_dir + file_name;
    else
        return m_user_dir + file_name;
}

bool CBackgroundManager::isCurrentSvg()
{
    auto ext = QFileInfo(settings->bgName()).suffix();

    return ext.startsWith("svg", FILE_NAME_SENSITIVITY);
}

// Вернуть индекс в m_files текущего файла
int CBackgroundManager::currentIndex()
{
    auto file_name = settings->bgName();
    auto it = std::find_if(m_files.cbegin(), m_files.cend(), [&file_name](const auto record){
        return record.file_name.compare(file_name, FILE_NAME_SENSITIVITY) == 0;
    });

    return it == m_files.cend() ? -1 : static_cast<int>(std::distance(m_files.cbegin(), it));
}

// Читаем и инициализируем все файлы
void CBackgroundManager::initFiles()
{
    m_files.clear();
    m_files.clear();

    QDir lib_dir(m_lib_dir);

    for (const auto &file : lib_dir.entryList(filters_svg, QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase)) {
        loadSvg(m_lib_dir, file);
    }

    QDir user_dir(m_user_dir);

    for (const auto &file : user_dir.entryList(filters_images, QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase)) {
        loadImage(m_user_dir, file);
    }

}

void CBackgroundManager::loadSvg(const QString &dir, const QString &file_name)
{
    BackgroundFile file;
    file.file_name = file_name;

    // SVG item
    auto renderer = new QSvgRenderer(dir + file_name, this);
    auto svg = new QGraphicsSvgItem;
    svg->setSharedRenderer(renderer);

    // Рисуем на Image
    QImage img(thumbnail_size, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&img);
    renderer->render(&painter);

    // И сохраним как зависимый Pixmap
    file.pixmap = QPixmap::fromImage(img);
    m_files.append(std::move(file));

}

void CBackgroundManager::loadImage(const QString &dir, const QString &file_name)
{
    BackgroundFile file;
    file.file_name = file_name;

    QImage image(dir + file_name);

    // Рисуем на Image
    QImage img(thumbnail_size, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&img);
    painter.drawImage(img.rect(), image, image.rect());

    file.pixmap = QPixmap::fromImage(img);
    m_files.append(std::move(file));
}

void CBackgroundManager::slotDirectoryChanged(const QString &)
{
    initFiles();
    // Сказать окну настроек, что изменилась директория
    emit signalChangeBackgrounds();
}

