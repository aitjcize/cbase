/* cbase.c - v0.1
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#define A_MAX 4096
#define ARGS_MAX 64
#define BASE_MAX 256
#define BASE_STRLEN_MAX 10
#define TR_MAX 10

#define PROGRAM_NAME "cbase"
#define VERSION "0.1.2"

/* some global flags */
bool outter_string_cut = false;
bool outter_string_group = false;
bool inner_string_cut = false;
bool inner_string_group = false;
bool outter_use_field = false;
bool inner_use_field = false;
bool outter_suppress = false;
bool inner_suppress = false;
bool from_alpha = false;
bool to_alpha = false;

/* global variables */

/* function definition */
char* trans(char* target, char* buf, int base_from, int base_to, char fd, int ign, char* uifd);
char* itoa(int num);
void usage(void);

int main(int argc, char  *argv[])
{
  char* program_name = argv[0];
  char *optargs[ARGS_MAX];
  int argcount = 0;                             /* argumant to process count */
  char c_base_from[BASE_STRLEN_MAX], c_base_to[BASE_STRLEN_MAX];
  char output[A_MAX *8];
  char alpha[A_MAX *3];                         /* for alphabet */
  char *send;
  char uifd[2], uofd[2];
  char sc;
  char ifd = '.', ofd = '.';
  int ign = 0, ogn = 0;
  char buf[A_MAX];
  int i, j;

  strcpy(uofd, "."); /* initialize field */
  strcpy(uifd, ".");

  for(i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      for(j = 1; j < strlen(argv[i]); j++)
        switch(argv[i][j]) {
          case 'i':
            if(i == argc -1) {
              fprintf(stderr ,"cbase: no input base specified.\n");
              exit(1);
            }
            if(strlen(argv[i +1]) > A_MAX) {
              fprintf(stderr ,"cbase: invalid base.\n");
              exit(1);
            }
            strcpy(c_base_from, argv[++i]);
            if((atoi(c_base_from) < 2 || atoi(c_base_from) > 256) && strcmp(c_base_from, "a")) {
              printf("error: base not allowed.\n");
              exit(1);
            }
            j++;
            break;

          case 'o':
            if(i == argc -1) {
              fprintf(stderr ,"cbase: no output base specified.\n");
              exit(1);
            }
            if(strlen(argv[i +1]) > A_MAX) {
              fprintf(stderr ,"cbase: invalid base.\n");
              exit(1);
            }
            strcpy(c_base_to, argv[++i]);
            if((atoi(c_base_to) < 2 || atoi(c_base_to) > 256) && strcmp(c_base_to, "a")) {
              printf("error: base not allowed.\n");
              exit(1);
            }
            j++;
            break;

          case 'd':
            sc = argv[i][j +1];
            if(sc == '\0') {
              fprintf(stderr ,"cbase: please specify inner or outter.\n");
              exit(1);
            }
            else if (sc == 'i' || sc == 'o') {
              if((sc == 'i' && inner_string_group) || (sc == 'o' && outter_string_group)) {
                fprintf(stderr ,"cbase: conflicted option.\n");
                exit(1);
              }
              sc == 'i' ? (inner_string_cut = true): (outter_string_cut = true);
              if(i < argc -1)
                if(strlen(argv[i +1]) == 1) {
                  if(sc == 'i')
                    ifd = argv[++i][0];
                  else
                    ofd = argv[++i][0];
                }
              j++;
            }
            else {
              fprintf(stderr ,"cbase: invalid option `%c'.\n", sc);
              exit(1);
            }
            break;

          case 'g':
            if(i == argc -1) {
              fprintf(stderr ,"cbase: option without value --`%c'.\n", argv[i][j]);
              exit(1);
            }
            sc = argv[i][j +1];
            if(strlen(argv[i +1]) > 1) {
              fprintf(stderr ,"cbase: please specify group count.\n");
              exit(1);
            }

            if(sc == '\0') {
              fprintf(stderr ,"cbase: please specify inner or outter.\n");
              exit(1);
            }
            else if (sc == 'i' || sc == 'o') {
              if((sc == 'i' && inner_string_cut) || (sc == 'o' && outter_string_cut)) {
                fprintf(stderr ,"cbase: conflicted option.\n");
                exit(1);
              }
              sc == 'i' ? (inner_string_group = true): (outter_string_group = true);
              if(sc == 'i')
                ign = atoi(argv[++i]);
              else
                ogn = atoi(argv[++i]);
              if((sc == 'i'? ign: ogn) == 0) {
                fprintf(stderr ,"cbase: group count can not be zero.\n");
                exit(1);
              }
              j++;
            }
            else {
              fprintf(stderr ,"cbase: invalid option `%c'.\n", sc);
              exit(1);
            }
            break;

          case 'f':
            sc = argv[i][j +1];
            if(sc == '\0') {
              fprintf(stderr ,"cbase: please specify inner or outter.\n");
              exit(1);
            }
            else if (sc == 'i' || sc == 'o') {
              if((sc == 'i' && inner_suppress) || (sc == 'o' && outter_suppress)) {
                fprintf(stderr ,"cbase: conflicted option.\n");
                exit(1);
              }
              sc == 'i'? (inner_use_field = true): (outter_use_field = true);
              if(i < argc -1)
                if(strlen(argv[i +1]) == 1)
                  strncpy(sc == 'i'? uifd: uofd, argv[++i], 1);
              j++;
            }
            else {
              fprintf(stderr ,"cbase: invalid option `%c'", sc);
              exit(1);
            }
            break;

          case 's':
            sc = argv[i][j +1];
            if(sc == '\0') {
              fprintf(stderr ,"cbase: please specify inner or outter.\n");
              exit(1);
            }
            else if (sc == 'i' || sc == 'o') {
              if((sc == 'i' && inner_use_field) || (sc == 'o' && outter_use_field)) {
                fprintf(stderr ,"cbase: conflicted option.\n");
                exit(1);
              }
              sc == 'i'? (inner_suppress = true): (outter_suppress = true);
              j++;
            }
            else {
              fprintf(stderr ,"cbase: invalid option `%c'", sc);
              exit(1);
            }
            break;

          case 'h':
            usage();
            exit(0);

          case 'v':
            fprintf(stderr ,"cbase Ver. %s\n", VERSION);
            printf("Copyright (C) 2009 Aitjcize (Wei-Ning Huang)\n");
            printf("License GPLv2 <http://gnu.org/licenses/gpl.html>\n");
            printf("This is free software: you are free to change and redistribute it.\n");
            printf("There is NO WARRANTY, to the extent permitted by law.\n\n");
            printf("Written by Aitjcize (Wei-Ning Huang).\n");
            exit(0);

          default:
            fprintf(stderr ,"cbase: unknown option `%c'.\n", argv[i][j]);
            printf("Try `%s -h' for more information.\n", program_name);
            exit(1);
        }
    }
    else {
      if(strlen(argv[i]) > A_MAX) {
        fprintf(stderr ,"cbase: string too long.\n");
        exit(1);
      }
      optargs[argcount++] = argv[i];
    }
  }

  if(argcount == 0) {
    fprintf(stderr ,"cbase: no input.\n");
    exit(1);
  }
  if(atoi(c_base_from) > BASE_MAX || atoi(c_base_to) > BASE_MAX) {
    fprintf(stderr ,"cbase: invalid base.\n");
  }

  /* start processing */
  for(i = 0; i < argcount; i++) {
    output[0] = '\0';
    if(strcmp(c_base_from, c_base_to) == 0) {
      printf("%s (%s) = %s (%s)\n", optargs[i], c_base_from, 
          optargs[i], c_base_from);
      continue;
    }

    send = optargs[i];

    /* preprocess for input alphabet */
    if(strcmp(c_base_from, "a") == 0 || from_alpha) {
      from_alpha = true;
      char stmp[4];
      alpha[0] = '\0';
      outter_string_group = true;
      ogn = 3;
      for(j = 0; j < strlen(optargs[i]); j++) {
        if(optargs[i][j] < 10)
          sprintf(stmp, "00%d", optargs[i][j]);
        else if (optargs[i][j] < 100)
          sprintf(stmp, "0%d", optargs[i][j]);
        else
          sprintf(stmp, "%d", optargs[i][j]);
        strcat(alpha, stmp);
      }
      strcpy(c_base_from, "10");
      send = alpha;
    }

    /* preprocess for output alphabet */
    if(strcmp(c_base_to, "a") == 0) {
      to_alpha = true;
      strcpy(c_base_to, "10");
      //if(!outter_string_group && !outter_string_cut) {
      //  fprintf(stderr, "cbase: please use `-do' or `-go' to group.\n");
      //  exit(1);
      //}
    }

    if(outter_string_cut) {                     /* # CUT */
      int pos = -1;                             /* positon will scanning */
      for(j = 0; j < strlen(send); j++) {
        char scan[A_MAX], tmp[2];
        if(send[j] == ofd || j +1 == strlen(send)) {
          strcpy(scan, send);
          if(j +1 == strlen(send)) j++;
          scan[j] = '\0';
          trans(scan +pos +1, buf, atoi(c_base_from), atoi(c_base_to), ifd, ign, uifd);
          if(to_alpha && atoi(buf) > 255) {
            fprintf(stderr ,"cbase: not an alphabet `%d'\n", atoi(buf));
            exit(1);
          }
          if(strlen(output) +strlen(buf) +1> A_MAX *8) {
            fprintf(stderr ,"cbase: string too long.\n");
            exit(1);
          }
          if(to_alpha) {
            tmp[0] = atoi(buf);
            tmp[1] = '\0';
            strcat(output, tmp);
          }
          else
            strcat(output, buf);

          if(j != strlen(send) && !outter_suppress) { /* put delimiter */
            if(outter_use_field)
              strcat(output, uofd);
            else {
              tmp[0] = ofd;
              tmp[1] = '\0';
              strcat(output, tmp);
            }
          }
          pos = j;
        }
      }
    }
    else if(outter_string_group) {              /* # GROUP */
      if(strlen(send) % ogn) {
        fprintf(stderr ,"cbase: group count not match.\n");
        exit(1);
      }
      char scan[A_MAX];
      if(ogn == 0)
        ogn = strlen(send);
      for(j = 0; j < strlen(send); j += ogn) {
        char tmp[2];
        strcpy(scan, send);
        scan[j +ogn] = '\0';
        trans(scan +j, buf, atoi(c_base_from), atoi(c_base_to), ifd, ign, uifd);
        if(to_alpha && atoi(buf) > 255) {
          fprintf(stderr ,"cbase: not an alphabet `%d'\n", atoi(buf));
          exit(1);
        }
        if(strlen(output) +strlen(buf) +1> A_MAX *8) {
          fprintf(stderr ,"cbase: string too long.\n");
          exit(1);
        }
        if(to_alpha) {
          tmp[0] = atoi(buf);
          tmp[1] = '\0';
          strcat(output, tmp);
        }
        else
          strcat(output, buf);
        if(outter_use_field && j != strlen(send) -ogn && !outter_suppress) /* put delimiter */
          strcat(output, uofd);
      }
    }
    else {                                      /* # OTHERS */
      trans(optargs[i], buf, atoi(c_base_from), atoi(c_base_to), ifd, ign, uifd);
      if(strlen(output) +strlen(buf) > A_MAX *8) {
        fprintf(stderr ,"cbase: string too long.\n");
        exit(1);
      }
      if(to_alpha) {
        char tmp[2];
        tmp[0] = atoi(buf);
        tmp[1] = '\0';
        strcat(output, tmp);
      }
      else
        strcat(output, buf);
    }
    printf("%s (%s) = %s (%s)\n", optargs[i], from_alpha? "a": c_base_from, output,
        to_alpha? "a": c_base_to);
  }
  if(to_alpha && inner_use_field)
    fprintf(stderr ,"cbase: delimitor suppressed when printing alphabet.\n");

  printf("\n");
  return 0;
}

char* trans(char* target, char* buf, int base_from, int base_to, char fd, int ign, char* uifd)
{
  int total = 0,                                /* total delimiter*/
      count = 0,                                /* string position */
      curr = 0,            /* for current position when counting words*/
      max_digits = 0;
  int i, c, pos = -1;
  unsigned long long int sum = 0, tmp = 0, times = 0;
  char *tr;                                     /* for insert char when c > 36 */
  char t;
  char *scan = malloc(from_alpha? A_MAX *3: A_MAX);

  buf[0] = '\0';                                /* reset buffer */

  /* start convert to dec from base_from */
  if(inner_string_cut) {                        /* # CUT */
    for(i = 0; i < strlen(target); i++)
      if(target[i] == fd) total++;

    if(target[strlen(target) -1] == fd) {       /* remove the last delimiter */
      target[strlen(target) -1] = '\0';
      total--;
    }
    for(i = 0; i < strlen(target); i++) {
      strcpy(scan, target);
      if(target[i] == fd || (i +1 == strlen(target))) { /* or it's the last one */
        if(i +1 == strlen(target)) i++;
        scan[i] = '\0';
        curr++;
        times = atoi(scan +pos +1);
        if(times >= base_from) {
          printf("cbase: wrong base, %lld is greater than %d.\n", times, base_from);
          exit(1);
        }
        sum += times *pow(base_from, total -curr +1);
        pos = i;
      }
    }
  }
  else if(inner_string_group) {                 /* # GROUP */
    if(strlen(target) % ign) {
      fprintf(stderr ,"cbase: group count not match.\n");
      exit(1);
    }
    for(i = 0; i < strlen(target); i += ign) {
      strcpy(scan, target);
      scan[i +ign] = '\0';
      times = atoi(scan +i);
      if(times >= base_from) {
        printf("cbase: wrong base, %lld is greater than %d.\n", times, base_from);
        exit(1);
      }
      sum += times *pow(base_from, strlen(target)/ign -i/ign -1);
    }
  }
  else {                                        /* # OTHERS */
    for (i = 0; i < strlen(target); i++) {
      char ck = target[strlen(target) -i -1];
      if(ck >= 'a' && ck <= 'z')
        times = ck -'a' +10;
      else if(ck >= 'A' && ck <= 'Z')
        times = ck -'A' +10;
      else if (ck >= '0' && ck <= '9')
        times = ck -'0';
      else {
        printf("cbase: wrong base, please use `-di' or `-gi' to group.\n");
        exit(1);
      }
      if(times >= base_from) {
        printf("cbase: wrong base, %lld is greater than %d.\n", times, base_from);
        exit(1);
      }
      sum += times *pow(base_from, i);
    }
  }

  /* start convert to base_to */
  tmp = sum;
  if(tmp == 0)
    strcpy(buf, "0");
  /* get max digits */
  while(tmp) {
    c = tmp % base_to;
    if(strlen(itoa(c)) > max_digits)
      max_digits = strlen(itoa(c));
    tmp /= base_to;
  }

  tmp = sum;
  while(tmp) {
    if(count > (to_alpha? A_MAX*3: A_MAX)) {
      fprintf(stderr ,"cbase: output string too long.\n");
      exit(1);
    }
    c = tmp % base_to;
    tmp /= base_to;
    if(base_to > 10 && base_to <= 36 && c >= 10) { /* # digits >= 10 */
      if(inner_use_field && !inner_suppress && !to_alpha) {
        strcat(buf, uifd);
        count++;
      }
      buf[count++] = c -10 +'a';
      buf[count] = '\0';
    }
    else if (base_to > 36) {                      /* # for digits > 36 */
      tr = itoa(c);
      if(!inner_suppress && !to_alpha) {
        strcat(buf, uifd);
        count++;
      }
      /* we must put it in reverse order because we reverse the whole
       * string at the end */
      for(i = 0; i < strlen(tr); i++) {
        if(count +i > (to_alpha? A_MAX *3: A_MAX)) {
          printf("\ncbase: output string too long.\n");
          exit(1);
        }
        buf[count +i] = tr[strlen(tr) -i -1];
      }
      count += strlen(tr);
      buf[count] = '\0';
      if(strlen(tr) < max_digits)
        for(i = 0; i < max_digits -strlen(tr); i++) {
          strcat(buf, "0");
          count++;
        }
    }
    else {                                      /* # for normal */
      if(inner_use_field && !inner_suppress && !to_alpha) {
        strcat(buf, uifd);
        count++;
      }
      buf[count++] = c +'0';
      buf[count] = '\0';
    }
  }

  /* reverse the string */
  for(i = 0; i < strlen(buf) /2; i++) {
    t = buf[i];
    buf[i] = buf[strlen(buf) -i -1];
    buf[strlen(buf) -i -1] = t;
  }

  if(buf[strlen(buf) -1] == uifd[0])
    buf[strlen(buf) -1] = '\0';

  free(scan);
  return buf;
}

char* itoa(int num)                             /*  This is neat! */
{
  int i;
  static char cha[64];
  int tmp = 0;
  tmp = (num < 0)? -num: num;
  int digits = 0;
  while((int) (num / pow(10, digits)))
    digits++;
  if(digits > 64)
  {
    fprintf(stderr ,"cbase: number to large.\n");
    exit(1);
  }
  for(i = 0; i < digits; i++)
  {
    cha[digits -i -(num > 0)] = tmp % 10 +'0';
    tmp /= 10;
  }
  if(num < 0) cha[0] = '-';
  cha[digits +(num < 0)] = '\0';
  return cha;
}

void usage(void)
{
  printf("Usage: %s [-i inbase] [-o outbase] [options] string1 string2 ...\n\n", PROGRAM_NAME);
  printf("Argumets & options:\n");
  printf("   -i [inbase]    Inbase, the base of the given string. Base can range from 2 to\n");
  printf("                  256, use 'a' to specify that the input string is alphabet.\n");
  printf("   -o [inbase]    Outbase, like inbase, but specify that the base of output string,\n");
  printf("                  use 'a' to specify the output string is alphabet.\n");
  printf("   -d[i|o] DELIM  use DELM as delimiter to identify each terms in the string.\n");
  printf("                  if DELM is not specified, '.' is used by default.\n\n");
  printf("                  -do: Tell cbase that the terms delimit by DELIM is processed\n");
  printf("                       rescpectively. For example:\n\n");
  printf("                         cbase -i 10 -o 16 -do ':' 97:98:99\n\n");
  printf("                       will output\n\n");
  printf("                         97:98:99 (10) = 61:62:63 (16)\n");
  printf("                       97,98 and 99 are treated as 10-based number respectively,\n");
  printf("                       and processed repectively.\n\n");
  printf("                  -di: Tell cbase that the terms delimit by DELIM is treated\n");
  printf("                       as a single digit. This is often used when the base is\n");
  printf("                       larger than 36. For example:\n\n");
  printf("                         cbase -i 256 -o 10 -di 168.128.0.1\n\n");
  printf("                       \"168.128.0.1\" is an IP address and can be treated as a\n");
  printf("                       256-based number. However, no letters can represent such\n");
  printf("                       large number(Note: In cbase, 0-9a-z is used to present\n");
  printf("                       numbers whose base is less than 36), so DELIM is used to\n");
  printf("                       distinguish each digits for base larger than 36. For the\n");
  printf("                       example, the output is\n\n");
  printf("                         168.128.0.1 (256) = 2826960897 (10)\n\n");
  printf("   -g[i|o] COUNT  The `-g' option is like the `-d' option, but instead of using\n");
  printf("                  delimiter, COUNT is used to group each terms. The example in\n");
  printf("                  `-do' can be repeated with the `-go' option.\n\n");
  printf("                    cbase -i 10 -o 16 -go 2 979899\n\n");
  printf("                  will output\n\n");
  printf("                    979899 (10) = 616263 (16)\n\n");
  printf("                  In the same manner, the second example with `-di' is repeated\n");
  printf("                  here with the `-gi' option:\n\n");
  printf("                    cbase -i 256 -o 10 -gi 3 168128000001\n\n");
  printf("                  will output:\n\n");
  printf("                  168128000001 (256) = 2826960897 (10)\n\n");
  printf("                  Note that you shound put '0' in front of '0', '1' to make than\n");
  printf("                  grouped in there, otherwise, cbase will to process the input.\n\n");
  printf("   -f[i|o]        The `-f' option is used to dilimit the string ouput.\n");
  printf("                  -fo: Delimit each term in the ouput string, this is especially\n");
  printf("                       useful when the `-g' or `-a' option is used. For example:\n\n");
  printf("                         cbase -i a -o 10 ntuee\n\n");
  printf("                       will output:\n\n");
  printf("                         ntuee (a) = 110116117101101 (10)\n\n");
  printf("                       but with the `-fo ':'` option, cbase will output.\n\n");
  printf("                         ntuee (a) = 110:116:117:101:101 (10)\n\n");
  printf("                  -fi: Like the `-fo` option, but will delimit the digits inside\n");
  printf("                       the term. For example:\n\n");
  printf("                         cbase -i a -o 10 -fo '|' -fi '.' ntuee\n\n");
  printf("                       will output:\n\n");
  printf("                       ntuee (a) = 1.1.0|1.1.6|1.1.7|1.0.1|1.0.1 (10)\n\n");
  printf("                       Note that, `-fi` option will be suppress when the ouput\n");
  printf("                       base is `a'. If it's no suppress, it will crash the\n");
  printf("                       output\n\n");
  printf("   -s[i|o]        Becaue if the `-do` option is specified, the output by default\n");
  printf("                  will delimit the string. the `-s' option is used the suppress\n");
  printf("                  the delimiter in the output string. For example:\n\n");
  printf("                    cbase -i 10 -o 256 -si 2826960897\n\n");
  printf("                  Will output:\n\n");
  printf("                    2826960897 (10) = 16812801 (256)\n\n");
  printf("                  To decode unicode strings, the `-so' option must be used. For\n");
  printf("                  Example:\n\n");
  printf("                  cbase -i 16 -o a -do '%%' -so [unicode_string]\n\n");
  printf("   -v             Show version information.\n\n");
  printf("   -h             Give this help list.\n\n");
  printf("By default, `-gi' option will not print out delimiter unless the base is greater\n");
  printf("than 36; `-do' option will print out delimiter, but you can suppress it by\n");
  printf("adding the `-so' option.\n\n");
  printf("Please report bugs to Aitjcize <aitjcize@gmail.com>\n");
  return;
}

