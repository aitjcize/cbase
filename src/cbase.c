/**
 * cbase.c - A simple program for base convertion between numbers.
 *
 * Copyright (C) 2010 -  Wei-Ning Huang (AZ) <aitjcize@gmail.com>
 * All Rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <getopt.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define L_IS_TYPE(level, type) ((level & type) == type)
#define L_INFO    0x01
#define L_MSG     0x02
#define L_FATAL   0x04

static struct option longopts[] = {
  { "input-base",                required_argument, NULL, 'i' },
  { "output-base",               required_argument, NULL, 'o' },
  { "term-delimiter",            required_argument, NULL, 'D' },
  { "place-delimiter",           required_argument, NULL, 'd' },
  { "group-term-by",             required_argument, NULL, 'G' },
  { "group-place-by",            required_argument, NULL, 'g' },
  { "delimit-term-with",         required_argument, NULL, 'F' },
  { "delimit-place-with",        required_argument, NULL, 'f' },
  { "suppress-term-delimiter",   no_argument,       NULL, 'S' },
  { "suppress-place-delimiter",  no_argument,       NULL, 's' },
  { "alphabet",                  no_argument,       NULL, 'a' },
  { "version",                   no_argument,       NULL, 'v' },
  { "help",                      no_argument,       NULL, 'h' }
};

typedef enum _mode { DELMITER, GROUP } mode;

/* flags */
static bool supress_term_delimiter = false;
static bool supress_place_delimiter = false;
static bool to_ascii = false;
static bool from_ascii = false;
static bool use_alphabet = false;
static mode place_mode, term_mode;

/* fucntion prototypes */
void term_convert(char* input, int i_base, int o_base,
    const char* place_delimiter, const char* term_delimiter,
    const int place_group_num, const int term_group_num,
    const char* place_forced_delimiter, const char* term_forced_delimiter);

void place_convert(char* input, int i_base, int o_base,
    const char* place_delimiter, const int place_group_num,
    const char* place_forced_delimiter, const int term_group_num);

int count_terms(const char* input, const char* delim);
int cbase_atoi(const char* str);
char* cbase_itoa(int num);
void slog(int level, const char *fmt, ...);
void version(void);
void help(void);

int main(int argc, char *argv[])
{
  char databuf[BUFSIZ + 1];
  char opt = 0;
  int i = 0;
  int input_base = 0;
  int output_base= 0;
  char *term_delimiter = 0;
  char *place_delimiter = 0;
  char *term_forced_delimiter = 0;
  char *place_forced_delimiter = 0;
  int term_group_num = 0;
  int place_group_num = 0;

  while (-1 != (opt = getopt_long(argc, argv, "i:o:D:d:G:g:F:f:Ssavh",
          longopts, NULL))) {
    switch (opt) {
      case 'i':
        if (strcmp(optarg, "a") == 0) {
          input_base = 256;
          from_ascii = true;
        } else
          input_base = atoi(optarg);
        break;
      case 'o':
        if (strcmp(optarg, "a") == 0) {
          output_base = 256;
          to_ascii = true;
        } else
          output_base = atoi(optarg);
        break;
      case 'D':
        term_delimiter = optarg;
        break;
      case 'd':
        place_delimiter = optarg;
        break;
      case 'G':
        term_group_num = atoi(optarg);
        break;
      case 'g':
        place_group_num = atoi(optarg);
        break;
      case 'F':
        term_forced_delimiter = optarg;
        break;
      case 'f':
        place_forced_delimiter = optarg;
        break;
      case 'S':
        supress_term_delimiter = true;
        break;
      case 's':
        supress_place_delimiter = true;
        break;
      case 'a':
        use_alphabet = true;
        break;
      case 'v':
        version();
        exit(0);
      case 'h':
        help();
        exit(0);
      default:
        exit(1);
    }
  }

  if (!input_base)
    slog(L_FATAL, "no input base specified\n");

  if (!output_base)
    slog(L_FATAL, "no output base specified\n");

  if (output_base > 16 && !use_alphabet && !place_forced_delimiter)
    place_forced_delimiter = ".";

  if (term_delimiter && place_delimiter &&
      0 == strcmp(term_delimiter, place_delimiter))
    slog(L_FATAL, "term delimiter should differ from place delimiter\n");

  if (term_delimiter && term_group_num)
    slog(L_FATAL, "conflicted options: `-D' and `-G'\n");

  if (place_delimiter && place_group_num)
    slog(L_FATAL, "conflicted options: `-d' and `-g'\n");

  /* group_num is always 1 if input_base is ascii */
  if (from_ascii) {
    term_group_num = 1;
    term_mode = GROUP;
  }

  if (!place_delimiter && !place_group_num)
    place_group_num = 1;

  if (!term_delimiter && !term_group_num)
    term_delimiter = " ";

  if (!term_forced_delimiter)
    term_forced_delimiter = term_delimiter;

  if (!place_forced_delimiter && output_base > 36 && !use_alphabet)
    place_forced_delimiter = place_delimiter;

  if (term_delimiter)
    term_mode = DELMITER;
  else
    term_mode = GROUP;

  if (place_delimiter)
    place_mode = DELMITER;
  else
    place_mode = GROUP;

  if (optind < argc) { /* read from command line arguments */
    for (i = optind; i < argc; ++i) {
      term_convert(argv[i], input_base, output_base,
                     place_delimiter, term_delimiter,
                     place_group_num, term_group_num,
                     place_forced_delimiter, term_forced_delimiter);
    }
  } else { /* read from stdin */
    while (fgets(databuf, BUFSIZ, stdin)) {
      if (databuf[strlen(databuf) -1] == '\n')
        databuf[strlen(databuf) -1] = 0;
      term_convert(databuf, input_base, output_base,
                     place_delimiter, term_delimiter,
                     place_group_num, term_group_num,
                     place_forced_delimiter, term_forced_delimiter);
    }
  }

  return 0;
}

void term_convert(char* input, int i_base, int o_base,
    const char* place_delimiter, const char* term_delimiter,
    const int place_group_num, const int term_group_num,
    const char* place_forced_delimiter, const char* term_forced_delimiter) {

  int i = 0, term_count = 0;
  char* ptr = 0;
  char** plist = 0;

  if (term_mode == DELMITER) {
    term_count = count_terms(input, term_delimiter);
    plist = (char**) malloc(term_count * sizeof(char*));
    plist[i = 0] = strtok(input, term_delimiter);
    while ((ptr = strtok(NULL, term_delimiter)))
      plist[++i] = ptr;
  } else {
    term_count = strlen(input) / term_group_num;
    if (strlen(input) % term_group_num != 0)
      slog(L_FATAL, "term group count does not match\n");
    plist = (char**) malloc(term_count * sizeof(char*));
    for (i = 0; i < term_count; ++i)
      plist[i] = input + i*term_group_num;
  }

  for (i = 0; i < term_count; ++i) {
    place_convert(plist[i], i_base, o_base, place_delimiter, place_group_num,
        place_forced_delimiter, term_group_num);
    if (i != term_count -1 && term_forced_delimiter &&
        !supress_term_delimiter)
      printf("%s", term_forced_delimiter);
  }
  printf("\n");
  free(plist);
}

void place_convert(char* input, int i_base, int o_base,
    const char* place_delimiter, const int place_group_num,
    const char* place_forced_delimiter, const int term_group_num) {

  int i = 0, place_count = 0;
  char* ptr = 0;
  char* arg = 0;
  char** plist = 0;

  if (term_mode == GROUP) {
    arg = (char*) malloc(term_group_num + 1);
    memcpy(arg, input, term_group_num);
    arg[term_group_num] = 0;
  } else {
    arg = input;
  }

  if (place_mode == DELMITER) {
    place_count = count_terms(input, place_delimiter);
    plist = (char**) malloc(place_count * sizeof(char*));
    plist[i = 0] = strtok(arg, place_delimiter);
    while ((ptr = strtok(NULL, place_delimiter)))
      plist[++i] = ptr;
  } else {
    place_count = strlen(arg) / place_group_num;
    if (strlen(arg) % place_group_num != 0)
      slog(L_FATAL, "place group count does not match\n");
    plist = (char**) malloc(place_count * sizeof(char*));
    for (i = 0; i < place_count; ++i)
      plist[i] = arg + i*place_group_num;
  }

  /* convert to 10-base */
  int source = 0;
  unsigned long long int total = 0, tmp = 0;
  char* carg = 0;
  if (place_mode == GROUP)
    carg = (char*) malloc(place_group_num + 1);

  for (i = 0; i < place_count; ++i) {
    if (place_mode == GROUP) {
      memcpy(carg, plist[i], place_group_num);
      carg[place_group_num] = 0;
    } else {
      carg = plist[i];
    }
    source = cbase_atoi(carg);
    if (source >= i_base)
     slog(L_FATAL, "invalid input base, %s is greater than %d\n", carg,i_base);

    total += source * pow(i_base, place_count - i - 1);
  }

  if (place_mode == GROUP)
    free(carg);
  free(plist);

  /* convert to o_base */
  tmp = total;
  long int digits = 0;
  int r = 0, max_digits = 0;

  if (o_base <= 16 || (use_alphabet && o_base <= 36 ))
    max_digits = 1;
  else
    max_digits = strlen(cbase_itoa(o_base -1));

  while (tmp) {
    r = tmp % o_base;
    tmp /= o_base;
    if (place_forced_delimiter)
      digits += max_digits + strlen(place_forced_delimiter);
    else
      digits += max_digits;
  }
  ++digits;

  char* outstr = (char*) malloc(digits);
  char* buf = (char*) malloc(digits);
  memset(outstr, 0, digits);
  memset(buf, 0, digits);
  tmp = total;

  if (tmp == 0)
    printf("0");

  while (tmp) {
    r = tmp % o_base;
    tmp /= o_base;
    if (to_ascii) {
      snprintf(buf, digits, "%c%s", r, outstr);
      strncpy(outstr, buf, digits);
    } else if (r >= 10 && ((o_base > 10 && o_base <= 16) ||
          (o_base > 16 && o_base <= 36 && use_alphabet))) {
      snprintf(buf, digits, "%c%s", r - 10 + 'a', outstr);
      strncpy(outstr, buf, digits);
    } else {
      snprintf(buf, digits, "%.*d%s", max_digits, r, outstr);
      strncpy(outstr, buf, digits);
    }
    if (tmp != 0 && place_forced_delimiter&& !supress_place_delimiter) {
      snprintf(buf, digits, "%s%s", place_forced_delimiter, outstr);
      strncpy(outstr, buf, digits);
    }
  }
  printf("%s", outstr);

  if (term_mode == GROUP)
    free(arg);

  free(outstr);
  free(buf);
}

int count_terms(const char* input, const char* delim) {
  const char* i = 0;
  int count = 0;
  int delim_len = strlen(delim);
  bool prev_is_delim = (strncmp(input, delim, delim_len) == 0);

  for (i = input; i < input + strlen(input) - strlen(delim) + 1; ++i) {
    if (0 == strncmp(i, delim, delim_len)) {
      if (!prev_is_delim) ++count;
      prev_is_delim = true;
      i += delim_len;
    } else
      prev_is_delim = false;
  }
  if (!prev_is_delim) ++count;
  return count;
}

int cbase_atoi(const char* str) {
  int i;
  int num = atoi(str);

  if (from_ascii)
    return str[0];

  char ck = str[0];
  
  if (num == 0 && strlen(str) == 1) {
    if(ck >= 'a' && ck <= 'z')
      num = ck -'a' +10;
    else if(ck >= 'A' && ck <= 'Z')
      num = ck -'A' +10;
    else if (ck >= '0' && ck <= '9')
      num = ck -'0';
  }

  if (num == 0) {
    for (i = 0; i < strlen(str); ++i)
      if (str[i] != '0')
        slog(L_FATAL, "invalid digit: `%s'\n", str);
  }

  return num;
}

char* cbase_itoa(int num) {
  int i;
  static char cha[64] = "0";
  if (num == 0) {
    strcpy(cha, "0");
    return cha;
  }

  int tmp = num;
  int digits = 0;
  while((int) (num / pow(10, digits)))
    ++digits;
  if(digits > 64)
    slog(L_FATAL ,"cbase: number to large.\n");

  for(i = 0; i < digits; ++i) {
    cha[digits -i -1] = tmp % 10 +'0';
    tmp /= 10;
  }
  cha[digits] = '\0';
  return cha;
}

void slog(int level, const char *fmt, ...) {
  va_list vap;

  if (L_IS_TYPE(level, L_MSG)) {
    va_start(vap, fmt);
    vfprintf(stdout, fmt, vap);
    va_end(vap);
  } else {
    fprintf(stderr, "%s: ", PACKAGE);
    va_start(vap, fmt);
    vfprintf(stderr, fmt, vap);
    va_end(vap);
  }

  if (!L_IS_TYPE(level, L_INFO) && !L_IS_TYPE(level, L_MSG))
    exit(1);
}

void version(void) {
  fprintf(stderr ,"%s\n", PACKAGE_STRING);
  printf("Copyright (C) 2010 Aitjcize (Wei-Ning Huang)\n"
"License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n\n"
"Written by Aitjcize (Wei-Ning Huang).\n");
}

void help(void) {
  printf("Usage: %s [ -i input_base ] [ -o output_base ] [ OPTIONS ] NUM1 NUM2"
         " ...\n\n", PACKAGE);
  printf("Options:\n"
"  -i, --input-base INPUT_BASE\n"
"                        base of input number\n"
"  -o, --output-base OUTPUT_BASE\n"
"                        base of output number\n"
"  -a, --alphabet        use alphabet in places for base between 17 and 36\n"
"  -d, --place-delimiter DELIM\n"
"                        place delimiter in input string\n"
"  -D, --term-delimiter DELIM\n"
"                        term delimiter in input string\n"
"  -g, --group-place-by COUNT\n"
"                        group places by COUNT\n"
"  -G, --group-term-by COUNT\n"
"                        group terms by COUNT\n"
"  -f, --delimit-place-with DELIM\n"
"                        use DELIM as output place delimiter\n"
"  -F, --delimit-term-with DELIM\n"
"                        use DELIM as output term delimiter\n"
"  -s, --suppress-place-delimiter DELIM\n"
"                        suppress output place delimiter\n"
"  -S, --suppress-term-delimiter DELIM\n"
"                        suppress output term delimiter\n"
"  -v, --version         show version information\n"
"  -h, --help            show this help message and exit\n\n"
"Please see manual page for details and examples.\n"
  );
}
