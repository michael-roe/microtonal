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
  int duration;
};

static struct note_offset bass[] = {
  {"A", "n3", 0, 4}
};

int main(int argc, char **argv)
{
double semitones;
int base;
double delta;
int root;
int root_modulo;
int i;
int time;
int play_chords = 1;

  setlocale(LC_ALL, getenv("LANG"));

  printf("0, 0, Header, 1, 2, 480\n");
  printf("1, 0, Start_track\n");
  printf("1, 0, Time_signature, 4, 2, 24, 8\n");
  printf("1, 0, Tempo, 500000\n");
  printf("1, 0, Key_signature, -1, minor\n");
  printf("1, 0, End_track\n");
  printf("2, 0, Start_track\n");
  printf("2, 0, Program_c, 1, 33\n"); /* 33 = acoustic bass */
  printf("2, 0, Control_c, 1, 10, 64\n"); /* pan */
  printf("2, 0, Control_c, 1, 101, 0\n");
  printf("2, 0, Control_c, 1, 100, 0\n"); /* RPN = 0 pitch bend sensitivity */
  printf("2, 0, Control_c, 1, 38, 0\n");
  printf("2, 0, Control_c, 1, 6, 2\n"); /* data entry */
  if (play_chords)
  {
    printf("2, 0, Program_c, 2, 1\n"); /* 1 = piano */
    printf("2, 0, Control_c, 2, 10, 64\n"); /* pan */
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
    printf("# %s %d\n", chromatic_scale[root_modulo], bass[i].duration);
    printf("2, %d, Pitch_bend_c, 1, %d\n", time,
      8192 + (int) round(4096.0 * delta));
    printf("2, %d, Note_on_c, 1, %d, 81\n", time,
      52 + root + 12*bass[i].octave);
    if (play_chords)
    {
      printf("2, %d, Note_on_c, 2, %d, 81\n", time, 64 + base);
    }
    time += 240*bass[i].duration;
    printf("2, %d, Note_off_c, 1, %d, 0\n", time,
      52 + root + 12*bass[i].octave);
    if (play_chords)
    {
      printf("2, %d, Note_off_c, 2, %d, 81\n", time, 64 + base);
    }
  }
  time += 480;
  printf("2, %d, End_track\n", time);
  printf("0, 0, End_of_file\n");

  return 0;

}
