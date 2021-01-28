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
};

static struct note_offset bass[] = {
  {"D", "g3"}
};

int main(int argc, char **argv)
{
char *base = "D";
char *interval = "g3";
double semitones;
double delta;
int root;
int i;

  setlocale(LC_ALL, getenv("LANG"));

  for (i=0; i<1; i++)
  {
    semitones = (double) chromatic_index(bass[i].base)
      - 12.0 * log(sruti_ratio(bass[i].interval))/log(2.0);
    delta = semitones - round(semitones);
    root = round(semitones);
    if (root < 0)
    {
      root += 12;
    }
    wprintf(L"%2ls %3d\n", chromatic_scale_flats[root], (int) round(delta*100.0));
  }
  return 0;

}
