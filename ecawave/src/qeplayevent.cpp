// ------------------------------------------------------------------------
// qeplayevent.cpp: Simple audio-playback using the default output device
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#include <ecasound/eca-debug.h>

#include "qeplayevent.h"

QEPlayEvent::QEPlayEvent(ECA_CONTROLLER* ctrl,
			 const string& input,
			 const string& output,
			 long int start_pos, 
			 long int length) 
  : QEEvent(ctrl) {
  toggle_valid_state(false);
  init();
  ectrl->add_chain("default");
  set_input(input);
  set_input_position(start_pos);
  set_output(output);
  set_output_position(start_pos);
  set_length(length);
  get_default_audio_format(input);
  ectrl->add_default_output();
  toggle_valid_state(true);
}

