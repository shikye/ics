#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start(char *argv[]);
int is_exit_status_bad();
word_t expr(char *e, bool *success);

int main(int argc, char *argv[]) {

  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif


  engine_start(argv);

  return is_exit_status_bad();

//printf("test %d times\n",cnt);

return 0;
}
