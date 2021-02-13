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
#include <string.h>

static char *chromatic_scale[] = {
  "C",
  "C#",
  "D",
  "D#",
  "E",
  "F",
  "F#",
  "G",
  "G#",
  "A",
  "A#",
  "B"
};

static int chromatic_index(char *str)
{
int i;

  for (i=0; i<12; i++)
  {
    if (strcmp(str, chromatic_scale[i]) == 0)
      return i;
  }
  fprintf(stderr, "Note not found\n");
  return -1;
}

struct chord {
  char *bass;
  char *note1;
  double ratio1;
  int octave1;
  char *note2;
  double ratio2;
  int octave2;
  char *note3;
  int octave3;
  char *bass_comma;
  int duration;
  int program;
};

#define N_CHORDS 1
struct chord chords[1] = {
  {"A#", "D",  0.0,  0, "A",  0.0, 0, "C", 1, NULL, 4, 2}
};

static double tuning[12] = {
  1.0,      /* C  */
  16.0/15.0,      /* C# */
  9.0/8.0,  /* D  */
  6.0/5.0,  /* D# */
  5.0/4.0,  /* E  */
  4.0/3.0,  /* F  */
  /* 10.0/7.0, */ 25.0/18.0, /* F# */
  3.0/2.0,  /* G  */
  8.0/5.0,  /* G# */
  5.0/3.0,  /* A  */
  9.0/5.0,  /* A# */
  15.0/8.0, /* B  */
};

void define_tuning(int program, int tuning_shift)
{
int rounded;
double semitones;
double fractional;
int fractional_rounded;
int i;

  for (i=0; i<36; i++)
  {
    semitones = (double) 48.0 + 12.0*log(tuning[i % 12])/log(2.0);
    semitones += tuning_shift;
    semitones += 12.0*(i / 12);
    rounded = floor(semitones);
    fractional = semitones - rounded;
    fractional_rounded = (int) (fractional*16384.0);
    printf("# rounded = %d fractional = %lf %d\n", rounded, fractional, fractional_rounded);
    printf("1, 0, System_exclusive, 10, 127, 0, 8, 2, %d, 1, %d, %d, %d, %d\n",
      program, 48 + tuning_shift + i, rounded, fractional_rounded >> 7,
      fractional_rounded & 0x7f);
  }
}

int main(int argc, char **argv)
{
int time;
int i;
int tuning_shift;
int n1;
int n2;
int n3;
int bend1;
double syntonic_comma;
double septimal_diesis;
int bass_comma;
double bpm = 95.0;
double pitch_bend;
int index;

  syntonic_comma = 12.0*log(81.0/80.0)/log(2.0);
  fprintf(stderr, "syntonic comma = %lf\n", syntonic_comma);
  septimal_diesis = 12.0*log(36.0/35.0)/log(2.0);
  fprintf(stderr, "septimal diesis = %lf\n", septimal_diesis);
  /*
   * 25/14 = (9/5)*(125/126)
   * 12/7 = (5/3)*(36/35)  -- E of a G-based scale
   * 10/7 = (25/18)*(36/35) - Db of a G-based scale
   * 9/7 = (5/4)*(36/35)
   * 7/4 = (9/5)*(35/36)
   * 7/6 = (6/5)*(35/36)
   *
   */

  printf("0, 0, Header, 1, 3, 480\n");
  printf("1, 0, Start_track\n");
  printf("1, 0, Time_signature, 4, 2, 24, 8\n");
  printf("1, 0, Tempo, %d\n", (int) (1.0E6*60.0/bpm));
  printf("1, 0, Key_signature, -1, minor\n");
  tuning_shift = 7; /* base the scale on G */
  define_tuning(1, tuning_shift);
  define_tuning(2, 9);
  printf("1, 0, End_track\n");
  printf("2, 0, Start_track\n");
  printf("2, 0, Program_c, 1, 68\n"); /* baritone sax */
  time = 0;
  printf("2, 0, Control_c, %d, 10, 80\n", 1);
  printf("2, 0, Control_c, %d, 101, 0\n", 1);
  printf("2, 0, Control_c, %d, 100, 0\n", 1);
  printf("2, 0, Control_c, %d, 38, 0\n", 1);
  printf("2, 0, Control_c, %d, 6, 2\n", 1);
  printf("2, 0, Control_c, %d, 100, 4 # tuning bank\n", 1);
  printf("2, 0, Control_c, %d, 6, 0\n", 1);
  printf("2, 0, Control_c, %d, 100, 3 # tuning program change\n", 1);
  bass_comma = 0;
  for (i=0; i<N_CHORDS; i++)
  {
    printf("2, %d, Control_c, %d, 6, %d\n", time, 1, chords[i].program);
    n1 = chromatic_index(chords[i].bass) /* + 12*chords[i].octave1 */;
    if (chords[i].bass_comma)
    {
      if (strcmp(chords[i].bass_comma, "7") == 0)
      {
        printf("2, %d, Pitch_bend_c, 1, %d\n", time,
          8192 + (int) round(4096.0 * septimal_diesis));
      }
      else
      {
        printf("2, %d, Pitch_bend_c, 1, %d\n", time,
          8192 + (int) round(4096.0 * syntonic_comma));
      }
      bass_comma = 1;
    }
    else if (bass_comma)
    {
      printf("2, %d, Pitch_bend_c, 1, %d\n", time, 8192);
      bass_comma = 0;
    }
    printf("2, %d, Note_on_c, 1, %d, 120\n", time, n1 + 36);
    time += 240*chords[i].duration;
    printf("2, %d, Note_off_c, 1, %d, 0\n", time, n1 + 36);
  }
  printf("2, %d, End_track\n", time);
  printf("3, 0, Start_track\n");
  printf("3, 0, Program_c, 1, 70\n"); /* English horn */
  printf("3, 0, Program_c, 2, 70\n"); /* English horn */
  printf("3, 0, Program_c, 3, 70\n"); /* English horn */
  printf("3, 0, Program_c, 4, 70\n"); /* English horn */
  printf("3, 0, Control_c, 1, 10, 0\n");
  for (i=2; i<5; i++)
  {
    printf("3, 0, Control_c, %d, 10, 64\n", i);
    printf("3, 0, Control_c, %d, 101, 0\n", i);
    printf("3, 0, Control_c, %d, 100, 0\n", i);
    printf("3, 0, Control_c, %d, 38, 0\n", i);
    printf("3, 0, Control_c, %d, 6, 2\n", i);
    printf("3, 0, Control_c, %d, 100, 4 # tuning bank\n", i);
    printf("3, 0, Control_c, %d, 6, 0\n", i);
    printf("3, 0, Control_c, %d, 100, 3 # tuning program change\n", i);
    printf("3, 0, Control_c, %d, 6, 2\n", i);
  }

  time = 0;
#if 0
  for (i=0; i<12; i++)
  {
    printf("3, %d, Note_on_c, 2, %d, 81\n", time, 48 + tuning_shift + i);
    printf("3, %d, Note_on_c, 3, %d, 81\n", time, 48 + 7 + tuning_shift + i);
    printf("3, %d, Note_on_c, 4, %d, 81\n", time, 48 + 12 + tuning_shift + i);
    time += 480;
    printf("3, %d, Note_off_c, 2, %d, 0\n", time, 48 + tuning_shift + i);
    printf("3, %d, Note_off_c, 3, %d, 0\n", time, 48 + 7 + tuning_shift + i);
    printf("3, %d, Note_off_c, 4, %d, 0\n", time, 48 + 12 + tuning_shift + i);
  }
#endif
  bend1 = 0;
  for (i=0; i<N_CHORDS; i++)
  {
    n1 = chromatic_index(chords[i].note1) + 12*chords[i].octave1;
    n2 = chromatic_index(chords[i].note2) + 12*chords[i].octave2;
    n3 = chromatic_index(chords[i].note3) + 12*chords[i].octave3;
    if (chords[i].ratio1 != 0.0)
    {
      fprintf(stderr, "ratio = %lf (%lf/7)\n", chords[i].ratio1,
        chords[i].ratio1*7.0);
      index = (n1 - tuning_shift);
      if (index < 0)
      {
        index += 12;
      }
      fprintf(stderr, "index = %d\n", index);
      fprintf(stderr, "n1 = %d\n", n1 + 12);
      fprintf(stderr, "tuning[index] = %lf\n", tuning[index]);
      pitch_bend =  12.0*log(chords[i].ratio1)/log(2.0) - 12.0*log(tuning[index])/log(2.0);
      fprintf(stderr, "pitch_bend = %lf\n", pitch_bend);
      printf("3, %d, Pitch_bend_c, 2, %d\n", time,
          8192 + (int) round(4096.0 * pitch_bend));
      bend1 = 1;
    } else if (bend1)
    {
      printf("3, %d, Pitch_bend_c, 2, %d\n", time, 8192);
      bend1 = 0;
    }
    printf("3, %d, Control_c, 2, 6, %d\n", time, chords[i].program);
    printf("3, %d, Control_c, 3, 6, %d\n", time, chords[i].program);
    printf("3, %d, Control_c, 4, 6, %d\n", time, chords[i].program);
    printf("3, %d, Note_on_c, %d, %d, 81\n", time, 2, 60 + n1);
    printf("3, %d, Note_on_c, %d, %d, 81\n", time, 3, 60 + n2);
    printf("3, %d, Note_on_c, %d, %d, 81\n", time, 4, 60 + n3);
    time += 240*chords[i].duration;
    printf("3, %d, Note_off_c, %d, %d, 0\n", time, 2, 60 + n1);
    printf("3, %d, Note_off_c, %d, %d, 0\n", time, 3, 60 + n2);
    printf("3, %d, Note_off_c, %d, %d, 0\n", time, 4, 60 + n3);
  }

  printf("3, %d, End_track\n", time);

  printf("0, 0, End_of_file\n");

  return 0;
}
