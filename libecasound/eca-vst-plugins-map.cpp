// ------------------------------------------------------------------------
// eca-vst-plugins-map: Dynamic register for storing VST1.0/2.0 plugins
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <vector>
#include <string>
#include <algorithm>

#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>

#include "eca-vst-plugins-map.h"

ECA_VST_PLUGINS_MAP::ECA_VST_PLUGINS_MAP(void) {
  DIR *dp;

  //  ECA_RESOURCES ecarc;
  //  string dir_name = ecarc.resource("vst-plugin-directory");
  string dir_name = "/usr/local/lib/vst";

  int n = 1;
  struct stat statbuf;
  dp = opendir(dir_name.c_str());
  if (dp != 0) {
    struct dirent *entry = readdir(dp);
    while(entry != 0) {
      lstat(entry->d_name, &statbuf);
      if (string(entry->d_name).find(".so") != string::npos) {
	object_keyword_map["vst-" + kvu_numtostr(n)] = dir_name + "/" + string(entry->d_name);
	++n;
      }
      entry = readdir(dp);
    }
  }
}

void ECA_VST_PLUGINS_MAP::register_object(const string& keyword,
					  const string PRESET* object) {
  object_map[keyword] = object;
  object_keyword_map[object->name()] = keyword;
}

const map<string,string>& ECA_VST_PLUGINS_MAP::registered_objects(void) const {
  return(object_keyword_map);
}

ECA_OBJECT* ECA_VST_PLUGINS_MAP::object(const string& keyword) const {
  map<string, PRESET*>::const_iterator p = object_map.begin();
  while(p != object_map.end()) {
    if (p->first == keyword) 
      return(dynamic_cast<PRESET*>(p->second));
    ++p;
  }
  PRESET* gp = 0;
  try {
    gp = dynamic_cast<PRESET*>(new GLOBAL_PRESET(keyword));
  }
  catch(...) { gp = 0; }
  return(gp);
}

string ECA_VST_PLUGINS_MAP::object_identifier(const PRESET* object) const {
  assert(object != 0);
  return(object->name());
}
