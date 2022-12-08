#include <string.h>
#include <version.h>

/* - On some systems git is not installed or
 *   installed on a place where FindGit.cmake cannot find it
 * - No git information is present (no .git directory)
 * in those cases clear the gitVersionString (would have string GIT-NOTFOUND).
 */
const char *getGitVersion(void)
{
#define BUF_SIZE 100
  static char gitVersionString[BUF_SIZE];
  static bool init = false;
  if (!init)
  {
    strncpy(gitVersionString,"9d3b2855aa803a18bfc00fe5127e0bd7729c9f1f",BUF_SIZE-1);
    strncat(gitVersionString,!strcmp("true","true")?"*":"",BUF_SIZE-1);
    if (!strcmp("9d3b2855aa803a18bfc00fe5127e0bd7729c9f1f", "GIT-NOTFOUND")) gitVersionString[0] = '\0';
    gitVersionString[BUF_SIZE-1]='\0';
    init = true;
  }
  return gitVersionString;
}
