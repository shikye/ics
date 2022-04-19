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

void gen_rand_expr();
//void gen_rand_expr_nozero();

uint32_t choose(uint32_t n)
{
  return(rand()%n);
}

void gen_num_nozero()
{
  unsigned int num;
  char str[20];
  int bios;
  while((num = rand()%100) == 0);
  sprintf(str,"%u",num);
  sprintf(buf+buf_loc,"%u",num);
  bios = strlen(str);
  buf_loc = buf_loc + bios;
}

void gen_num()
{
  unsigned int num = rand()%1000;
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
  }
  buf_loc = buf_loc + 1;
}

int flag =  0;

void gen_rand_expr() //get the value inside 
{ 
 char* loc = buf + buf_loc; 
 char str[30];
 char* ptr;
 unsigned int r1;
 unsigned int r2;
 switch (choose(3)) {
    case 0: gen_num();
            break;
    case 1: gen('(');
            gen_rand_expr();
            gen(')');
            break;
    default: gen_rand_op(); 
             break;
  }
}


/*void gen_rand_expr_nozero() 
{  
 switch (choose(3)) {
    case 0: gen_num_nozero(); break;
    case 1: gen('('); gen_rand_expr_nozero(); gen(')'); break;
    default: gen_rand_expr_nozero(); gen_rand_op(); 
             gen_rand_expr_nozero();
             break;
  }
}
*/
static int check_zero_loc = 0;
/*int check_zero(char* ptr) // div 0 return -1;
{
    char* begin = ptr + 1;
    char* end;
    char* loc = begin;



    
}
*/
int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 0;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    char * loc = buf + buf_loc;
    char * begin;
    int    kuohao = 0;
    while(loc >= buf)
    {
        if(*loc == '/')
        {
            begin = loc + 1;
            if(*begin == '0')
                flag = 1;
            if(*begin == '(')
            {    
                kuohao ++;
                begin ++;



            }


        }

        loc --;
    }

 
    buf_loc = 0;
  
   
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
    if(result <0)
    {
        i ++;
        memset(buf,0,strlen(buf));
        continue;
    }
    printf("%u %s\n", result, buf);
  
    memset(buf,0,strlen(buf));
  }
  
  return 0;
}
