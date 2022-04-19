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

unsigned int  gen_rand_expr();
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

unsigned int gen_rand_expr() //get the value inside ()
{ 
 char* loc = buf + buf_loc; 
 char str[30];
 char* ptr;
 unsigned int r1;
 unsigned int r2;
 switch (choose(3)) {
    case 0: gen_num();
            strncpy(str,loc,buf + buf_loc - loc);
            r1 = strtoul(str,&ptr,10); 
            return r1; 
            break;
    case 1: gen('(');
            gen_rand_expr();
            gen(')'); strncpy(str,loc+1,buf+buf_loc-loc-1);
            r1 =  strtoul(str,&ptr,10);
            return r1;
            break;
    default: r1 = gen_rand_expr();
             loc = buf + buf_loc ;
             gen_rand_op(); 
             r2 = gen_rand_expr();
             switch (*loc) {
                case '+': return r1 + r2;
                case '-': if(r1 == r2 || r1 < r2) return -1; else  return r1 - r2;
                case '*': return r1 * r2;
                case '/': if(r1 >= r2)return r1 / r2; else return -1;
             }
                  
             break;
  }

 return -1;
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
  unsigned int check = 0;
  for (i = 0; i < loop; i ++) {
    check = gen_rand_expr();
    /*char * check_point = 0;
    check_point = buf + buf_loc;
    int check_result = 0;
    while(check_point >0)
    {
        if(*check_point == '/')
        {
            check_result = check_zero(check_point);
            
        }


        check_point --;

    }
    */     


    buf_loc = 0;
    if(check == -1)
    {
        i ++;
        memset(buf,0,strlen(buf));
        continue;
    }
    //int check = check_zero();
    //check_zero_loc = 0;
    //if(check == -1)
    //{
    //    i ++;
    //    memset(buf,0,strlen(buf));
    //    continue;
    // }
    



    
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
