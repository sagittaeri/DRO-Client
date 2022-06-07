#include "draudiotrackmetadata.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QSettings>
#include <QStringList>

#include "aoapplication.h"
#include "utils.h"

static QMap<QString, DRAudiotrackMetadata> s_audiotrack_cache;

void DRAudiotrackMetadata::update_cache()
{
  AOApplication *ao_app = dynamic_cast<AOApplication *>(qApp);
  if (!ao_app)
  {
    qCritical() << "error: AOApplication has not been created";
    return;
  }

  QList<QFileInfo> l_ini_list;
  { // fetch ini files
    const QDir l_dir(ao_app->get_music_folder_path());
    const QList<QFileInfo> l_file_list = l_dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::IgnoreCase);
    for (const QFileInfo &i_file : l_file_list)
    {
      if (i_file.suffix().toLower() != "ini")
        continue;
      l_ini_list.append(i_file);
    }
  }

  QMap<QString, DRAudiotrackMetadata> l_new_audiotrack_cache;
  QMap<QString, QString> l_new_audiotrack_origin;
  for (const QFileInfo &i_ini_file : l_ini_list)
  {
    const QString l_ini_path = i_ini_file.absoluteFilePath();
    QSettings l_settings(l_ini_path, QSettings::IniFormat);
    l_settings.setIniCodec("UTF-8");
    if (l_settings.status() != QSettings::NoError)
    {
      qWarning() << "error: failed to read audiotrack metadata" << l_settings.status() << "file:" << l_ini_path;
      continue;
    }
    qDebug() << "reading audiotrack metadata" << l_ini_path;
    utils::QSettingsKeyFetcher l_fetcher(l_settings);

    const QStringList l_group_list = l_settings.childGroups();
    for (const QString &i_group : l_group_list)
    {
      l_settings.beginGroup(i_group);
      DRAudiotrackMetadata l_audiotrack;
      l_audiotrack.m_title = l_settings.value(l_fetcher.lookup_value("title")).toString();
      l_audiotrack.m_file_name = l_settings.value(l_fetcher.lookup_value("file_name")).toString();
      l_audiotrack.m_play_once = l_settings.value(l_fetcher.lookup_value("play_once")).toBool();
      l_audiotrack.m_loop_start = l_settings.value(l_fetcher.lookup_value("loop_start")).toULongLong();
      l_audiotrack.m_loop_end = l_settings.value(l_fetcher.lookup_value("loop_end")).toULongLong();
      l_settings.endGroup();

      const QString l_track_name = l_audiotrack.m_file_name;
      if (l_track_name.isEmpty())
      {
        qWarning() << "error: empty file name in section" << i_group;
        continue;
      }
      else if (!QFileInfo::exists(ao_app->get_music_path(l_track_name)))
      {
        qWarning() << "error: audiotrack not found" << l_track_name;
        continue;
      }

      const QString l_lower_track_name = l_track_name.toLower();
      if (l_new_audiotrack_origin.contains(l_lower_track_name))
      {
        qWarning() << "warning: replacing track" << l_track_name << "; previously defined in"
                   << l_new_audiotrack_origin[l_lower_track_name];
      }
      l_new_audiotrack_origin.insert(l_lower_track_name, l_ini_path);
      l_new_audiotrack_cache.insert(l_lower_track_name, std::move(l_audiotrack));
    }
  }
  s_audiotrack_cache = std::move(l_new_audiotrack_cache);
}

DRAudiotrackMetadata::DRAudiotrackMetadata()
{}

DRAudiotrackMetadata::DRAudiotrackMetadata(QString p_file_name) : m_file_name(p_file_name)
{
  const QString l_lower_file_name = p_file_name.toLower();
  if (s_audiotrack_cache.contains(l_lower_file_name))
  {
    *this = s_audiotrack_cache[l_lower_file_name];
  }
}

DRAudiotrackMetadata::~DRAudiotrackMetadata()
{}

QString DRAudiotrackMetadata::file_name()
{
  return m_file_name;
}

QString DRAudiotrackMetadata::title()
{
  return m_title.isEmpty() ? m_file_name : m_title;
}

bool DRAudiotrackMetadata::play_once()
{
  return m_play_once;
}

quint64 DRAudiotrackMetadata::loop_start()
{
  return m_loop_start;
}

quint64 DRAudiotrackMetadata::loop_end()
{
  return m_loop_end;
}
