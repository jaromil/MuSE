#ifndef __XMLRPC_UI_H__
#define __XMLRPC_UI_H__

#include <jmixer.h>
#include <gui.h>

class XMLRPC_UI : public GUI {
  
 public:
  XMLRPC_UI(int argc, char **argv, Stream_mixer *mix);
  ~XMLRPC_UI();
  
  void run();
  /*
  void set_lcd(unsigned int chan, char *lcd);
  void set_pos(unsigned int chan, float pos);
  void set_title(char *txt);
  void set_status(char *txt); 
  */
  void add_playlist(unsigned int ch, char *txt);
  void sel_playlist(unsigned int ch, int row);
};

#endif
