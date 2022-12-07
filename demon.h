#include <syslog.h>
#define MAXFD 64

void daemon_init(const char *pname, int facility)
{
  int i;
  pid_t pid;
  if ((pid = fork()) != 0)
  {
    exit(0);
  }
  setsid();
  signal(SIGHUP, SIG_IGN);
  if ((pid = fork()) != 0)
  {
    exit(0);
  }
  chdir("/");
  umask(0);
  for (i = 0; i < MAXFD; i++)
  {
    close(i);
  }
  openlog(pname, LOG_PID, LOG_LOCAL1);
  syslog(LOG_INFO, "Dziala!!");
}
