/* Uniqify lines without sorting - how cool is that?
 * anand venkataraman - initial version Jan, 1999
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>

#include <openssl/md5.h> // for md5 in printUniqs()
#include <ext/hash_set>  // for alreadyPrinted, in printUniqs()
#include <ext/hash_map>  // for counts, in countUniqs()

using namespace __gnu_cxx;

// Prototypes
void usage(void);
void printUniqs(void);

void usage() {
  char *usageMsg =
   "Uniqify: Print only first occurrence of each line regardless of input sort order, unlike uniq(1).\n\
   \n\
   Advantages over sort -u:\n\
    (1) Immediate output (2) Upto 2x faster (3) Typically, much smaller mem profile\n\
   Cons & Caveats:\n\
    (1) Uniqueness is upto MD5 of input (2) Output is unsorted (3) No count mode\n\
   \n\
   Usage: cat infile.txt | uniqify [-f NUM] >outfile.txt\n\
     -f NUM uniqifies on field NUM only (not entire line)\n\
   \n\
   Notes: uniqify is designed along the lines of a Bloom filter (http://en.wikipedia.org/wiki/Bloom_filter)\n\
   and so there is a teeeeny chance of false-positives in tagging dupes.\n";

  fprintf(stderr, "%s\n", usageMsg);
  exit(1);
}

// Functors for hashing
//
struct hashMd5 {
  size_t operator()(const unsigned char *md5) const {
    size_t h = 0;
    for (size_t i = 0; i < 16; i++)
      h = 31*h+md5[i];
    return h;
  }
};

struct eqMd5 {
  bool operator()(const unsigned char *m1, const unsigned char *m2) const {
    for (size_t i = 0; i < 16; i++)
      if (m1[i] != m2[i]) return false;
    return true;
  }
};

struct eqStr {
  bool operator()(const char *s1, const char *s2) const {
    return strcmp(s1,s2) == 0;
  }
};

/* Split a string into its whitespace separated components. Changes s using
 * strtok. This is only needed for the -f NUM option
 */
int splitStr(char *s, vector<char *> &v) {
  char *savePtr;
  char *p;

  v.clear();
  for (p = strtok_r(s, " \t\n", &savePtr); p != NULL; p = strtok_r(NULL, " \t\n", &savePtr))
    v.push_back(p);
  return v.size();
}

// Use a Bloom filter-like technique to remember if an input line was seen before.
// Since each line is passed through to stdout immediately upon seeing it for the
// first time only, and NEVER printed a second time, the actual input lines themselves
// *need not* be remembered. This works well for text corpora with exceedingly long lines
// which might take up large amounts of RAM in sort -u. The trade-off is that a md5 collision
// might eat up that occasional line that has the same md5 as an earlier one. However, for a 16
// byte MD5 as we're using below, the space is large (16 Billion Billion) and so the compromise
// is a wise one most of the time except in critically important situations
//
inline void printUniqs(int fnum) {
  char buf[BUFSIZ], bufcopy[BUFSIZ];
  hash_set<unsigned char *, hashMd5, eqMd5> alreadyPrinted;
  char *stringToUniqify;

  while (fgets(buf, BUFSIZ, stdin)) {
    MD5_CTX c;
    unsigned char md5[16];

    stringToUniqify = buf;
    if (fnum > 0) {
      strcpy(bufcopy, buf);
      vector<char *> v;
      splitStr(bufcopy, v);
      if (fnum < (int) v.size())
	stringToUniqify = v[fnum-1];
    }

    MD5_Init(&c);
    MD5_Update(&c, stringToUniqify, strlen(stringToUniqify));
    MD5_Final(md5, &c);

    if (alreadyPrinted.find(md5) == alreadyPrinted.end()) {
      printf("%s", buf);
      unsigned char *storableMd5 = (unsigned char *) malloc(16);
      if (storableMd5) {
	memcpy(storableMd5, md5, 16);
	alreadyPrinted.insert(storableMd5);
      } else {
        perror("malloc");
        exit(1);
      }
    }
  }
  return;
}

int main(int argc, char *argv[]) {
  int fnum = 0;         // By default uniqify on entire line
  
  setbuf(stderr, NULL); // Unbuffer to flush progress msgs with \r

  while (--argc && ++argv) {
    if (!strncmp(*argv, "-f", 2) && argc > 1) {
      int n = atoi(*++argv);
      --argc;
      if (n > 0)
	fnum = n;
    } else
      usage();
  }
  printUniqs(fnum);

  exit(0);
}



