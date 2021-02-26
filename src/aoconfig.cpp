#include "aoconfig.h"

#include "draudioengine.h"

// qt
#include <QApplication>
#include <QDir>
#include <QPointer>
#include <QSettings>
#include <QVector>

#include <QDebug>

/*!
    We have to suffer through a lot of boilerplate code
    but hey, when has ao2 ever cared?
    Wait, am I using the term wrong? Nice.
*/
class AOConfigPrivate : public QObject
{
  Q_OBJECT

public:
  AOConfigPrivate(QObject *p_parent = nullptr);
  ~AOConfigPrivate();

  // setters
public slots:
  void read_file();
  void save_file();

private:
  void invoke_signal(QString p_method_name, QGenericArgument p_arg1 = QGenericArgument(nullptr));
  void update_favorite_device();

private slots:
  void on_application_state_changed(Qt::ApplicationState p_state);

private:
  friend class AOConfig;

  QSettings cfg;
  // hate me more
  QVector<QObject *> children;

  // data
  bool autosave;
  QString username;
  QString callwords;
  QString theme;
  QString gamemode;
  bool manual_gamemode;
  QString timeofday;
  bool manual_timeofday;
  bool server_alerts;
  bool always_pre;
  int chat_tick_interval;
  int log_max_lines;
  bool log_is_topdown;
  bool log_uses_newline;
  bool log_music;
  bool log_is_recording;

  // audio
  std::optional<QString> favorite_device_driver;
  int master_volume;
  bool mute_background_audio;
  int effect_volume;
  int system_volume;
  int music_volume;
  int blip_volume;
  int blip_rate;
  bool blank_blips;

  // audio sync
  DRAudioEngine *audio_engine = nullptr;
};

AOConfigPrivate::AOConfigPrivate(QObject *p_parent)
    : QObject(p_parent), cfg(QDir::currentPath() + "/base/config.ini", QSettings::IniFormat),
      audio_engine(new DRAudioEngine(this))
{
  Q_ASSERT_X(qApp, "initialization", "QGuiApplication is required");
  connect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)), this,
          SLOT(on_application_state_changed(Qt::ApplicationState)));

  read_file();
}

AOConfigPrivate::~AOConfigPrivate()
{
  if (autosave)
  {
    save_file();
  }
}

void AOConfigPrivate::read_file()
{
  autosave = cfg.value("autosave", true).toBool();
  username = cfg.value("username").toString();
  callwords = cfg.value("callwords").toString();
  server_alerts = cfg.value("server_alerts", true).toBool();

  theme = cfg.value("theme").toString();
  if (theme.trimmed().isEmpty())
    theme = "default";

  gamemode = cfg.value("gamemode").toString();
  manual_gamemode = cfg.value("manual_gamemode", false).toBool();
  timeofday = cfg.value("timeofday", "").toString();
  manual_timeofday = cfg.value("manual_timeofday", false).toBool();
  always_pre = cfg.value("always_pre", true).toBool();
  chat_tick_interval = cfg.value("chat_tick_interval", 60).toInt();
  log_max_lines = cfg.value("chatlog_limit", 200).toInt();
  log_is_topdown = cfg.value("chatlog_scrolldown", true).toBool();
  log_uses_newline = cfg.value("chatlog_newline", false).toBool();
  log_music = cfg.value("music_change_log", true).toBool();
  log_is_recording = cfg.value("enable_logging", true).toBool();

  if (cfg.contains("favorite_device_driver"))
    favorite_device_driver = cfg.value("favorite_device_driver").toString();

  mute_background_audio = cfg.value("mute_background_audio").toBool();
  master_volume = cfg.value("default_master", 50).toInt();
  system_volume = cfg.value("default_system", 50).toInt();
  effect_volume = cfg.value("default_sfx", 50).toInt();
  music_volume = cfg.value("default_music", 50).toInt();
  blip_volume = cfg.value("default_blip", 50).toInt();
  blip_rate = cfg.value("blip_rate", 1000000000).toInt();
  blank_blips = cfg.value("blank_blips").toBool();

  // audio update
  audio_engine->set_volume(master_volume);
  audio_engine->get_family(DRAudio::Family::FSystem)->set_volume(system_volume);
  audio_engine->get_family(DRAudio::Family::FEffect)->set_volume(effect_volume);
  audio_engine->get_family(DRAudio::Family::FMusic)->set_volume(music_volume);
  audio_engine->get_family(DRAudio::Family::FBlip)->set_volume(blip_volume);

  // audio device
  update_favorite_device();
}

void AOConfigPrivate::save_file()
{
  cfg.setValue("autosave", autosave);
  cfg.setValue("username", username);
  cfg.setValue("callwords", callwords);
  cfg.setValue("server_alerts", server_alerts);
  cfg.setValue("theme", theme);
  cfg.setValue("gamemode", gamemode);
  cfg.setValue("manual_gamemode", manual_gamemode);
  cfg.setValue("timeofday", timeofday);
  cfg.setValue("manual_timeofday", manual_timeofday);
  cfg.setValue("always_pre", always_pre);
  cfg.setValue("chat_tick_interval", chat_tick_interval);
  cfg.setValue("chatlog_limit", log_max_lines);
  cfg.setValue("chatlog_scrolldown", log_is_topdown);
  cfg.setValue("chatlog_newline", log_uses_newline);
  cfg.setValue("music_change_log", log_music);
  cfg.setValue("enable_logging", log_is_recording);

  // audio
  if (favorite_device_driver.has_value())
    cfg.setValue("favorite_device_driver", favorite_device_driver.value());

  cfg.setValue("mute_background_audio", mute_background_audio);
  cfg.setValue("default_master", master_volume);
  cfg.setValue("default_sfx", effect_volume);
  cfg.setValue("default_system", system_volume);
  cfg.setValue("default_music", music_volume);
  cfg.setValue("default_blip", blip_volume);
  cfg.setValue("blip_rate", blip_rate);
  cfg.setValue("blank_blips", blank_blips);
  cfg.sync();
}

void AOConfigPrivate::invoke_signal(QString p_method_name, QGenericArgument p_arg1)
{
  for (QObject *i_child : children)
  {
    QMetaObject::invokeMethod(i_child, p_method_name.toStdString().c_str(), p_arg1);
  }
}

void AOConfigPrivate::update_favorite_device()
{
  if (!favorite_device_driver.has_value())
    return;
  audio_engine->set_favorite_device_by_driver(favorite_device_driver.value());
}

void AOConfigPrivate::on_application_state_changed(Qt::ApplicationState p_state)
{
  audio_engine->set_suppressed(mute_background_audio && p_state != Qt::ApplicationActive);
}

// AOConfig ////////////////////////////////////////////////////////////////////

/*!
 * private classes are cool
 */
namespace
{
static QPointer<AOConfigPrivate> d;
}

AOConfig::AOConfig(QObject *p_parent) : QObject(p_parent)
{
  // init if not created yet
  if (d == nullptr)
  {
    Q_ASSERT_X(qApp, "initialization", "QGuiApplication is required");
    d = new AOConfigPrivate(qApp);
  }

  // ao2 is the pinnacle of thread security
  d->children.append(this);
}

AOConfig::~AOConfig()
{
  // totally safe!
  d->children.removeAll(this);
}

QString AOConfig::get_string(QString p_name, QString p_default)
{
  return d->cfg.value(p_name, p_default).toString();
}

bool AOConfig::get_bool(QString p_name, bool p_default)
{
  return d->cfg.value(p_name, p_default).toBool();
}

int AOConfig::get_number(QString p_name, int p_default)
{
  return d->cfg.value(p_name, p_default).toInt();
}

bool AOConfig::autosave()
{
  return d->autosave;
}

QString AOConfig::username()
{
  return d->username;
}

QString AOConfig::callwords()
{
  return d->callwords;
}

QString AOConfig::theme()
{
  return d->theme;
}

QString AOConfig::gamemode()
{
  return d->gamemode;
}

bool AOConfig::manual_gamemode_enabled()
{
  return d->manual_gamemode;
}

QString AOConfig::timeofday()
{
  return d->timeofday;
}

bool AOConfig::manual_timeofday_enabled()
{
  return d->manual_timeofday;
}

bool AOConfig::server_alerts_enabled()
{
  return d->server_alerts;
}

bool AOConfig::always_pre_enabled()
{
  return d->always_pre;
}

int AOConfig::chat_tick_interval()
{
  return d->chat_tick_interval;
}
int AOConfig::log_max_lines()
{
  return d->log_max_lines;
}

bool AOConfig::log_is_topdown_enabled()
{
  return d->log_is_topdown;
}

bool AOConfig::log_uses_newline_enabled()
{
  return d->log_uses_newline;
}

bool AOConfig::log_music_enabled()
{
  return d->log_music;
}

bool AOConfig::log_is_recording_enabled()
{
  return d->log_is_recording;
}

std::optional<QString> AOConfig::favorite_device_driver()
{
  return d->favorite_device_driver;
}

int AOConfig::master_volume()
{
  return d->master_volume;
}

bool AOConfig::mute_background_audio()
{
  return d->mute_background_audio;
}

int AOConfig::system_volume()
{
  return d->system_volume;
}
int AOConfig::effect_volume()
{
  return d->effect_volume;
}

int AOConfig::music_volume()
{
  return d->music_volume;
}

int AOConfig::blip_volume()
{
  return d->blip_volume;
}

int AOConfig::blip_rate()
{
  return d->blip_rate;
}

bool AOConfig::blank_blips_enabled()
{
  return d->blank_blips;
}

void AOConfig::save_file()
{
  d->save_file();
}

void AOConfig::set_autosave(bool p_enabled)
{
  if (d->autosave == p_enabled)
    return;
  d->autosave = p_enabled;
  d->invoke_signal("autosave_changed", Q_ARG(bool, p_enabled));
}

void AOConfig::set_username(QString p_string)
{
  if (d->username == p_string)
    return;
  d->username = p_string;
  d->invoke_signal("username_changed", Q_ARG(QString, p_string));
}

void AOConfig::set_callwords(QString p_string)
{
  if (d->callwords == p_string)
    return;
  d->callwords = p_string;
  d->invoke_signal("callwords_changed", Q_ARG(QString, p_string));
}

void AOConfig::set_server_alerts(bool p_enabled)
{
  if (d->server_alerts == p_enabled)
    return;
  d->server_alerts = p_enabled;
  d->invoke_signal("server_alerts_changed", Q_ARG(bool, p_enabled));
}

void AOConfig::set_theme(QString p_string)
{
  if (d->theme == p_string)
    return;
  d->theme = p_string;
  d->invoke_signal("theme_changed", Q_ARG(QString, p_string));
}

void AOConfig::set_gamemode(QString p_string)
{
  if (d->gamemode == p_string)
    return;
  d->gamemode = p_string;
  d->invoke_signal("gamemode_changed", Q_ARG(QString, p_string));
}

void AOConfig::set_manual_gamemode(bool p_enabled)
{
  if (d->manual_gamemode == p_enabled)
    return;
  d->manual_gamemode = p_enabled;
  d->invoke_signal("manual_gamemode_changed", Q_ARG(bool, p_enabled));
}

void AOConfig::set_timeofday(QString p_string)
{
  if (d->timeofday == p_string)
    return;
  d->timeofday = p_string;
  d->invoke_signal("timeofday_changed", Q_ARG(QString, p_string));
}

void AOConfig::set_manual_timeofday(bool p_enabled)
{
  if (d->manual_timeofday == p_enabled)
    return;
  d->manual_timeofday = p_enabled;
  d->invoke_signal("manual_timeofday_changed", Q_ARG(bool, p_enabled));
}

void AOConfig::set_always_pre(bool p_enabled)
{
  if (d->always_pre == p_enabled)
    return;
  d->always_pre = p_enabled;
  d->invoke_signal("always_pre_changed", Q_ARG(bool, p_enabled));
}

void AOConfig::set_chat_tick_interval(int p_number)
{
  if (d->chat_tick_interval == p_number)
    return;
  d->chat_tick_interval = p_number;
  d->invoke_signal("chat_tick_interval_changed", Q_ARG(int, p_number));
}

void AOConfig::set_log_max_lines(int p_number)
{
  if (d->log_max_lines == p_number)
    return;
  d->log_max_lines = p_number;
  d->invoke_signal("log_max_lines_changed", Q_ARG(int, p_number));
}

void AOConfig::set_log_is_topdown(bool p_enabled)
{
  if (d->log_is_topdown == p_enabled)
    return;
  d->log_is_topdown = p_enabled;
  d->invoke_signal("log_is_topdown_changed", Q_ARG(bool, p_enabled));
}

void AOConfig::set_log_uses_newline(bool p_enabled)
{
  if (d->log_uses_newline == p_enabled)
    return;
  d->log_uses_newline = p_enabled;
  d->invoke_signal("log_uses_newline_changed", Q_ARG(bool, p_enabled));
}

void AOConfig::set_log_music(bool p_enabled)
{
  if (d->log_music == p_enabled)
    return;
  d->log_music = p_enabled;
  d->invoke_signal("log_music_changed", Q_ARG(bool, p_enabled));
}

void AOConfig::set_log_is_recording(bool p_enabled)
{
  if (d->log_is_recording == p_enabled)
    return;
  d->log_is_recording = p_enabled;
  d->invoke_signal("log_is_recording_changed", Q_ARG(bool, p_enabled));
}

void AOConfig::set_master_volume(int p_number)
{
  if (d->master_volume == p_number)
    return;
  d->master_volume = p_number;
  d->audio_engine->set_volume(p_number);
  d->invoke_signal("master_volume_changed", Q_ARG(int, p_number));
}

void AOConfig::set_mute_background_audio(bool p_enabled)
{
  if (d->mute_background_audio == p_enabled)
    return;
  d->mute_background_audio = p_enabled;
  d->invoke_signal("mute_background_audio_changed", Q_ARG(bool, p_enabled));
}

void AOConfig::set_favorite_device_driver(QString p_device_driver)
{
  if (d->favorite_device_driver.has_value() && d->favorite_device_driver.value() == p_device_driver)
    return;
  d->favorite_device_driver = p_device_driver;
  d->update_favorite_device();
  d->invoke_signal("favorite_device_changed", Q_ARG(QString, p_device_driver));
}

void AOConfig::set_system_volume(int p_number)
{
  if (d->system_volume == p_number)
    return;
  d->system_volume = p_number;
  d->audio_engine->get_family(DRAudio::Family::FSystem)->set_volume(p_number);
  d->invoke_signal("system_volume_changed", Q_ARG(int, p_number));
}

void AOConfig::set_effect_volume(int p_number)
{
  if (d->effect_volume == p_number)
    return;
  d->effect_volume = p_number;
  d->audio_engine->get_family(DRAudio::Family::FEffect)->set_volume(p_number);
  d->invoke_signal("effect_volume_changed", Q_ARG(int, p_number));
}

void AOConfig::set_music_volume(int p_number)
{
  if (d->music_volume == p_number)
    return;
  d->music_volume = p_number;
  d->audio_engine->get_family(DRAudio::Family::FMusic)->set_volume(p_number);
  d->invoke_signal("music_volume_changed", Q_ARG(int, p_number));
}

void AOConfig::set_blip_volume(int p_number)
{
  if (d->blip_volume == p_number)
    return;
  d->blip_volume = p_number;
  d->audio_engine->get_family(DRAudio::Family::FBlip)->set_volume(p_number);
  d->invoke_signal("blip_volume_changed", Q_ARG(int, p_number));
}

void AOConfig::set_blip_rate(int p_number)
{
  if (d->blip_rate == p_number)
    return;
  d->blip_rate = p_number;
  d->invoke_signal("blip_rate_changed", Q_ARG(int, p_number));
}

void AOConfig::set_blank_blips(bool p_enabled)
{
  if (d->blank_blips == p_enabled)
    return;
  d->blank_blips = p_enabled;
  d->invoke_signal("blank_blips_changed", Q_ARG(bool, p_enabled));
}

// moc
#include "aoconfig.moc"
