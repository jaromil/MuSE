/* MuSE - Multiple Streaming Engine
 * profile (config file) class
 *
 * this file contains algorithms written by Flavio de Ayra Mender and
 * an object oriented approach to them (and bugfix) by Denis "jaromil" Rojo
 *
 * Copyright (C) 2001 - 2002 Flavio de Ayra Mendes <h4@locked.org>
 * Copyright (C) 2002 - 2003 Denis Rojo <jaromil@dyne.org>
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
 * "$Id$"
 *
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <jutils.h>
#include <profile.h>
#include <config.h>


char *Profile::is_a_section(char *str)
{
  char section_tmp[MAX_SECTION_SIZE];

  //  fprintf(stderr,"CFG::is_a_section(%s)\n",str);

  if(str[0]!='[') return(NULL);
	
  if (!sscanf(str, "[%[^]]]", section_tmp))
    return(NULL);
  
  //  fprintf(stderr,"CFG::YES it is! %s\n",section_tmp);
  /* return it */
  return(strdup(section_tmp));
}

char **Profile::parse_option(char *str) {
  char option_tmp[MAX_OPTION_SIZE];
  char value_tmp[MAX_VALUE_SIZE];

  memset(option_tmp, '\0', MAX_OPTION_SIZE);
  memset(value_tmp, '\0', MAX_VALUE_SIZE);

  /* if its not an option return 0 */
  if (sscanf(str, "%[^= ] = %[^=\n]", option_tmp, value_tmp) != 2)
    return NULL;
  
  /* if dest_opt were passed copy the option name there */
  res[0] = strdup(option_tmp);
  
  /* if dest_val were passed copy the value there */
  res[1] = strdup(value_tmp);

  //  func("parse_option: opt[%s] val[%s]",res[0],res[1]);
  
  /* return true */
  return res;
}

/* returns:
   -1 EOF
   or offset of section */
off_t Profile::find_section(FILE *fp, const char *section) {
  char tmp[MAX_LINE_SIZE], *sect;
  off_t offset = -1;
  off_t bck, pos;

  //  fprintf(stderr,"CFG::find_section: [%s]\n",section);
  
  /* save initial offset */
  bck = ftell(fp);
  rewind(fp);

  while (!feof(fp)) {

    pos = ftell(fp);
    if(!fgets(tmp, MAX_LINE_SIZE,fp)) break;
    
    chomp(tmp);
    //    fprintf(stderr,"find section : \"%s\"\n",tmp);
    
    /* ignore comments */
    if (tmp[0] == '#'
	|| tmp[0] == '\r'
	|| tmp[0] == '\n'
	|| tmp[0]=='\0')
      continue;
        
    /* is a section line */
    sect = is_a_section(tmp);
    if(sect) {
      /* yes, this is our section. return his offset */
      if (!strncmp(sect,section,MAX_SECTION_SIZE)) {
	offset = pos;
	free(sect);
	break;
      } else free(sect);
    }
  }
  
  /* get back to initial offset */
  fseek(fp,bck,SEEK_SET);
  
  return offset;
}

/* returns:
   -1 EOF
   0 no option in current section
   OR offset of the option */
off_t Profile::find_option(FILE *fp, const char *option, char *value, off_t section_offset) {
  char tmp[MAX_LINE_SIZE];
  char **res = NULL;
  off_t offset = -1;
  off_t bck, pos;

  //  func("find_option(%s)",option);

  /* backup file pointer */
  bck = ftell(fp);
  
  /* seek to section */
  fseek(fp, section_offset, SEEK_SET);
  fgets(tmp, MAX_LINE_SIZE, fp); /* goes to the first option line */

  while (!feof(fp)) {

    pos = ftell(fp);
    if(!fgets(tmp, MAX_LINE_SIZE, fp)) break;
    
    chomp(tmp);
    //    fprintf(stderr,"find option : \"%s\"\n",tmp);
    
    if (tmp[0] == '#'
	|| tmp[0] == '\r'
	|| tmp[0] == '\n'
	|| tmp[0] == '\0')
      continue;

    if (tmp[0] == '[') break;/* END OF SECTION - nothing found */
    
    res = parse_option(tmp);
    if(res) {
      if (!strncmp(res[0], option, MAX_OPTION_SIZE)) { /* SUCCESS */
	if(value) strncpy(value,res[1],MAX_VALUE_SIZE);
	offset = pos;
	//	func("find_option() : FOUND at offset %lu\n",offset);
	break;
      }
      //      free(res[0]); free(res[1]);
    }
  }
  
  /* restore backup */
  fseek(fp,bck,SEEK_SET);

  if(res) { free(res[0]); free(res[1]); }

  return offset;
}

int Profile::cfg_check(const char *file, int output) {
  unsigned int line = 0; /* line number */
  unsigned int section = 0; 
  unsigned int err = 0; /* error checking */
  char buff[MAX_LINE_SIZE]; /* line buffer */
  char *sect = NULL;
  char **res = NULL;
  FILE *fd;

  fd = fopen(file,"r");
  if(!fd) {
    error("cfg_check(): can't open %s (%s)",file,strerror(errno));
    return(false); }
  
  
  /* let's work */
  while (fgets(buff, MAX_LINE_SIZE, fd)) {
    /* line counting */
    line++;
    
    chomp(buff);
    
    /* ignore comments and blank lines */
    if (*buff == '#'
	|| *buff == '\r'
	|| *buff == '\n'
	|| *buff == '\0')
      continue;
    
    if ((sect = is_a_section(buff))) { /* ok, its a section */
      section = 1;
      free(sect);
    } else if ((res = parse_option(buff))) {
      if (!section) {
	/* option without a section */
	err = 1;
	if (output)
	  error("CFG::check_config : %s:%d: option '%s' whithout a section.\n",
		file, line, res[0]);
      }
      free(res[0]); free(res[1]);
    } else {
      err = 1;
      if (output)
	error("%s:%d: syntax error: '%s'\n", file, line, buff);
    }
  }
  
  free(res[0]); free(res[1]);
  fclose(fd);
  /* return 0 if failed or 1 if ok */
  return (err ? 0: 1);
}


bool Profile::cfg_read(const char *file, const char *section,
		const char *option, char *value) {
  
  off_t option_offset;
  off_t section_offset;
  FILE *fd;
  
  fd = fopen(file,"r");
  if(!fd) {
    error("cfg_read : can't open %s",file);
    error("%s",strerror(errno));
    return(false); }

  /* return if section not found */
  if ((section_offset = find_section(fd, section)) < 0) {
    func("cfg_read(): can't find section '%s'", section);
    return false;
  }
  
  option_offset = find_option(fd, option, value, section_offset);
  /* return if option not found */
  if(option_offset<1) {
    func("cfg_read(): can't find option[%s] section[%s]", option, section);
    return false;
  }

  fclose(fd);
  return(true);
}

bool Profile::cfg_write(const char *file, const char *section,
		 const char *option, const char *value) {
  off_t section_offset; 
  off_t option_offset;
  char val[MAX_VALUE_SIZE];
  char tmp[MAX_LINE_SIZE];
  FILE *fd, *tmp_fp;
  struct stat st;

  //  func("cfg_write(%s,%s,%s,%s)",file,section,option,value);

  if (stat(file, &st) == -1) {
    /* file is NEW */
    fd = fopen(file,"w");
    if(!fd) {
      error("cfg_write() : can't open %s",file);
      error("%s",strerror(errno));
      return(false); }
    fprintf(fd, "[%s]\n%s = %s\n", section,option,value);
    fflush(fd);
    fclose(fd);
    return(true);
  } else fd = fopen(file,"r+");
  if(!fd) {
    error("cfg_write : can't open %s (%s)",file, strerror(errno));
    return(false); }
  
  section_offset = find_section(fd, section);

  if(section_offset<0) { /* there is no section */
    func("cfg_write() : create new section [%s]",section);

    /* write at the end if section not found */
    fseek(fd, 0, SEEK_END);
    fprintf(fd, "\n[%s]\n%s = %s\n\n", section,option,value);
    //    fflush(fd);
    fclose(fd);
    return(true);
  }

  option_offset = find_option(fd, option, val, section_offset);

  if(option_offset>0) /* option found */
    if(!strncmp(val,value,MAX_VALUE_SIZE)) { /* value is the same */
      fclose(fd);
      return(true);
    }

  /* start a tmp file */
  tmp_fp = tmpfile();
  if (tmp_fp == NULL) {
    error("cfg_write(): can't create temp file (%s)",strerror(errno));
    fclose(fd);
    return(false);
  }

  if(option_offset>0) { /* option exists, change value */

    fseek(fd, option_offset, SEEK_SET);
    fgets(tmp, MAX_LINE_SIZE, fd);
    // func("substituting: %s\n",tmp);
    while(fgets(tmp, MAX_LINE_SIZE, fd))
      fputs(tmp, tmp_fp);
    ftruncate(fileno(fd), option_offset);
    fprintf(fd,"%s = %s\n", option, value);
    rewind(tmp_fp);
    while(fgets(tmp, MAX_LINE_SIZE, tmp_fp))
      fputs(tmp, fd);

  } else { /* option not found, add to section (on top) */

    fseek(fd, section_offset, SEEK_SET);
    fgets(tmp, MAX_LINE_SIZE, fd);
    option_offset = ftell(fd);
    while(fgets(tmp, MAX_LINE_SIZE, fd))
      fputs(tmp, tmp_fp);
    ftruncate(fileno(fd), option_offset);
    fseek(fd,0,SEEK_END);
    fprintf(fd,"%s = %s\n", option, value);
    rewind(tmp_fp);
    while(fgets(tmp, MAX_LINE_SIZE, tmp_fp))
      fputs(tmp, fd);

  }
  
  /* close tmp file (gets deleted automatically) */
  fclose(tmp_fp);
  
  fclose(fd);
  
  return(true);
}

bool Profile::cfg_erase(const char *file, const char *section, const char *option) {
  off_t section_offset;
  off_t option_offset;
  FILE *fd, *tmp_fp;
  char tmp[MAX_LINE_SIZE];
  struct stat st;

  if (stat(file, &st) == -1) {
    error("cfg_erase(): %s does'nt exist",file);
    return(false);
  } else fd = fopen(file,"r+");
  if(!fd) {
    error("cfg_erase(): can't open %s (%s)",file, strerror(errno));
    return(false); }
  
  section_offset = find_section(fd, section);
  if(section_offset < 0) return(0); /* section not found */

  option_offset = find_option(fd, option, NULL, section_offset);
  if(option_offset < 0) return(0); /* option not found */

  /* start a tmp file */
  tmp_fp = tmpfile();
  if (tmp_fp == NULL) {
    error("cfg_erase(): can't create temp file (%s)",strerror(errno));
    fclose(fd);
    return(false);
  }

  /* go one line under the one to be deleted */
  fseek(fd,option_offset,SEEK_SET);
  fgets(tmp,MAX_LINE_SIZE,fd);
  
  /* copy everything after truncate point to tmp file */
  while (!fgets(tmp, MAX_LINE_SIZE, fd))
    fputs(tmp, tmp_fp);
  
  /* truncate config file */
  ftruncate(fileno(fd), option_offset);

  /* restore the backup after truncation from tmp file */
  rewind(tmp_fp);
  while (!fgets(tmp, MAX_LINE_SIZE, tmp_fp))
    fputs(tmp,fd);

  fclose(tmp_fp);
  fclose(fd);

  return(true);
}

int Profile::cfg_get_sections(const char *file, char *dest) {
  
  char tmp[MAX_LINE_SIZE], *sect;
  unsigned int num = 0;
  unsigned int spac = 0;
  FILE *fd;

  fd = fopen(file,"r");
  if(!fd) {
    error("cfg_get_sections(): can't open %s (%s)",file,strerror(errno));
    return(false); }
  
  while (fgets(tmp,MAX_LINE_SIZE,fd)) {

    chomp(tmp);
    /* ignore comments */
    if (tmp[0] == '#'
	|| tmp[0] == '\r'
	|| tmp[0] == '\n'
	|| tmp[0]=='\0')
      continue;
    
    sect = is_a_section(tmp);
    /* copy the section name there */
    if(sect) {
      sprintf(&dest[spac],"%s:",sect);
      num++; spac+=strlen(sect)+1;
      free(sect);
    }
  } 
  return num;
}

Profile::Profile(char *name) {
  char *home = getenv("HOME");
  /* TODO: FIXME */
  if(!home) {
    error("i'm very sorry: you got no $HOME!");
    cfg = NULL; return;
  }
  snprintf(profile_path,MAX_PATH_SIZE,"%s/.muse/%s.muserc",home,name);
  cfg = NULL;
}

Profile::~Profile() { }

/* returns false if a option was missing */
bool Profile::load_profile(const char *section) {
  int ires, *tvar;
  float fres, *fvar;
  bool tres, res = true;
  struct stat st;
  
  if(!cfg) return(false); /* no setup() called */
  if(stat(profile_path, &st) == -1) return(false); /* file not there */

  for(int i=0;cfg[i].name;i++) {
    
    switch(cfg[i].type) {

    case cfgINT:
      tvar = (int*)cfg[i].var;
      ires = read_int(section,cfg[i].name);
      *tvar = (ires<0) ? atoi(cfg[i].defval) : ires;
      func("load_profile(%s) parsed %s value %i",
	   section, cfg[i].name, *(int*)cfg[i].var);
      res &= (ires>=0);
      break;

    case cfgSTR:
      tres = read_str(section,cfg[i].name,(char*)cfg[i].var);
      if(!tres) {
	snprintf((char*)cfg[i].var,MAX_VALUE_SIZE,"%s",cfg[i].defval);
	res = false;
      }
      func("load_profile(%s) parsed %s value %s",
	   section, cfg[i].name,(char*)cfg[i].var);
      break;

    case cfgFLOAT:
      fvar = (float*)cfg[i].var;
      fres = read_float(section,cfg[i].name);
      if(fres==-1.0f) {
	sscanf(cfg[i].defval,"%f",fvar);
	res = false; }
      else *fvar = fres;
      func("load_profile(%s) parsed %s value %.4f",
	   section, cfg[i].name, *(float*)cfg[i].var);
      break;

    case cfgNULL: break;

    }
  }
  return(res);
}

bool Profile::write_profile(const char *section) {
  bool res = true;
  bool err = true;
  if(!cfg) return(false);
  
  for(int i=0;cfg[i].name;i++) {
    switch(cfg[i].type) {

    case cfgINT:
      res = write_int(section,cfg[i].name,*(int*)cfg[i].var );
      if(!res) {
	error("write_profile(%s) : write_int error",section);
	write_str(section,cfg[i].name,cfg[i].defval);
      }
      break;

    case cfgSTR:
      res = write_str(section,cfg[i].name,(const char*)cfg[i].var);
      if(!res) {
	error("write_profile(%s) : write_str error",section);
	write_str(section,cfg[i].name,cfg[i].defval);
      }
      break;

    case cfgFLOAT:
      res = write_float(section, cfg[i].name,*(float*)cfg[i].var );
      if(!res) {
	error("write_profile(%s) : write_float error",section);
	write_str(section,cfg[i].name,cfg[i].defval);
      }
      break;

    case cfgNULL: break;
    }
  }
  return(err);
}

bool Profile::create_default_profile() {
  int i;

  if(!cfg) return(false);

  for(i=0;cfg[i].name;i++) {
    switch(cfg[i].type) {
    case cfgINT:
      sscanf(cfg[i].defval,"%i",(int*)cfg[i].var);
      break;
    case cfgSTR:
      snprintf((char*)cfg[i].var,MAX_VALUE_SIZE,"%s",cfg[i].defval);
      break;
    case cfgFLOAT:
      sscanf(cfg[i].defval,"%f",(float*)cfg[i].var);
      break;
    case cfgNULL: break;
    }
  }
  return write_profile("default");
}

bool Profile::default_profile() {
  /* loads the default */
  if(!load_profile("default")) {
    if(!create_default_profile()) {
      error("can't create default profile %s", profile_path); 
      return false;
    }
  }
  return apply_profile();
}

int Profile::read_int(const char *section, const char *option) {
  char temp[MAX_VALUE_SIZE];
  int res = -1;

  if(!cfg_read(profile_path,section,option,temp)) {
    warning("Profile::read_int(%s,%s) : %s",
	  option,section,profile_path);
    return(-1); }

  res = strtol(temp,NULL,10);
  if(errno==ERANGE) {
    error("Profile::read_int(%s,%s) value %s : ",
	  section, option, temp, strerror(errno));
    return(-1);
  } else return(res);
}

bool Profile::write_int(const char *section, const char *option, const int value) {
  //  func("Profile::write_int(%s,%s,%i)",section,option,value);
  char temp[MAX_VALUE_SIZE];

  snprintf(temp,MAX_VALUE_SIZE,"%i",value);
  if(!cfg_write(profile_path, section, option, temp)) {
    warning("Profile::write_int(%s,%s,%s) : %s",
	  section,option,value,profile_path);
    return(false);
  }
  return(true);
}

float Profile::read_float(const char *section, const char *option) {
  char temp[MAX_VALUE_SIZE];
  float val;

  if(!cfg_read(profile_path, section, option, temp)) {
    warning("Profile::read_float(%s,%s) : %s",
	    option,section,profile_path);
    return(-1.0f); }
  sscanf(temp,"%f",&val);
  return val;
}

bool Profile::write_float(const char *section, const char *option, const float value) {
  char temp[MAX_VALUE_SIZE];
  
  snprintf(temp,MAX_VALUE_SIZE,"%.1f",value);
  if(!cfg_write(profile_path, section, option, temp)) {
    warning("Profile::write_float(%s,%s,%s) : %s",
	    section,option,value,profile_path);
    return false;
  }
  return true;
}

bool Profile::read_str(const char *section, const char *option, char *store) {
  if(!cfg_read(profile_path,section,option,store)) {
    warning("Profile::read_str(%s,%s) : %s",
	  option,section,profile_path);
    return(false); }
  return(true);
}

bool Profile::write_str(const char *section, const char *option, const char *value) {
  //  func("Profile::write_int(%s,%s,%s)",section,option,value);
  if(!cfg_write(profile_path, section, option, value)) {
    warning("Profile::write_str(%s,%s,%s) : %s",
	  section,option,value,profile_path);
    return(false);
  }
  return(true);
}
  
