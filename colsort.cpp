/* colsort.cc
 * Sort columns in alphabetical order - useful for ensuring predictable ordering
 * of fields
 * anand venkataraman - initial version Jan, 1999
 */
#include <cstdio>
#include <cstdlib>

#include <vector>
#include <algorithm>

using namespace std;

inline bool wordComparator(const char *s1, const char *s2) {return strcmp(s2,s1) > 0;}

void usage() {
  fprintf(stderr, "Colsort: Strictly a filter. Sort whitespace separated columns in alphabetical order.\n");
  fprintf(stderr, "  Use it to ensure consistent ordering of data in a pipeline to match downstream regexes.\n");
  fprintf(stderr, "  e.g. gunzip -dfc access_log* | sed -e 's/[?&]/ /g' | grep -i 'param1.*param2'\n");
  fprintf(stderr, "  You need this because User-Agents are notorious for haphazard param orderings.\n\n");
  exit(1);
}

/* Split a string into its whitespace separated components. Changes s using
 * strtok.
 */
int splitStr(char *s, vector<char *> &v) {
  char *savePtr;
  char *p;

  v.clear();
  for (p = strtok_r(s, " \t\n", &savePtr); p != NULL; p = strtok_r(NULL, " \t\n", &savePtr))
    v.push_back(p);
  return v.size();
}

int main(int argc, char **argv) {
  char buf[BUFSIZ];
  vector<char *> words;
  
  if (argc != 1)
    usage();
  
  while (fgets(buf, BUFSIZ, stdin)) {
    (void) splitStr(buf, words);
    std::sort(words.begin(), words.end(), wordComparator);
    for (size_t i = 0; i < words.size(); i++)
      printf("%s%s", words[i], i<words.size()-1? " " : "\n");
  }
  return 0;
}
