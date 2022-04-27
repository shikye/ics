#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
word_t expr(char *e, bool *success);

int main(int argc, char *argv[]) {
  int arg_n = 2;
  unsigned int comp1 = 0;
  unsigned int comp2 = 0;
  bool flag = 0;

  /* Initialize the monitor. */
//#ifdef CONFIG_TARGET_AM
//  am_init_monitor();
//#else
//  init_monitor(argc, argv);
//#endif

  while(argv[arg_n] != NULL)
  {
    comp1 = atoi(argv[arg_n]);
    arg_n ++;
    comp2 = expr(argv[arg_n],&flag);

    if(comp1 != comp2)
      printf("error!");
  }
  

  /* Start engine. */
//  engine_start();
//
//  return is_exit_status_bad();

return 0;
}
