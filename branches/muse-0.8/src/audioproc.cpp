/* MuSE - Multiple Streaming Engine
 *
 * DSP processing routines
 * Copyright (C) 2002 Matteo Nastasi aka mop <nastasi@alternativeoutput.it>
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
 
 "$Id$"
 
 */

#include <math.h>
#include <generic.h>
#include <config.h>

#ifdef MOP_RESAMP
#include <stdio.h>
#include <resarr320to441.h>
#include <resarr160to441.h>

#define CLIP16BIT(a) ((a) > 32767L ? 32767L : ((a) < -32767L ? -32767L : (a)))


#endif
#include <jutils.h>

/* TERMINOLOGY
   samples are 32bit if stereo, 16bit if mono (assuming 16bit sound)
   frames = samples * channels;
   bytes = frames * ( sizeof( typeof( frames ) ) );
   in our case the type of frames is IN_DATATYPE configured to be 2 bytes
*/


/* resampling routines
   
   these resample an input pcm buffer to 16bit stereo 44khz
   stereo is treated as interleaved
   *src is the input buffer
   *dest is the the resampled buffer: interleaved stereo 16bit 44khz
   num is number of FRAMES to be mixed
   volume multiplier is also applied during resampling
   
   value returned is number of FRAMES processed
*/

#ifdef MOP_RESAMP
int resample_mono_16(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		     unsigned int num, float volume) {
  /* il .001 per garantire che una qualche approssimazione non faccia
     andare a leggere dati che non sono presenti */
  int tnum = (int)((float)num * (441.0 / 160.001))<<1; 
  int c, cc;
  int disp = 0, modul = 0, oldin = -1, curin;
  long lframe;
  float ka,kb,kc;

  //  fprintf(stderr,"prev %d\n",prev[3]);

  lframe = (long)(prev[2] * volume);
  dest[0] = dest[1] = (IN_DATATYPE)CLIP16BIT(lframe);
  /*    a = y1
	b = 2y2 - (y3 + 3y1) / 2
	c = (y3 + y1) / 2 - y2
	dove:
	y1 = prev[2]    y2 = prev[3]    y3 = src[0]  */
  ka = (float)prev[2];
  kb = (float)(2.0*prev[3]) - ((float)(src[0] + 3 * prev[2]) / 2.0);
  kc = ((float)(src[0] + prev[2]) / 2.0) - (float)prev[3];
    lframe = (long)((ka + (kb + kc * resarr160to441x[1]) *
                   resarr160to441x[1]) * volume);
  dest[2] = dest[3] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ka + (kb + kc * resarr160to441x[2]) *
                   resarr160to441x[2]) * volume);
  dest[4] = dest[5] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ka + (kb + kc * resarr160to441x[3]) *
                   resarr160to441x[3]) * volume);
  dest[6] = dest[7] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ka + (kb + kc * resarr160to441x[4]) *
                   resarr160to441x[4]) * volume);
  dest[8] = dest[9] = (IN_DATATYPE)CLIP16BIT(lframe);

  /*    y1 = prev[3]    y2 = src[0]    y3 = src[1]    */
  ka = (float)prev[3];
  kb = (float)(2.0*src[0]) - ((float)(src[1] + 3 * prev[3]) / 2.0);
  kc = ((float)(src[1] + prev[3]) / 2.0) - (float)src[0];
  lframe = (long)((ka + (kb + kc * resarr160to441x[5]) * 
                   resarr160to441x[5]) * volume);
  dest[10] = dest[11] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ka + (kb + kc * resarr160to441x[6]) * 
                   resarr160to441x[6]) * volume);
  dest[12] = dest[13] = (IN_DATATYPE)CLIP16BIT(lframe);

  for (c = 14 , modul = 0 ; c < tnum ; c+= 2) {
    cc = (c>>1) - modul;
    curin = disp+resarr160to441lc[cc];
    if (curin != oldin) {
      oldin = curin;
      /*    y1 = src[curin]    y2 = src[curin+1]    y3 = src[curin+2]    */
      ka = (float)src[curin];
      kb = (float)(2.0*src[curin+1]) - ((float)(src[curin+2] + 3 * src[curin]) / 2.0);
      kc = ((float)(src[curin+2] + src[curin]) / 2.0) - (float)src[curin+1];
    }
    lframe = (long)((ka + (kb + kc * resarr160to441lx[cc]) * 
                     resarr160to441lx[cc]) * volume);
    dest[c] = dest[c+1] = (IN_DATATYPE)CLIP16BIT(lframe);
    if (cc == 441) {
      modul += 441;
      disp += 160;
    }
  }
  //  fprintf(stderr,"last %d\n",src[(num>>1)]);

  return (tnum);
}

int resample_stereo_16(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		     unsigned int num, float volume) {
  /* il .001 per garantire che una qualche approssimazione non faccia
     andare a leggere dati che non sono presenti */
  int tnum = (int)((float)(num>>1) * (441.0 / 160.001))<<1; 
  int c, cc;
  int disp = 0, modul = 0, oldin = -1, curin;
  long lframe;
  float ka,kb,kc;
  float ha,hb,hc;

  //  fprintf(stderr,"prev %d %d %d %d\n",prev[0],prev[1],prev[2],prev[3]);

  /* CHANNEL 0  -  samples 0,1,2,3,4  -  frames 0,2,4,6,8  */
  /*    a = y1
	b = 2y2 - (y3 + 3y1) / 2
	c = (y3 + y1) / 2 - y2
	dove:
	y1 = prev[0]    y2 = prev[2]    y3 = src[0]  */
  ka = (float)prev[0];
  kb = (float)(2.0*prev[2]) - ((float)(src[0] + 3 * prev[0]) / 2.0);
  kc = ((float)(src[0] + prev[0]) / 2.0) - (float)prev[2];
  
  lframe = (long)(prev[0] * volume);
  dest[0] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ka + (kb + kc * resarr160to441x[1]) *
                   resarr160to441x[1]) * volume);
  dest[2] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ka + (kb + kc * resarr160to441x[2]) *
                   resarr160to441x[2]) * volume);
  dest[4] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ka + (kb + kc * resarr160to441x[3]) *
                   resarr160to441x[3]) * volume);
  dest[6] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ka + (kb + kc * resarr160to441x[4]) *
                   resarr160to441x[4]) * volume);
  dest[8] = (IN_DATATYPE)CLIP16BIT(lframe);

  /* CHANNEL 1  -  samples 0,1,2,3,4  -  frames 1,3,5,7,9  */
  /*  y1 = prev[1]    y2 = prev[3]    y3 = src[1]  */
  ha = (float)prev[1];
  hb = (float)(2.0*prev[3]) - ((float)(src[1] + 3 * prev[1]) / 2.0);
  hc = ((float)(src[1] + prev[1]) / 2.0) - (float)prev[3];
  
  lframe = (long)(prev[1] * volume);
  dest[1] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ha + (hb + hc * resarr160to441x[1]) *
                   resarr160to441x[1]) * volume);
  dest[3] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ha + (hb + hc * resarr160to441x[2]) *
                   resarr160to441x[2]) * volume);
  dest[5] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ha + (hb + hc * resarr160to441x[3]) *
                   resarr160to441x[3]) * volume);
  dest[7] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ha + (hb + hc * resarr160to441x[4]) *
                   resarr160to441x[4]) * volume);
  dest[9] = (IN_DATATYPE)CLIP16BIT(lframe);
  // dest[1] = dest[3] = dest[5] = dest[7] = dest[9] = 0;
  /* CHANNEL 0  -  samples 5,6  -  frames 10,12  */
  /*    y1 = prev[2]    y2 = src[0]    y3 = src[2]    */
  ka = (float)prev[2];
  kb = (float)(2.0*src[0]) - ((float)(src[2] + 3 * prev[2]) / 2.0);
  kc = ((float)(src[2] + prev[2]) / 2.0) - (float)src[0];
  lframe = (long)((ka + (kb + kc * resarr160to441x[5]) * 
                   resarr160to441x[5]) * volume);
  dest[10] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ka + (kb + kc * resarr160to441x[6]) * 
                   resarr160to441x[6]) * volume);
  dest[12] = (IN_DATATYPE)CLIP16BIT(lframe);

  /* CHANNEL 1  -  samples 5,6  -  frames 11,13  */
  /*    y1 = prev[3]    y2 = src[1]    y3 = src[3]    */
  ha = (float)prev[3];
  hb = (float)(2.0*src[1]) - ((float)(src[3] + 3 * prev[3]) / 2.0);
  hc = ((float)(src[3] + prev[3]) / 2.0) - (float)src[1];
  lframe = (long)((ha + (hb + hc * resarr160to441x[5]) * 
                   resarr160to441x[5]) * volume);
  dest[11] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ha + (hb + hc * resarr160to441x[6]) * 
                   resarr160to441x[6]) * volume);
  dest[13] = (IN_DATATYPE)CLIP16BIT(lframe);
  // dest[11] = dest[13] = 0;


  for (c = 14 , modul = 0 ; c < tnum ; c+= 2) {
    cc = (c>>1) - modul;
    curin = disp+(resarr160to441lc[cc] << 1);
    if (curin != oldin) {
      oldin = curin;
      /*    y1 = src[curin]    y2 = src[curin+2]    y3 = src[curin+4]    */
      ka = (float)src[curin];
      kb = (float)(2.0*src[curin+2]) - ((float)(src[curin+4] + 
						3 * src[curin]) / 2.0);
      kc = ((float)(src[curin+4] + src[curin]) / 2.0) - (float)src[curin+2];
      /*    y1 = src[curin+1]  y2 = src[curin+3]    y3 = src[curin+5]    */
      ha = (float)src[curin+1];
      hb = (float)(2.0*src[curin+3]) - ((float)(src[curin+5] + 
						3 * src[curin+1]) / 2.0);
      hc = ((float)(src[curin+5] + src[curin+1]) / 2.0) - (float)src[curin+3];

    }
    lframe = (long)((ka + (kb + kc * resarr160to441lx[cc]) * 
                     resarr160to441lx[cc]) * volume);
    dest[c] = (IN_DATATYPE)CLIP16BIT(lframe);
    lframe = (long)((ha + (hb + hc * resarr160to441lx[cc]) * 
                     resarr160to441lx[cc]) * volume);
    dest[c+1] = (IN_DATATYPE)CLIP16BIT(lframe);
    // dest[c+1] = 0;
    if (cc == 441) {
      modul += 441;
      disp += 320;
    }
  }
  //  fprintf(stderr,"last %d %d  %d %d|",src[num-4],src[num-3],src[num-2],src[num-1]);

  return (tnum);
}
#endif

int resample_mono_22(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		     unsigned int num, float volume) {
#ifdef MOP_RESAMP
  float volmez;
  unsigned int i, ipu;

  volmez = volume / 2.0;
  dest[0] = dest[1] = (IN_DATATYPE)((float)((long)prev[3] + (long)src[0]) * volmez);
  dest[2] = dest[3] = (IN_DATATYPE)((float)src[0] * volume);
  
  for (i = 1 ; i < num ; i++) {
    ipu = i<<2;
    dest[ipu] = dest[ipu+1] = (IN_DATATYPE)((float)((long)src[i-1] + 
                                                    (long)src[i]) * volmez);
    dest[ipu+2] = dest[ipu+3] = (IN_DATATYPE)((float)src[i] * volume);
  }

  return((int)num<<2);
#else
  unsigned int c,cc;
  for(c=0,cc=0;c<num;c++) {
    cc = c<<2;
    dest[cc] = dest[cc+1] = dest[cc+2] = dest[cc+3] =
      (IN_DATATYPE) (src[c]*volume);
  }
  return((int)num<<2);
#endif
}

int resample_stereo_22(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		     unsigned int num, float volume) {
#ifdef MOP_RESAMP
  float volmez;
  unsigned int i, ipu;

  volmez = volume / 2.0;
  dest[0] = (IN_DATATYPE)((float)((long)prev[2] + (long)src[0]) * volmez);
  dest[1] = (IN_DATATYPE)((float)((long)prev[3] + (long)src[1]) * volmez);
  dest[2] = (IN_DATATYPE)((float)src[0] * volume);
  dest[3] = (IN_DATATYPE)((float)src[1] * volume);
  
  for (i = 2 ; i < num ; i+=2) {
    ipu = i<<1;
    dest[ipu]   = (IN_DATATYPE)((float)((long)src[i-2] + 
                                                    (long)src[i]) * volmez);
    dest[ipu+1] = (IN_DATATYPE)((float)((long)src[i-1] + 
                                                    (long)src[i+1]) * volmez);
    dest[ipu+2] = (IN_DATATYPE)((float)src[i] * volume);
    dest[ipu+3] = (IN_DATATYPE)((float)src[i+1] * volume);
  }

  return((int)num<<1);
#else
  unsigned int c,cc;
  for(c=0,cc=0;c<num;c++) {
    cc = c<<1;
    dest[cc] = dest[cc+1] = (IN_DATATYPE) (src[c]*volume);
  }
  return((int)num<<1);
#endif
}


#ifdef MOP_RESAMP
int resample_mono_32(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		     unsigned int num, float volume) {
  /* il .001 per garantire che una qualche approssimazione non faccia
     andare a leggere dati che non sono presenti */
  int tnum = (int)((float)num * (441.0 / 320.001))<<1; 
  int c, cc;
  int disp = 0, modul = 0, oldin = -1, curin;
  long lframe;
  float ka,kb,kc;

  /*    a = y1
	b = 2y2 - (y3 + 3y1) / 2
	c = (y3 + y1) / 2 - y2
	dove:
	y1 = prev[2]    y2 = prev[3]    y3 = src[0]  */
  ka = (float)prev[2];
  kb = (float)(2.0*prev[3]) - ((float)(src[0] + 3 * prev[2]) / 2.0);
  kc = ((float)(src[0] + prev[2]) / 2.0) - (float)prev[3];
  
  lframe = (long)(prev[2] * volume);
  dest[0] = dest[1] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ka + (kb + kc * resarr320to441x[1]) *
                   resarr320to441x[1]) * volume);
  dest[2] = dest[3] = (IN_DATATYPE)CLIP16BIT(lframe);
  lframe = (long)((ka + (kb + kc * resarr320to441x[2]) *
                   resarr320to441x[2]) * volume);
  dest[4] = dest[5] = (IN_DATATYPE)CLIP16BIT(lframe);

  /*    y1 = prev[3]    y2 = src[0]    y3 = src[1]    */
  ka = (float)prev[3];
  kb = (float)(2.0*src[0]) - ((float)(src[1] + 3 * prev[3]) / 2.0);
  kc = ((float)(src[1] + prev[3]) / 2.0) - (float)src[0];
  lframe = (long)((ka + (kb + kc * resarr320to441x[3]) * 
                   resarr320to441x[3]) * volume);
  dest[6] = dest[7] = (IN_DATATYPE)CLIP16BIT(lframe);
  for (c = 8 , modul = 0 ; c < tnum ; c+= 2) {
    cc = (c>>1) - modul;
    curin = disp+resarr320to441lc[cc];
    if (curin != oldin) {
      oldin = curin;
      /*    y1 = src[curin]    y2 = src[curin+1]    y3 = src[curin+2]    */
      ka = (float)src[curin];
      kb = (float)(2.0*src[curin+1]) - ((float)(src[curin+2] + 3 * src[curin]) / 2.0);
      kc = ((float)(src[curin+2] + src[curin]) / 2.0) - (float)src[curin+1];
    }
    lframe = (long)((ka + (kb + kc * resarr320to441lx[cc]) * 
                     resarr320to441lx[cc]) * volume);
    dest[c] = dest[c+1] = (IN_DATATYPE)CLIP16BIT(lframe);
    
    if (cc == 441) {
      modul += 441;
      disp += 320;
    }
  }

  return (tnum);
}
#endif

#ifdef MOP_RESAMP
int resample_stereo_32(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		     unsigned int num, float volume) {
  /* il .001 per garantire che una qualche approssimazione non faccia
     andare a leggere dati che non sono presenti */
  int tnum = (int)((float)(num>>1) * (441.0 / 320.001))<<1; 
  int c, cc;
  int disp = 0, modul = 0, oldin = -1, curin;
  long lframe;
  float ka,kb,kc;
  float ha,hb,hc;

  /* CHANNEL 0  -  samples 0,1,2  -  frames 0,2,4  */
  /*    a = y1
	b = 2y2 - (y3 + 3y1) / 2
	c = (y3 + y1) / 2 - y2
	dove:
	y1 = prev[0]    y2 = prev[2]    y3 = src[0]  */
  ka = (float)prev[0];
  kb = (float)(2.0*prev[2]) - ((float)(src[0] + 3 * prev[0]) / 2.0);
  kc = ((float)(src[0] + prev[0]) / 2.0) - (float)prev[2];
  
  lframe = (long)(prev[0] * volume);
  dest[0] = (IN_DATATYPE)CLIP16BIT(lframe);

  lframe = (long)((ka + (kb + kc * resarr320to441x[1]) *
                   resarr320to441x[1]) * volume);
  dest[2] = (IN_DATATYPE)CLIP16BIT(lframe);

  lframe = (long)((ka + (kb + kc * resarr320to441x[2]) *
                   resarr320to441x[2]) * volume);
  dest[4] = (IN_DATATYPE)CLIP16BIT(lframe);

  /* CHANNEL 1  -  samples 0,1,2  -  frames 1,3,5  */
  /*  y1 = prev[1]    y2 = prev[3]    y3 = src[1]  */
  ha = (float)prev[1];
  hb = (float)(2.0*prev[3]) - ((float)(src[1] + 3 * prev[1]) / 2.0);
  hc = ((float)(src[1] + prev[1]) / 2.0) - (float)prev[3];
  
  lframe = (long)(prev[1] * volume);
  dest[1] = (IN_DATATYPE)CLIP16BIT(lframe);

  lframe = (long)((ha + (hb + hc * resarr320to441x[1]) *
                   resarr320to441x[1]) * volume);
  dest[3] = (IN_DATATYPE)CLIP16BIT(lframe);

  lframe = (long)((ha + (hb + hc * resarr320to441x[2]) *
                   resarr320to441x[2]) * volume);
  dest[5] = (IN_DATATYPE)CLIP16BIT(lframe);

  /* CHANNEL 0  -  samples 3  -  frames 6  */
  /*    y1 = prev[2]    y2 = src[0]    y3 = src[2]    */
  ka = (float)prev[2];
  kb = (float)(2.0*src[0]) - ((float)(src[2] + 3 * prev[2]) / 2.0);
  kc = ((float)(src[2] + prev[2]) / 2.0) - (float)src[0];
  lframe = (long)((ka + (kb + kc * resarr320to441x[3]) * 
                   resarr320to441x[3]) * volume);
  dest[6] = (IN_DATATYPE)CLIP16BIT(lframe);

  /* CHANNEL 1  -  samples 3  -  frames 7  */
  /*    y1 = prev[3]    y2 = src[1]    y3 = src[3]    */
  ha = (float)prev[3];
  hb = (float)(2.0*src[1]) - ((float)(src[3] + 3 * prev[3]) / 2.0);
  hc = ((float)(src[3] + prev[3]) / 2.0) - (float)src[1];
  lframe = (long)((ha + (hb + hc * resarr320to441x[3]) * 
                   resarr320to441x[3]) * volume);
  dest[7] = (IN_DATATYPE)CLIP16BIT(lframe);

  for (c = 8 , modul = 0 ; c < tnum ; c+= 2) {
    cc = (c>>1) - modul;
    curin = disp+(resarr320to441lc[cc] << 1);
    if (curin != oldin) {
      oldin = curin;
      /*    y1 = src[curin]    y2 = src[curin+2]    y3 = src[curin+4]    */
      ka = (float)src[curin];
      kb = (float)(2.0*src[curin+2]) - ((float)(src[curin+4] + 3 * src[curin]) / 2.0);
      kc = ((float)(src[curin+4] + src[curin]) / 2.0) - (float)src[curin+2];
      /*    y1 = src[curin+1]  y2 = src[curin+3]    y3 = src[curin+5]    */
      ha = (float)src[curin+1];
      hb = (float)(2.0*src[curin+3]) - ((float)(src[curin+5] + 3 * src[curin+1]) / 2.0);
      hc = ((float)(src[curin+5] + src[curin+1]) / 2.0) - (float)src[curin+3];
    }
    lframe = (long)((ka + (kb + kc * resarr320to441lx[cc]) * 
                     resarr320to441lx[cc]) * volume);
    dest[c] = (IN_DATATYPE)CLIP16BIT(lframe);
    lframe = (long)((ha + (hb + hc * resarr320to441lx[cc]) * 
                     resarr320to441lx[cc]) * volume);
    dest[c+1] = (IN_DATATYPE)CLIP16BIT(lframe);
    
    if (cc == 441) {
      modul += 441;
      disp += 320 * 2;
    }
  }

  return (tnum);
}
#endif

int resample_mono_44(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		     unsigned int num, float volume) {
  unsigned int c,cc;
  
  for(c=0;c<num;c++) {
    cc = c<<1;    
    dest[cc] = dest[cc+1] = (IN_DATATYPE) (src[c]*volume);
  }
  
  /* return numbero OF FRAMES */
  return(num<<1);

}

int resample_stereo_44(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		     unsigned int num, float volume) {
  unsigned int c;
  for(c=0;c<num;c++)
    dest[c] = (IN_DATATYPE) (src[c]*volume);

  return(num);
}

int mixxx_stereo_44_novol(int *dest, short *chan, int num) {
  for(int c=0;c<num<<1;c++)
    dest[c] = (int) dest[c] + chan[c];  
  return(num);
}
