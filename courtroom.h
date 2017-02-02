#ifndef COURTROOM_H
#define COURTROOM_H

#include "aoimage.h"
#include "aobutton.h"
#include "aocharbutton.h"
#include "aopacket.h"
#include "aoscene.h"
#include "aomovie.h"
#include "aocharmovie.h"
#include "datatypes.h"

#include <QMainWindow>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QVector>
#include <QCloseEvent>
#include <QSignalMapper>
#include <QSoundEffect>

class AOApplication;

class Courtroom : public QMainWindow
{
  Q_OBJECT
public:
  explicit Courtroom(AOApplication *p_ao_app);

  void append_char(char_type p_char){char_list.append(p_char);}
  void append_evidence(evi_type p_evi){evidence_list.append(p_evi);}
  void append_music(QString f_music){music_list.append(f_music);}

  void set_widgets();
  void set_window_title(QString p_title);
  void set_size_and_pos(QWidget *p_widget, QString p_identifier);
  void set_taken(int n_char, bool p_taken);
  void set_char_select_page();
  void set_background(QString p_background);

  //sets desk and bg based on pos in chatmessage
  void set_scene();

  //sets text color based on text color in chatmessage
  void set_text_color();

  //implementations in path_functions.cpp
  QString get_background_path();
  QString get_default_background_path();

  int get_cid() {return m_cid;}
  int get_vp_x(){return m_viewport_x;}
  int get_vp_y(){return m_viewport_y;}
  int get_vp_w(){return m_viewport_width;}
  int get_vp_h(){return m_viewport_height;}


  void enter_courtroom(int p_cid);

  void append_ms_chatmessage(QString f_message);
  void append_server_chatmessage(QString f_message);

  void handle_chatmessage(QStringList *p_contents);
  void handle_chatmessage_2();

  void handle_wtce(QString p_wtce);

  ~Courtroom();

private:
  AOApplication *ao_app;

  int m_courtroom_width = 714;
  int m_courtroom_height = 668;

  int m_viewport_x = 0;
  int m_viewport_y = 0;

  int m_viewport_width = 256;
  int m_viewport_height = 192;

  QVector<char_type> char_list;
  QVector<evi_type> evidence_list;
  QVector<QString> music_list;

  QSignalMapper *char_button_mapper;

  //determines how fast messages tick onto screen
  QTimer *chat_tick_timer;
  int chat_tick_interval = 60;
  //which tick position(character in chat message) we are at
  int tick_pos = 0;

  //delay before chat messages starts ticking
  QTimer *text_delay;

  //delay before sfx plays
  QTimer *sfx_delay;

  static const int chatmessage_size = 15;
  QString m_chatmessage[chatmessage_size];
  bool chatmessage_is_empty = false;

  //state of animation, 0 = objecting, 1 = preanim, 2 = talking, 3 = idle
  int anim_state = 0;

  //state of text ticking, 0 = not yet ticking, 1 = ticking in progress, 2 = ticking done
  int text_state = 0;

  //0 is the first page, 1 second etc.
  //makes char arithmetic easier
  int current_char_page = 0;

  //character id, which index of the char_list the player is
  int m_cid = 0;

  //is set to true if the bg folder contains defensedesk.png, prosecutiondesk.png and stand.png
  bool is_ao2_bg = false;

  //wether the ooc chat is server or master chat, true is server
  bool server_ooc = true;

  QString current_background = "gs4";

  QSoundEffect *sfx_player;

  AOImage *ui_background;

  AOScene *ui_vp_background;
  AOCharMovie *ui_vp_player_char;
  AOScene *ui_vp_desk;
  AOScene *ui_vp_legacy_desk;
  AOImage *ui_vp_chatbox;
  QLabel *ui_vp_showname;
  QPlainTextEdit *ui_vp_message;
  AOImage *ui_vp_testimony;
  AOImage *ui_vp_realization;
  AOMovie *ui_vp_wtce;
  AOMovie *ui_vp_objection;

  QPlainTextEdit *ui_ic_chatlog;

  QPlainTextEdit *ui_ms_chatlog;
  QPlainTextEdit *ui_server_chatlog;

  QListWidget *ui_mute_list;
  QListWidget *ui_area_list;
  QListWidget *ui_music_list;

  QLineEdit *ui_ic_chat_message;

  QLineEdit *ui_ooc_chat_message;
  QLineEdit *ui_ooc_chat_name;

  QLineEdit *ui_area_password;
  QLineEdit *ui_music_search;

  //T0D0: add emote buttons

  AOButton *ui_emote_left;
  AOButton *ui_emote_right;

  AOImage *ui_defense_bar;
  AOImage *ui_prosecution_bar;

  QLabel *ui_music_label;
  QLabel *ui_sfx_label;
  QLabel *ui_blip_label;

  AOButton *ui_hold_it;
  AOButton *ui_objection;
  AOButton *ui_take_that;

  AOButton *ui_ooc_toggle;

  AOButton *ui_witness_testimony;
  AOButton *ui_cross_examination;

  AOButton *ui_change_character;
  AOButton *ui_reload_theme;
  AOButton *ui_call_mod;

  QCheckBox *ui_pre;
  QCheckBox *ui_flip;
  QCheckBox *ui_guard;
\
  AOButton *ui_custom_objection;
  AOButton *ui_realization;
  AOButton *ui_mute;

  AOButton *ui_defense_plus;
  AOButton *ui_defense_minus;

  AOButton *ui_prosecution_plus;
  AOButton *ui_prosecution_minus;

  QComboBox *ui_text_color;

  QSlider *ui_music_slider;
  QSlider *ui_sfx_slider;
  QSlider *ui_blip_slider;

  AOImage *ui_muted;

  //char select stuff under here

  AOImage *ui_char_select_background;

  QVector<AOCharButton*> ui_char_button_list;
  AOImage *ui_selector;

  AOButton *ui_back_to_lobby;

  QLineEdit *ui_char_password;

  AOButton *ui_char_select_left;
  AOButton *ui_char_select_right;

  AOButton *ui_spectator;

public slots:
  void objection_done();

private slots:
  void on_ooc_return_pressed();
  void on_ooc_toggle_clicked();

  void on_witness_testimony_clicked();
  void on_cross_examination_clicked();

  void on_change_character_clicked();
  void on_reload_theme_clicked();
  void on_call_mod_clicked();

  void on_back_to_lobby_clicked();

  void on_char_select_left_clicked();
  void on_char_select_right_clicked();

  void on_spectator_clicked();

  void char_clicked(int n_char);

};

#endif // COURTROOM_H
