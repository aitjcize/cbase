/* cbase.c
 * - change base of numbers

 * Copyright (C) 2009 -  Aitjcize <aitjcize@gmail.com>
 * All Rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.  
 */

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

const char* program_name = "cbase";
const char* program_version = "0.2";

static struct option longopts[] = {
  { "input-base",                required_argument, NULL, 'i' },
  { "ouput-base",                required_argument, NULL, 'o' },
  { "outter-delimeter",          required_argument, NULL, 'D' },
  { "inner-delimeter",           required_argument, NULL, 'd' },
  { "group-outter-by",           required_argument, NULL, 'G' },
  { "group-inner-by",            required_argument, NULL, 'g' },
  { "delimit-outter",            required_argument, NULL, 'F' },
  { "delimit-inner",             required_argument, NULL, 'f' },
  { "supress-outter-delimeter",  no_argument,       NULL, 'S' },
  { "supress-inner-delimeter",   no_argument,       NULL, 's' },
  { "version",                   no_argument,       NULL, 'v' },
  { "help",                      no_argument,       NULL, 'h' }
};

/* flags */
bool supress_outter_delimeter = false;
bool supress_inner_delimeter = false;
bool to_ascii = false;
bool from_ascii = false;
typedef enum _mode { DELMITER, GROUP } mode;
mode inner_mode, outter_mode;

void outter_convert(char* input, int i_base, int o_base,
    const char* inner_delimiter, const char* outter_delimiter,
    const int inner_group_num, const int outter_group_num,
    const char* inner_forced_delimiter, const char* outter_forced_delimiter);

void inner_convert(char* input, int i_base, int o_base,
    const char* inner_delimiter, const int inner_group_num,
    const char* inner_forced_delimiter, const int outter_group_num);

int cbase_atoi(const char* str);
char* cbase_itoa(int num);

void slog(int level, const char *fmt, ...);
void version(void);
void help(void);

int main(int argc, char *argv[])
{
  char opt = 0;
  int i = 0;
  int input_base = 0;
  int output_base= 0;
  char *outter_delimiter = 0;
  char *inner_delimiter = 0;
  char *outter_forced_delimiter = 0;
  char *inner_forced_delimiter = 0;
  int outter_group_num = 0;
  int inner_group_num = 0;

  while (-1 != (opt = getopt_long(argc, argv, "i:o:D:d:G:g:F:f:Ssvh",
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
        outter_delimiter = optarg;
        break;
      case 'd':
        inner_delimiter = optarg;
        break;
      case 'G':
        outter_group_num = atoi(optarg);
        break;
      case 'g':
        inner_group_num = atoi(optarg);
        break;
      case 'F':
        outter_forced_delimiter = optarg;
        break;
      case 'f':
        inner_forced_delimiter = optarg;
        break;
      case 'S':
        supress_outter_delimeter = true;
        break;
      case 's':
        supress_inner_delimeter = true;
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

  if (output_base > 36 && !inner_forced_delimiter)
    inner_forced_delimiter = ".";

  if (outter_delimiter && outter_group_num)
    slog(L_FATAL, "conflicted options: `-D' and `-G'\n");

  if (inner_delimiter && inner_group_num)
    slog(L_FATAL, "conflicted options: `-d' and `-g'\n");

  /* group_num is always 1 if input_base is ascii */
  if (from_ascii) {
    outter_group_num = 1;
    outter_mode = GROUP;
  }

  if (!inner_delimiter && !inner_group_num)
    inner_group_num = 1;

  if (!outter_delimiter && !outter_group_num)
    outter_delimiter = " ";

  if (!outter_forced_delimiter)
    outter_forced_delimiter = outter_delimiter;

  if (!inner_forced_delimiter && output_base > 36)
    inner_forced_delimiter = inner_delimiter;

  if (outter_delimiter)
    outter_mode = DELMITER;
  else
    outter_mode = GROUP;

  if (inner_delimiter)
    inner_mode = DELMITER;
  else
    inner_mode = GROUP;

  for (i = optind; i < argc; i++) {
    outter_convert(argv[i], input_base, output_base,
                   inner_delimiter, outter_delimiter,
                   inner_group_num, outter_group_num,
                   inner_forced_delimiter, outter_forced_delimiter);
  }

  return 0;
}

void outter_convert(char* input, int i_base, int o_base,
    const char* inner_delimiter, const char* outter_delimiter,
    const int inner_group_num, const int outter_group_num,
    const char* inner_forced_delimiter, const char* outter_forced_delimiter) {

  int i = 0, outter_count = 0;
  /* extract */
  char* ptr = 0;
  char** plist = 0;

  if (outter_mode == DELMITER) {
    for (i = 0; i < strlen(input); i++)
      if (input[i] == outter_delimiter[0])
        ++outter_count;
    ++outter_count;
    plist = (char**) malloc(outter_count * sizeof(char*));
    plist[i = 0] = strtok(input, outter_delimiter);
    while ((ptr = strtok(NULL, outter_delimiter)))
      plist[++i] = ptr;
  } else {
    outter_count = strlen(input) / outter_group_num;
    if (strlen(input) % outter_group_num != 0)
      slog(L_FATAL, "outter group count does not match\n");
    plist = (char**) malloc(outter_count * sizeof(char*));
    for (i = 0; i < outter_count; i++)
      plist[i] = input + i*outter_group_num;
  }

  for (i = 0; i < outter_count; i++) {
    inner_convert(plist[i], i_base, o_base, inner_delimiter, inner_group_num,
        inner_forced_delimiter, outter_group_num);
    if (i != outter_count -1 && outter_forced_delimiter &&
        !supress_outter_delimeter)
      printf("%s", outter_forced_delimiter);
  }
  printf("\n");
  free(plist);
}

void inner_convert(char* input, int i_base, int o_base,
    const char* inner_delimiter, const int inner_group_num,
    const char* inner_forced_delimiter, const int outter_group_num) {

  int i = 0, inner_count = 0;
  char* ptr = 0;
  char* arg = 0;
  char** plist = 0;

  if (outter_mode == GROUP) {
    arg = (char*) malloc(outter_group_num + 1);
    memcpy(arg, input, outter_group_num);
    arg[outter_group_num] = 0;
  } else {
    arg = input;
  }

  if (inner_mode == DELMITER) {
    for (i = 0; i < strlen(arg); i++)
      if (arg[i] == inner_delimiter[0])
        ++inner_count;
    ++inner_count;
    plist = (char**) malloc(inner_count * sizeof(char*));
    plist[i = 0] = strtok(arg, inner_delimiter);
    while ((ptr = strtok(NULL, inner_delimiter)))
      plist[++i] = ptr;
  } else {
    inner_count = strlen(arg) / inner_group_num;
    if (strlen(arg) % inner_group_num != 0)
      slog(L_FATAL, "inner group count does not match\n");
    plist = (char**) malloc(inner_count * sizeof(char*));
    for (i = 0; i < inner_count; i++)
      plist[i] = arg + i*inner_group_num;
  }

  /* convert to 10-base */
  int source = 0;
  unsigned long long int total = 0, tmp = 0;
  char* carg = 0;
  if (inner_mode == GROUP)
    carg = (char*) malloc(inner_group_num + 1);

  for (i = 0; i < inner_count; i++) {
    if (inner_mode == GROUP) {
      memcpy(carg, plist[i], inner_group_num);
      carg[inner_group_num] = 0;
    } else {
      carg = plist[i];
    }
    source = cbase_atoi(carg);
    if (source >= i_base)
     slog(L_FATAL, "invalid input base, %s is greater than %d\n", carg,i_base);

    total += source * pow(i_base, inner_count - i - 1);
  }

  if (inner_mode == GROUP)
    free(carg);
  free(plist);

  /* convert to o_base */
  tmp = total;
  long int digits = 0;
  int r = 0, max_digits = 0;

  if (o_base <= 36)
    max_digits = 1;
  else
    max_digits = strlen(cbase_itoa(o_base -1));

  while (tmp) {
    r = tmp % o_base;
    tmp /= o_base;
    if (inner_forced_delimiter)
      digits += max_digits + strlen(inner_forced_delimiter);
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
    } else if (o_base > 10 && o_base <= 36 && r >= 10) {
      snprintf(buf, digits, "%c%s", r - 10 + 'a', outstr);
      strncpy(outstr, buf, digits);
    } else {
      snprintf(buf, digits, "%.*d%s", max_digits, r, outstr);
      strncpy(outstr, buf, digits);
    }
    if (tmp != 0 && inner_forced_delimiter&& !supress_inner_delimeter) {
      snprintf(buf, digits, "%s%s", inner_forced_delimiter, outstr);
      strncpy(outstr, buf, digits);
    }
  }
  printf("%s", outstr);
  free(outstr);
  free(buf);
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
    for (i = 0; i < strlen(str); i++)
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
    digits++;
  if(digits > 64)
    slog(L_FATAL ,"cbase: number to large.\n");

  for(i = 0; i < digits; i++) {
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
    fprintf(stderr, "%s: ", program_name);
    va_start(vap, fmt);
    vfprintf(stderr, fmt, vap);
    va_end(vap);
  }

  if (!L_IS_TYPE(level, L_INFO) && !L_IS_TYPE(level, L_MSG))
    exit(1);
}

void version(void) {
  fprintf(stderr ,"%s Ver. %s\n", program_name, program_version);
  printf("Copyright (C) 2010 Aitjcize (Wei-Ning Huang)\n"
"License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n\n"
"Written by Aitjcize (Wei-Ning Huang).\n");
}

void help(void) {
  printf("Usage: %s [-i inbase] [-o outbase] [options] string1 string2 ...\n\n", program_name);
  printf("Argumets & options:\n"
"   -i [inbase]  Inbase, the base of the given string. Base can range from 2 to\n"
"                256, use 'a' to specify that the input string is alphabet.\n"
"   -o [inbase]  Outbase, like inbase, but specify that the base of output string,\n"
"                use 'a' to specify the output string is alphabet.\n"
"   -d,-D DELIM  use DELM as delimiter to identify each terms in the string.\n"
"                if DELM is not specified, '.' is used by default.\n\n"
"                -D: Tell cbase that the terms delimit by DELIM is processed\n"
"                    rescpectively. For example:\n\n"
"                      cbase -i 10 -o 16 -D ':' 97:98:99\n\n"
"                    will output\n\n"
"                      97:98:99 (10) = 61:62:63 (16)\n"
"                    97,98 and 99 are treated as 10-based number respectively,\n"
"                    and processed repectively.\n\n"
"                -d: Tell cbase that the terms delimit by DELIM is treated\n"
"                    as a single digit. This is often used when the base is\n"
"                    larger than 36. For example:\n\n"
"                      cbase -i 256 -o 10 -d 168.128.0.1\n\n"
"                    \"168.128.0.1\" is an IP address and can be treated as a\n"
"                    256-based number. However, no letters can represent such\n"
"                    large number(Note: In cbase, 0-9a-z is used to present\n"
"                    numbers whose base is less than 36), so DELIM is used to\n"
"                    distinguish each digits for base larger than 36. For the\n"
"                    example, the output is\n\n"
"                       168.128.0.1 (256) = 2826960897 (10)\n\n"
"   -g,-G COUNT  The `-g' option is like the `-d' option, but instead of using\n"
"                 a delimiter, COUNT is used to group each terms. The example in\n"
"                `-D' can be repeated with the `-go' option.\n\n"
"                  cbase -i 10 -o 16 -go 2 979899\n\n"
"                will output\n\n"
"                  979899 (10) = 616263 (16)\n\n"
"                In the same manner, the second example with `-d' is repeated\n"
"                here with the `-gi' option:\n\n"
"                  cbase -i 256 -o 10 -gi 3 168128000001\n\n"
"                will output:\n\n"
"                168128000001 (256) = 2826960897 (10)\n\n"
"                Note that you shound put '0' in front of '0', '1' to make than\n"
"                grouped in there, otherwise, cbase will to process the input.\n\n"
"   -f,-F        The `-f' option is used to dilimit the string ouput.\n"
"                -F: Delimit each term in the ouput string, this is especially\n"
"                    useful when the `-g' or `-a' option is used. For example:\n\n"
"                      cbase -i a -o 10 ntuee\n\n"
"                    will output:\n\n"
"                      ntuee (a) = 110116117101101 (10)\n\n"
"                    but with the `-F ':'` option, cbase will output.\n\n"
"                      ntuee (a) = 110:116:117:101:101 (10)\n\n"
"                -f: Like the `-F` option, but will delimit the digits inside\n"
"                    the term. For example:\n\n"
"                      cbase -i a -o 10 -F '|' -f '.' ntuee\n\n"
"                    will output:\n\n"
"                    ntuee (a) = 1.1.0|1.1.6|1.1.7|1.0.1|1.0.1 (10)\n\n"
"                    Note that, `-f` option will be suppress when the ouput\n"
"                    base is `a'. If it's no suppress, it will crash the\n"
"                    output\n\n"
"   -s[i|o]      Becaue if the `-D` option is specified, the output by default\n"
"                will delimit the string. the `-s' option is used the suppress\n"
"                the delimiter in the output string. For example:\n\n"
"                  cbase -i 10 -o 256 -s 2826960897\n\n"
"                Will output:\n\n"
"                  2826960897 (10) = 168128000001 (256)\n\n"
"                To decode unicode strings, the `-S' option must be used. For\n"
"                Example:\n\n"
"                cbase -i 16 -o a -D '%%' -S [unicode_string]\n\n"
"   -v           Show version information.\n\n"
"   -h           Give this help list.\n\n"
"By default, `-gi' option will not print out delimiter unless the base is greater\n"
"than 36; `-D' option will print out delimiter, but you can suppress it by\n"
"adding the `-S' option.\n\n"
"Please report bugs to Aitjcize <aitjcize@gmail.com>\n");
}
