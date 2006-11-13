/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2002 Denis Rojo aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id$
 *
 */

#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <stdio.h>
#include <stdlib.h>
#include <generic.h>
#include <config.h>

#define MAX_SECTION_SIZE 128
#define MAX_OPTION_SIZE 128
#define MAX_VALUE_SIZE 128
#define MAX_LINE_SIZE MAX_PATH_SIZE

enum cfgTYPE { cfgNULL, cfgINT, cfgSTR, cfgFLOAT };

struct settings {
  const char *name;
  void *var;
  enum cfgTYPE type;
  const char *defval;
};

class Profile {

 private:
  char *res[2];
  char *is_a_section(char *str);
  char **parse_option(char *str);
  off_t find_section(FILE *fp, const char *section);
  off_t find_option(FILE *fp, const char *option, char *value, off_t section_offset);

 protected:
  int cfg_check(const char *file, int output);
  bool cfg_read(const char *file, const char *section, const char *option, char *value);
  bool cfg_write(const char *file, const char *section, const char *option, const char *value);
  bool cfg_erase(const char *file, const char *section, const char *option);
  int cfg_get_sections(const char *file, char *dest);

  char profile_path[MAX_PATH_SIZE];  

  void setup(struct settings *conf) { cfg = conf; }

 public:
  Profile(char *name);
  virtual ~Profile();
  
  /* HERE IS THE PROFILE API
     if you're doing a GUI, those are the functions */
  bool create_default_profile(); /* creates a default profile 
				    and sets default values */
  bool load_profile(const char *section);
  bool write_profile(const char *section);
  virtual bool apply_profile() =0;
    
  /* =-----= */
  int list_profiles(char *str) {
    return cfg_get_sections(profile_path,str); }

  /* loads default settings from the conf file
     if the file is not existing calls create_profile()
     then calls apply_profile() */
  bool default_profile();

  int read_int(const char *section, const char *option);
  bool write_int(const char *section, const char *option, const int value);
  bool read_str(const char *section, const char *option, char *store);
  bool write_str(const char *section, const char *option, const char *value);
  float read_float(const char *section, const char *option);
  bool write_float(const char *section, const char *option, const float value);

  struct settings *cfg;
};

#endif
