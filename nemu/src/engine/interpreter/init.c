#include <cpu/cpu.h>

void sdb_mainloop();

void engine_start(char *argv[]) {
#ifdef CONFIG_TARGET_AM
  cpu_exec(-1);
#else
  /* Receive commands from user. */
  sdb_mainloop(argv);
#endif
}
