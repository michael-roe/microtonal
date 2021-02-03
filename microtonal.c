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
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <string.h>
#include <math.h>

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

static wchar_t *chromatic_scale_flats[] = {
  L"C",
  L"D\u266D",
  L"D",
  L"E\u266D",
  L"E",
  L"F",
  L"G\u266D",
  L"G",
  L"A\u266D",
  L"A",
  L"B\u266D",
  L"B"
};

static int chromatic_index(char *str)
{
int i;

  for (i=0; i<12; i++)
  {
    if (strcmp(str, chromatic_scale[i]) == 0)
      return i;
  }
  return -1;
}

struct sruti {
  char *name;
  int numerator;
  int denominator;
};

static struct sruti sruti_table[] = {
	{"sa", 1, 1},
	{"r1", 256, 243},
	{"r2", 16, 15},
	{"r3", 10, 9},
	{"r4", 9, 8},
	{"g1", 32, 27},
	{"g2", 6, 5},
	{"g3", 5, 4},
	{"g4", 81, 64},
	{"m1", 4, 3},
	{"m2", 27, 20},
	{"m3", 45, 32},
	{"m4", 64, 45},
	{"pa", 3, 2},
	{"d1", 128, 81},
	{"d2", 8, 5},
	{"d3", 5, 3},
	{"d4", 27, 16},
	{"n1", 16, 9},
	{"n2", 9, 5},
	{"n3", 15, 8},
	{"n4", 243, 128},
	{"7/4", 7, 4},
	{"7/5", 7, 5},
	{"7/6", 7, 6}
};

double sruti_ratio(char *str)
{
int i;

  for (i=0; i<25; i++)
  {
    if (strcmp(str, sruti_table[i].name) == 0)
    {
      return ((double) sruti_table[i].numerator) /
        ((double) sruti_table[i].denominator);
    }
  }
  return 0.0;
}

struct note_offset {
  char *base;
  char *interval;
  int octave;
  char *third;
  int third_octave;
  char *seventh;
  int seventh_octave;
  char *ninth;
  int ninth_octave;
  int duration;
};

static struct note_offset bass[] = {
  {"A", "n3",   0, "g3", 0, "n3",  0, "r4", 1, 4}
};

int main(int argc, char **argv)
{
double semitones;
int base;
double delta;
int root;
int root_modulo;
int i;
int track;
int time;
int play_chords = 1;
double bpm = 95.0;
double third;
int third_rounded;
int third_modulo;
double third_delta;
double seventh;
int seventh_rounded;
int seventh_modulo;
double seventh_delta;

  setlocale(LC_ALL, getenv("LANG"));

  printf("0, 0, Header, 1, 3, 480\n");
  printf("1, 0, Start_track\n");
  printf("1, 0, Time_signature, 4, 2, 24, 8\n");
  printf("1, 0, Tempo, %d\n", (int) (1.0E6*60.0/bpm));
  printf("1, 0, Key_signature, -1, minor\n");
  printf("1, 0, End_track\n");

  for (track=2; track<4; track++)
  {
    printf("%d, 0, Start_track\n", track);
    if (track == 2)
    {
      printf("%d, 0, Program_c, 1, 68\n", track); /* 68 = baritone sax */
      printf("%d, 0, Control_c, 1, 10, 64\n", track); /* pan */
      printf("%d, 0, Control_c, 1, 101, 0\n", track);
      printf("%d, 0, Control_c, 1, 100, 0\n", track); /* RPN = 0 pitch bend sensitivity */
      printf("%d, 0, Control_c, 1, 38, 0\n", track);
      printf("%d, 0, Control_c, 1, 6, 2\n", track); /* data entry */
    }
    else
    {
      printf("%d, 0, Program_c, 2, 70\n", track); /* 70 = English horn */
      printf("%d, 0, Control_c, 2, 10, 64\n", track); /* pan */
      printf("%d, 0, Program_c, 3, 70\n", track); /* 70 = English horn */
      printf("%d, 0, Control_c, 3, 10, 64\n", track); /* pan */
      printf("%d, 0, Control_c, 3, 101, 0\n", track);
      printf("%d, 0, Control_c, 3, 100, 0\n", track); /* RPN = 0 pitch bend sensitivity */
      printf("%d, 0, Control_c, 3, 38, 0\n", track);
      printf("%d, 0, Control_c, 3, 6, 2\n", track); /* data entry */
      printf("%d, 0, Program_c, 4, 70\n", track); /* 70 = English horn */
      printf("%d, 0, Control_c, 4, 10, 64\n", track); /* pan */
    }
  
    time = 0;
  
    for (i=0; i<1; i++)
    {
      base = chromatic_index(bass[i].base);
      semitones = (double) base
        - 12.0 * log(sruti_ratio(bass[i].interval))/log(2.0);
      delta = semitones - round(semitones);
      root = round(semitones);
      root_modulo = root;
      if (root_modulo < 0)
      {
        root_modulo += 12;
      }
      printf("# %s %lf  %d\n", chromatic_scale[root_modulo], delta,
        bass[i].duration);
      printf("# %lf (31TET)\n", 31.0*log(sruti_ratio(bass[i].interval))/log(2.0));
      if (track == 2)
      {
#if 0
        printf("%d, %d, Pitch_bend_c, 1, %d\n", track, time,
          8192 + (int) round(4096.0 * delta));
        printf("%d, %d, Note_on_c, 1, %d, 81\n", track, time,
          48 + root + 12*bass[i].octave);
#endif
      }
      else
      {
        printf("%d, %d, Note_on_c, 2, %d, 81\n", track, time, 60 + base);
	if (bass[i].third)
        {
	  third = semitones + 12.0*log(sruti_ratio(bass[i].third))/log(2.0);
	  third += 12.0*bass[i].third_octave;
	  third_rounded = round(third);
	  third_delta = third - (double) third_rounded;
	  third_modulo = third_rounded;
	  if (third_modulo > 12)
	  {
            third_modulo -= 12;
	  }
	  else if (third_modulo < 0)
	  {
	    third_modulo += 12;
	  }
	  printf("# third = %s %lf (%d, %s)\n", chromatic_scale[third_modulo],
            third_delta, third_rounded, bass[i].third);
          printf("%d, %d, Pitch_bend_c, 3, %d\n", track, time,
            8192 + (int) round(4096.0 * third_delta));
	  printf("%d, %d, Note_on_c, 3, %d, 81\n", track, time,
	    60 + third_rounded);
	}
	if (bass[i].seventh)
        {
	  seventh = semitones + 12.0*log(sruti_ratio(bass[i].seventh))/log(2.0);
	  seventh += 12.0*bass[i].seventh_octave;
	  seventh_rounded = round(seventh);
	  seventh_delta = seventh - (double) seventh_rounded;
	  seventh_modulo = seventh_rounded;
	  if (seventh_modulo > 12)
	  {
            seventh_modulo -= 12;
	  }
	  else if (seventh_modulo < 0)
	  {
	    seventh_modulo += 12;
	  }
	  printf("# seventh = %s %lf (%d, %s)\n", chromatic_scale[seventh_modulo],
            seventh_delta, seventh_rounded, bass[i].seventh);
          printf("%d, %d, Pitch_bend_c, 4, %d\n", track, time,
            8192 + (int) round(4096.0 * seventh_delta));
	  printf("%d, %d, Note_on_c, 4, %d, 81\n", track, time,
	    60 + seventh_rounded);
	}
      }
      time += 240*bass[i].duration;
      if (track == 2)
      {
#if 0
        printf("%d, %d, Note_off_c, 1, %d, 0\n", track, time,
          48 + root + 12*bass[i].octave);
#endif
      }
      else
      {
        printf("%d, %d, Note_off_c, 2, %d, 81\n", track, time, 60 + base);
	if (bass[i].third)
        {
	  printf("%d, %d, Note_off_c, 3, %d, 81\n", track, time,
            60 + third_rounded);
	}
	if (bass[i].seventh)
        {
	  printf("%d, %d, Note_off_c, 4, %d, 81\n", track, time,
            60 + seventh_rounded);
	}
      }
    }
    time += 480;
    printf("%d, %d, End_track\n", track, time);
  }
  printf("0, 0, End_of_file\n");

  return 0;

}
