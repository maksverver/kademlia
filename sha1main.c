/*
 sha1main.c: Implementation of SHA-1 Secure Hash Algorithm-1

 Based upon: NIST FIPS180-1 Secure Hash Algorithm-1
   http://www.itl.nist.gov/fipspubs/fip180-1.htm

 Non-official Japanese Translation by HIRATA Yasuyuki:
   http://yasu.asuka.net/translations/SHA-1.html

 Copyright (C) 2002 vi@nwr.jp. All rights reserved.

 This software is provided 'as-is', without any express or implied
 warranty. In no event will the authors be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgement in the product documentation would be
    appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as beging the original software.
 3. This notice may not be removed or altered from any source distribution.

 Note:
   The copyright notice above is copied from md5.h by L. Peter Deutsch
   <ghost@aladdin.com>. Thank him since I'm not a good speaker of English. :)
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sha1.h"

/* Global options */
int	opt_text = 
#ifdef	MSDOS
  0
#else
  1
#endif
;
int	opt_check = 0;
int	opt_status = 0;
int	opt_warn = 0;
char	*myname;

#define	MODE_TEXT	"rt"
#define	MODE_BINARY	"rb"

static int sha1it(char *fname, sha1_byte_t *p)
{
  sha1_state_s	pms;
  sha1_byte_t input[BUFSIZ];
  int s;
  FILE *fp;

  if(fname == NULL || p == NULL) {
    fprintf(stderr, "Bug: file name or return buffer is NULL\n");
    return 1;
  }
  if(strcmp(fname, "-") == 0) {
    fp = stdin;
  } else if((fp = fopen(fname, opt_text ? MODE_TEXT : MODE_BINARY)) == NULL) {
    fprintf(stderr, "%s: ", myname);
    perror(fname);
    return 1;
  }
  sha1_init(&pms);
  while((s = fread(input, sizeof(sha1_byte_t), BUFSIZ, fp)) > 0) {
    sha1_update(&pms, input, s);
  }
  if(ferror(fp)) {
    fprintf(stderr, "%s: ", myname);
    perror(fname);
    return 1;
  }
  fclose(fp);
  sha1_finish(&pms, p);
  return 0;
}

static void usage()
{
  fprintf(stderr, 
"Usage: %s [OPTION] [FILE] ...\n"
"  or : %s [OPTION] --check [FILE]\n"
"Print or check sha1 checksums.\n"
"With no FILE, or when FILE is -, read standard input.\n"
"\n"
"  -b, --binary         read files in binary mode (default on DOS/Windows)\n"
"  -c, --check          check SHA1 sums against given list\n"
"  -t, --text           read files in text mode (default)\n"
"\n"
"The following two options are useful only when verifying checksums:\n"
"\n"
"      --status         don't output anything, status code shows success\n"
"  -w, --warn           warn about improperly formatted checksum lines\n"
"\n"
"      --help           display this help and exit\n"
"      --version        output version information and exit\n"
"\n"
"The sums are computed as described in FIPS 180-1. When checking, the input\n"
"should be former output of this program.  The default mode is to print\n"
"a line with checksum, a character indicating type (`*' for binary, ` ' for\n"
"text), and name for each FILE.\n"
"\n"
"Please report bugs to <vi@nwr.jp>\n", myname, myname);
  exit(0);
}

static void unknown_option(char *opt)
{
  fprintf(stderr, "Unknown option `%s'\nConsult with `%s --help' for usage\n", opt, myname);
  exit(1);
}

static void print_version()
{
  fprintf(stderr, 
"%s 1.0\n"
"Author: vi@nwr.jp\n"
"\n"
"Copyright(C) 2002 vi@nwr.jp\n"
"This is free software; see the source for copying conditions. There is NO\n"
"WARRANTY; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
  myname);
  exit(0);
}

static void improperline(char *fname, int line)
{
  fprintf(stderr, "%s: %s: %d: improperly formatted SHA1 checksum line\n", myname, fname, line);
}

static void donotmatch(char *fname, char *target)
{
/*
  fprintf(stderr, "%s: %s: %s: checksum does not match\n", myname, fname, target);
 */
}

static int done(char *fname, int succ, int total)
{
  if(total == 0) {
    fprintf(stderr, "%s: %s: no properly formatted SHA1 checksum line\n", myname, fname);
    return 1;
  } else if(total == succ) {
    fprintf(stderr, "%s: %s: ok\n", myname, fname);
    return 0;
  } else {
    fprintf(stderr, "%s: %s: failed\n%s: Warning: %d of %d checksum(s) does not match\n", myname, fname, myname, total - succ, total);
    return 1;
  }
}

static int checkit(char *fname)
{
  FILE *fp;
  char fnbuf[BUFSIZ], *p, *q;
  sha1_byte_t	g1[SHA1_OUTPUT_SIZE], g2[SHA1_OUTPUT_SIZE];
  int i, j, line;
  int succ, tested;

  succ = 0, tested = 0;
  line = 0;
  if(strcmp(fname, "-") == 0) {
    fname = "stdin";
    fp = stdin;
  } else if((fp = fopen(fname, opt_text ? MODE_TEXT : MODE_BINARY)) == NULL) {
    fprintf(stderr, "%s: ", myname);
    perror(fname);
  }
top:
  line++;
  while(fgets(fnbuf, BUFSIZ - 1, fp)) {
    for(p = fnbuf, i = 0; i < 20; i++) {
      switch(*p++) {
      case '0': j = 0; break; case '1': j = 1; break; case '2': j = 2; break; case '3': j = 3; break;
      case '4': j = 4; break; case '5': j = 5; break; case '6': j = 6; break; case '7': j = 7; break;
      case '8': j = 8; break; case '9': j = 9; break; case 'a': j = 10; break; case 'b': j = 11; break;
      case 'c': j = 12; break; case 'd': j = 13; break; case 'e': j = 14; break; case 'f': j = 15; break;
      default:
        if(opt_warn) {
          improperline(fname, line);
          goto top;
        }
      }
      j <<= 4;
      switch(*p++) {
      case '0': j += 0; break; case '1': j += 1; break; case '2': j += 2; break; case '3': j += 3; break;
      case '4': j += 4; break; case '5': j += 5; break; case '6': j += 6; break; case '7': j += 7; break;
      case '8': j += 8; break; case '9': j += 9; break; case 'a': j += 10; break; case 'b': j += 11; break;
      case 'c': j += 12; break; case 'd': j += 13; break; case 'e': j += 14; break; case 'f': j += 15; break;
      default:
        if(opt_warn) {
          improperline(fname, line);
          goto top;
        }
      }
      g2[i] = j;
    }
    switch(*p++) {
    default:
      if(opt_warn) {
        improperline(fname, line);
        goto top;
      }
    case ' ':
      opt_text = 1; break;
    case '*':
      opt_text = 0; break;
    }
    for(q = p; *p && *p != '\n'; p++);
    *p = 0;
    if(*q == 0) {
      improperline(fname, line);
      goto top;
    }
    tested++;
    if(sha1it(q, g1)) {
      goto top;
    }
    if(memcmp(g1, g2, sizeof(g1)) == 0) {
      succ++;
    } else {
      donotmatch(fname, q);
    }
  }
  if(ferror(fp)) {
    fprintf(stderr, "%s: ", myname);
    perror(fname);
    return 1;
  }
  fclose(fp);
  return done(fname, succ, tested);
}

static void print_status(sha1_byte_t output[SHA1_OUTPUT_SIZE], char *file)
{
  int i;

  for(i = 0; i < SHA1_OUTPUT_SIZE; i++) {
    printf("%02x", output[i]);
  }
  printf("%c%s\n", opt_text ? ' ' : '*', file);
}

int	main(int argc, char **argv)
{
  int	i;
  sha1_byte_t output[SHA1_OUTPUT_SIZE];
  int	status = 0, xs;

  myname = argv[0];
/* -bctw --binary --check --text --status --warn --help --version */
  for(i = 1; i < argc; i++) {
    if(argv[i][0] != '-' ||
      strcmp(argv[i], "-") == 0 ||
      strcmp(argv[i], "--") == 0) {
      break;
    }
    if(argv[i][1] == '-') {
      if(strcmp(argv[i], "--binary") == 0) {
        opt_text = 0;
      } else if(strcmp(argv[i], "--text") == 0) {
        opt_text = 1;
      } else if(strcmp(argv[i], "--check") == 0) {
        opt_check = 1;
      } else if(strcmp(argv[i], "--status") == 0) {
        opt_status = 1;
      } else if(strcmp(argv[i], "--warn") == 0) {
        opt_warn = 1;
      } else if(strcmp(argv[i], "--help") == 0) {
        usage();
      } else if(strcmp(argv[i], "--version") == 0) {
        print_version();
      } else {
        unknown_option(argv[i]);
      }
    } else {
      int j;

      for(j = 1; argv[i][j]; j++) {
        switch(argv[i][j]) {
        default:
          {
            char v[2];

            v[0] = argv[i][j], v[1] = 0;
            unknown_option(v);
          }
          exit(0);
        case 'b':
          opt_text = 0;
          break;
        case 'c':
          opt_check = 0;
          break;
        case 't':
          opt_text = 1;
          break;
        case 'w':
          opt_warn = 1;
          break;
        }
      }
    }
  }

  if(opt_check) {
    status = checkit(i >= argc ? "-": argv[i]);
  } else {
    if(i >= argc) {
      if((status = sha1it("-", output)) == 0) {
        print_status(output, "");
      }
    } else {
      status = 0;
      for(; i < argc; i++) {
        if((xs = sha1it(argv[i], output)) == 0) {
          print_status(output, argv[i]);
        }
        status |= xs;
      }
    }
  }
  exit(status);
}

