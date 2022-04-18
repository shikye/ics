#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static int  buf_loc = 0;     
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

uint32_t choose(uint32_t n)
{
  return(rand()%n);
}

void gen_num()
{
  int num = rand()%100;
  char str[20];
  int bios;
  sprintf(str,"%d",num);
  sprintf(buf+buf_loc,"%d",num);
  bios = strlen(str);
  buf_loc = buf_loc + bios;
}

void gen(char para)
{
  sprintf(buf+buf_loc,"%c",para);
  buf_loc = buf_loc + 1;
}

void gen_rand_op()
{
  switch(choose(4))
  {
    case 0:
      sprintf(buf+buf_loc,"%c",'+');
      break;
    case 1:
      sprintf(buf+buf_loc,"%c",'-');
      break;
    case 2:
      sprintf(buf+buf_loc,"%c",'*');
      break;
    case 3:
      sprintf(buf+buf_loc,"%c",'/');
      break;
    default:sprintf(buf+buf_loc,"%c",'/');

  }
  buf_loc = buf_loc + 1;
}

void gen_rand_expr() {
  switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  buf_loc = 0;
  return 0;
}
