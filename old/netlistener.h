#ifndef __NETLISTENER_H
#define __NETLISTENER_H

#include <sys/time.h>
#include <arpa/inet.h>
#include "jmixer.h"
#include "jsync.h"

#define BUFFERSIZE 256
#define PASSWD "hackme"
#define HELLO "MuSE daemon v0.2.0\n\n"

class Net_listener: public JSyncThread {

private:
  int num_porta;
  int id_socket;

  bool disconnect;

  char buffer[BUFFERSIZE];
  struct hostent *phost;
  struct timeval timeout;
  struct sockaddr_in nome_socket;
  struct sockaddr_in nome_accept;
  unsigned int dim_nome_accept;
  Stream_mixer *mixer;

public:
  Net_listener(int porta, Stream_mixer *mix);

  void run();
  int esegui_comando(char *buf);  
  void send_playlist();
  int accetta_socket();
  void chiudi_socket();
  void get_connection();
  int getin();
  int getout(const char *buf);

  int id_accept;
  
  ~Net_listener();
};

#endif
