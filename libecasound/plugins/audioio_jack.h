#ifndef INCLUDED_AUDIOIO_JACK_H
#define INCLUDED_AUDIOIO_JACK_H

#include <string>

#include "audioio-device.h"
#include "sample-specs.h"

class AUDIO_IO_MANAGER;
class AUDIO_IO_JACK_MANAGER;

/**
 * Interface to JACK audio framework.
 *
 * @author Kai Vehmanen
 */
class AUDIO_IO_JACK : public AUDIO_IO_DEVICE {

 public:

  virtual std::string name(void) const { return("JACK interface"); }
  virtual std::string description(void) const { return(name()); }
  virtual std::string parameter_names(void) const;

  virtual int supported_io_modes(void) const { return(io_read | io_write); }
  virtual bool locked_audio_format(void) const { return(true); }

  AUDIO_IO_JACK (void);
  ~AUDIO_IO_JACK(void);
  
  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual void stop(void);
  virtual void start(void);
  virtual void prepare(void);

  virtual SAMPLE_SPECS::sample_pos_t position_in_samples(void) const;

  virtual void set_parameter(int param, string value);
  virtual string get_parameter(int param) const;

  virtual AUDIO_IO_MANAGER* create_object_manager(void) const;

  void set_manager(AUDIO_IO_JACK_MANAGER* mgr, int id);

  AUDIO_IO_JACK* clone(void) const { return new AUDIO_IO_JACK(*this); }
  AUDIO_IO_JACK* new_expr(void) const { return new AUDIO_IO_JACK(); }  

 private:

  AUDIO_IO_JACK_MANAGER* jackmgr_rep;
  int myid_rep;

  std::string secondparam_rep;
  std::string thirdparam_rep;
  SAMPLE_SPECS::sample_pos_t curpos_rep;

 private:

  AUDIO_IO_JACK (const AUDIO_IO_JACK& x) { }
  AUDIO_IO_JACK& operator=(const AUDIO_IO_JACK& x) {  return *this; }
};

extern "C" {
AUDIO_IO* audio_io_descriptor(void);
int audio_io_interface_version(void);
const char* audio_io_keyword(void);
const char* audio_io_keyword_regex(void);
};

#endif