#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define NR_TOKEN 500

// this should be enough
static char buf[65536] = {};
static int  buf_loc = 0;     
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned int result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";


int nt = 0;

void gen_rand_expr();

uint32_t choose(uint32_t n)
{
  return(rand()%n);
}


void gen_num()
{
  unsigned int num = rand()%1000;
  char str[20];
  int bios;
  if(nt >= NR_TOKEN)
     return;
  sprintf(str,"%d",num);
  sprintf(buf+buf_loc,"%d",num);
  bios = strlen(str);
  buf_loc = buf_loc + bios;
  ++ nt;
}


void gen(char para)
{
  if(nt >= NR_TOKEN && para != ')')
      return;
  sprintf(buf+buf_loc,"%c",para);
  buf_loc = buf_loc + 1;
  ++ nt;
}

void gen_rand_op()
{

  if(nt >= NR_TOKEN)
      return;
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
  }
  buf_loc = buf_loc + 1;
  ++ nt;
}


void gen_rand_expr() //get the value inside 
{ 
 if(nt >= NR_TOKEN)
     return;
 switch (choose(3)) {
    case 0: gen_num();
            break;
    case 1: 
            if(nt + 3 > NR_TOKEN)
                return;
            gen('(');
            gen_rand_expr();
            gen(')');
            break;
    default:
            if(nt + 3 > NR_TOKEN)
                return;
            gen_rand_expr(); 
            gen_rand_op(); 
            gen_rand_expr();
             break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 0;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {

    nt = 0;
    buf_loc = 0;

    gen_rand_expr();
  
   
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Werror /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);
    if(result <0)
    {
        i ++;
        memset(buf,0,strlen(buf));
        buf_loc = 0;
        continue;
    }
    printf("%u %s\n", result, buf);
  
    memset(buf,0,strlen(buf));

   
  }
  
  return 0;
}
