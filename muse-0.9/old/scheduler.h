#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "jmixer.h"
#include "jsync.h"

class Scheduler: public JSyncThread {
 private:
  Stream_mixer *mixer;
  
  FILE *queuefd;
  timespec timelap;
  char nextevent[255];
  char *command;
  bool endqueue;
  char *p, *pp; /* parsing pointers */

 public:
  Scheduler(char *queue_file, Stream_mixer *mix);

  void run();
  ~Scheduler();
};

#endif
