/* commandline VUMETER
 * Copyright (C) 2002 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 *
 * to compile:
 * gcc -o vumeter vumeter.c
 *
 * sucks from stdin and spits in stdout
 *
 * "$Id$"
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CHUNK_SIZE 16192

static short buf[CHUNK_SIZE];

void cafudda() {
  int c,cc, cpeak, readed = 0;
  long peak[8];
  unsigned long vumeter = 0, vubarsize = 0;
  int vupercent = 0;
  char *p,vubar[80];
  
  readed = fread(buf,1,CHUNK_SIZE,stdin);
  if(ferror(stdin))
    perror("[!] error reading from STDIN: ");
  
  peak[0] = 0; peak[1] = 0; peak[2] = 0; peak[3] = 0;
  peak[4] = 0; peak[5] = 0; peak[6] = 0; peak[7] = 0;
  for(cpeak=0;cpeak<8;cpeak++) {
    cc = (CHUNK_SIZE/8)*cpeak;
    for(c=0;c<CHUNK_SIZE/8;c++)
      if (buf[cc+c]>peak[cpeak]) peak[cpeak] = buf[cc+c];
  }
  vumeter = (peak[0]+peak[1]+peak[2]+peak[3]+
	     peak[4]+peak[5]+peak[6]+peak[7])/8;
  vumeter = (vumeter<0)?0:vumeter;
  
  /* MAX_vumeter : width = vumeter : vubarsize
     qualcuno ha un poster di euclide da regalarmi? */
  vubarsize = (70*vumeter)/16384; // (/16384);
  p = vubar; for(c=0;c<vubarsize;c++,p++) *p='#'; *p='\0';    
  vupercent = ((vumeter*100)/16384)+1;
  fprintf(stderr,"\rVOL [%-69s] %3i%%\r",vubar,vupercent);
  fwrite(buf,1,readed,stdout);
}

#include <signal.h>
/* signal handler */
void quitproc (int Sig) {
  cafudda();
  usleep(500000);
  fprintf(stderr,"\n");
  fflush(stdin);
  fflush(stdout);
  fclose(stdout);
  exit(1);
}
volatile sig_atomic_t userbreak;
/* ---------------------- */


/*
int main(int argc, char **argv) {

  // register signal traps 
  if (signal (SIGINT, quitproc) == SIG_ERR) {
    perror ("[!] Couldn't install SIGINT handler: "); exit (0); }
  if (signal (SIGQUIT, quitproc) == SIG_ERR) {
    perror ("[!] Couldn't install SIGQUIT handler: "); exit (0); }
  if (signal (SIGTERM, quitproc) == SIG_ERR) {
    perror ("[!] Couldn't install SIGTERM handler: "); exit (0); }
  
  while(!feof(stdin)) {
    cafudda();
  }

  exit(1);
}
*/
