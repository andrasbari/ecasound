#ifndef QESESSION_H
#define QESESSION_H

#include <string>
#include <memory>

#include <qwidget.h>

#include <kvutils/definition_by_contract.h>
#include <ecasound/eca-audio-format.h>

class ECA_CONTROLLER;
class ECA_SESSION;

class QEFile;
class QEButtonRow;
class QEStatusBar;
class QEEvent;
class QVBoxLayout;

#include "resources.h"

/**
 * Ecawave session widget
 *
 * Top level user-interface object. This class provides global
 * functions like opening a new file and opening a new session
 * window.
 */
class QESession : public QWidget, public DEFINITION_BY_CONTRACT {
  Q_OBJECT

public slots:

  void new_session(void);
  void new_file(void);
  void open_file(void);
  void save_event(void);
  void save_as_event(void);
  void close_file(void);
  void play_event(void);
  void stop_event(void);
  void effect_event(void);

private slots:

  void position_update(void);
  void selection_update(void); 
  void update_wave_data(void);

 public:

  /**
   * Whether to use wave form cache
   */
  void toggle_wave_cache(bool v) { wcache_toggle_rep = v; }

  /**
   * Whether to force wave form cache refresh
   */
  void toggle_cache_refresh(bool v) { refresh_toggle_rep = v; }

  QESession (const string& filename = "", 
	     const ECA_AUDIO_FORMAT& frm = ECA_AUDIO_FORMAT(), 
	     bool use_wave_cache = true, 
	     bool force_refresh = false,
	     bool direct_mode = false, 
	     QWidget *parent = 0,
	     const char *name = 0);
  ~QESession(void);

 private:

  void init_layout(void);
  void prepare_event(void);
  bool temp_file_created(void);

  string filename_rep;
  string tempfile_rep;

  long int start_pos;
  long int sel_length;

  long int edit_start;
  long int edit_length;

  bool editing_rep;

  QEResources ecawaverc;

  QEFile* file;
  QEButtonRow* buttonrow;
  QEButtonRow* buttonrow2;
  QEEvent* event;

  QVBoxLayout* vlayout;
  QEStatusBar* statusbar;

  bool refresh_toggle_rep, wcache_toggle_rep;
  bool direct_mode_rep;

  auto_ptr<ECA_CONTROLLER> auto_ectrl;
  auto_ptr<ECA_SESSION> auto_esession;

  ECA_CONTROLLER* ectrl;
  ECA_SESSION* esession;
};

#endif








