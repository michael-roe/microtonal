/*-
 * Copyright (c) 2021 Michael Roe
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <math.h>

double tuning[12] = {
  1.0,      /* C  */
  16.0/15.0,      /* C# */
  9.0/8.0,  /* D  */
  6.0/5.0,  /* D# */
  5.0/4.0,  /* E  */
  4.0/3.0,  /* F  */
  45.0/32.0,/* F# */
  3.0/2.0,  /* G  */
  8.0/5.0,  /* G# */
  5.0/3.0,  /* A  */
  9.0/5.0,  /* A# */
  15.0/8.0, /* B  */
};

int main(int argc, char **argv)
{
int time;
int i;
int rounded;
double semitones;
double fractional;
int fractional_rounded;

  printf("0, 0, Header, 1, 2, 480\n");
  printf("1, 0, Start_track\n");
  printf("1, 0, Time_signature, 4, 2, 24, 8\n");
  printf("1, 0, Tempo, 500000\n");
  printf("1, 0, Key_signature, -1, minor\n");
  for (i=0; i<24; i++)
  {
    semitones = (double) 48.0 + 12.0*log(tuning[i % 12])/log(2.0);
    semitones += 12.0*(i / 12);
    rounded = floor(semitones);
    fractional = semitones - rounded;
    fractional_rounded = (int) (fractional*16384.0);
    printf("# rounded = %d fractional = %lf %d\n", rounded, fractional, fractional_rounded);
    printf("1, 0, System_exclusive, 10, 127, 0, 8, 2, 1, 1, %d, %d, %d, %d\n",
      48 + i, rounded, fractional_rounded >> 7, fractional_rounded & 0x7f);
  }
  printf("1, 0, End_track\n");
  printf("2, 0, Start_track\n");
  printf("2, 0, Program_c, 1, 1\n");
  printf("2, 0, Program_c, 2, 1\n");
  printf("2, 0, Control_c, 1, 10, 0\n");
  printf("2, 0, Control_c, 2, 10, 127\n");
  printf("2, 0, Control_c, 2, 101, 0\n");
  printf("2, 0, Control_c, 2, 100, 0\n");
  printf("2, 0, Control_c, 2, 38, 0\n");
  printf("2, 0, Control_c, 2, 6, 2\n");
  printf("2, 0, Control_c, 2, 100, 4 # tuning bank\n");
  printf("2, 0, Control_c, 2, 6, 0\n");
  printf("2, 0, Control_c, 2, 100, 3 # tuning program change\n");
  printf("2, 0, Control_c, 2, 6, 1\n");

  time = 0;
  for (i=0; i<24; i++)
  {
    printf("2, %d, Note_on_c, 2, %d, 81\n", time, 48 + i);
    time += 480;
    printf("2, %d, Note_off_c, 2, %d, 81\n", time, 48 + i);
  }

  printf("2, %d, End_track\n", time);
  printf("0, 0, End_of_file\n");

  return 0;
}
