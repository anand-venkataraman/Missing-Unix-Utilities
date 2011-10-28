/* Simple line-counter - a lightweight process that you can stick in a shell pipeline
 * This will simply count the number of lines passing through it and print a progress
 * message every now and then
 * anand venkataraman - Initial version Jan, 1999
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

void checkPoint(char *s, char *tag) {
  char buf[256];
  time_t now = time(NULL);
  static time_t last = now;

  struct tm* tp = localtime(&now);
  sprintf(buf, "%02d/%02d %02d:%02d:%02d [+%lds] Checkpoint (%s): %s\n",
	  tp->tm_mon+1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec, now-last, tag, s);
  fprintf(stderr, "%s", buf);
  last = now;
}

void usage() {
  fprintf(stderr, "Usage: linecount [-q] -tag STR -progress N\n");
  fprintf(stderr, "  Simply pass lines through with same line progress every N lines\n");
  fprintf(stderr, "  and a STR-tagged checkpoint every 10*N lines.\n");
  fprintf(stderr, "  Use -q to eat input (strictly quiet counting mode)\n");
  fprintf(stderr, "  For example try: seq 10000000 | linecount -tag Me -p 100000 >/dev/null\n");
  fprintf(stderr, "  or equivalently: seq 10000000 | linecount -tag Me -q -p 100000\n\n");
  exit(1);
}

int main(int argc, char **argv) {
  char *tag = argv[0];
  u_long progMsgPeriod = 10000;           // Every 10000 lines update with \r
  bool quietMode = false;
  
  setbuf(stderr, NULL);
  setbuf(stdout, NULL);

  for (int i = 1; i < argc; i++) {
    if (!strncmp(argv[i], "-tag", 4) && i < argc-1) {
      tag = argv[++i];
    } else if (!strncmp(argv[i], "-q", 2)) {
      quietMode = true;
    } else if (!strncmp(argv[i], "-p", 2) && i < argc-1) {
      progMsgPeriod = atoi(argv[++i]);
      if (progMsgPeriod <= 0) {
	fprintf(stderr, "Illegal progress message period reset to 10000\n");
	progMsgPeriod = 10000;
      }
    } else {
      usage();
    }
  }

  u_long chkPtPeriod = 10*progMsgPeriod;   // Every 100,000 lines, update with \n
  u_long n = 0;
  char buf[BUFSIZ], msg[BUFSIZ];
  checkPoint("Starting", tag);
  while(fgets(buf, BUFSIZ, stdin) != NULL) {
    ++n;
    if (n % progMsgPeriod == 0) fprintf(stderr, "%lu\r", n);
    if (n % chkPtPeriod == 0) {
      sprintf(msg, "%lu lines passed", n);
      checkPoint(msg, tag);
    }
    if (!quietMode) fputs(buf, stdout);
  }
  sprintf(msg, "%lu lines passed in total", n);
  checkPoint(msg, tag);
  
  return 0;
}
