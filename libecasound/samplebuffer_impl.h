#ifndef INCLUDED_SAMPLEBUFFER_IMPL_H
#define INCLUDED_SAMPLEBUFFER_IMPL_H

#include "samplebuffer.h" 

class SAMPLE_BUFFER_impl {

 public:

  friend class SAMPLE_BUFFER;

 private:

  /** @name Misc member variables */
  /*@{*/

  SAMPLE_BUFFER::sample_type* old_buffer_repp; // for resampling
  std::vector<SAMPLE_BUFFER::sample_type> resample_memory_rep;

  /*@}*/
};

#endif
