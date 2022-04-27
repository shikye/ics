#include <isa.h>
#include <cpu/cpu.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include<regex.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include "sdb.h"
#include <memory/host.h>
#include <memory/paddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

char* find_strings(char* str) //eof is the end , use a array to store the return value
{
    static int i = 0;
    static char arg[10];
    int  cnt = 0;
    memset(arg,0,sizeof(arg));
    if(str[i] != '\0')
    {
        while(str[i] != '\0')
        {
            if(str[i] != 32)
            {
                arg[cnt] = str[i];       
                cnt ++;
                i++;
            }
            else
            {
                arg[cnt] = '\0';
                i++;
                cnt = 0;
                return arg;
            }       
        }
        arg[cnt] = '\0';
        return arg;
    }
    else
    {
        i = 0;
        return "eof";
    }
}


int regx(const char *pattern, char *input)
{
    regex_t preg;
    int cflags = REG_EXTENDED;
    size_t nmatch = 1;
    regmatch_t pmatch[1];
    int eflags = 0;
    int status = 0;
    char reserve[10] ;
    int number = 0;

    regcomp(&preg,pattern,cflags);
    status = regexec(&preg,input,nmatch,pmatch,eflags);

    regfree(&preg);
    if(status == 0)
        {
            memcpy(reserve,input + pmatch[0].rm_so,pmatch[0].rm_eo - pmatch[0].rm_so);   
            number = atoi(reserve);
            return number;
        }
    else
        return -1;
}



/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args, char* arg_after) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args, char* arg_after) {
  return -1;
}

static int cmd_s(char *args, char* arg_after) {
  if(args != NULL)
  {
    int i = atoi(args);
    cpu_exec(i);
  }
  else
    cpu_exec(1);
  return 0;
}


static int cmd_info(char *args, char* arg_after) {
  if(!strcmp(args,"r"))
  {
    for(int i = 0; i < 32; i++)
      {
        printf("r[%d]\t is %u\n", i , cpu.gpr[i]._32);
      }
    printf("pc\t is %u\n",cpu.pc);
  }
  else if(!strcmp(args,"w"))
  {
  }
  else
  {
    printf("error in info\n");
  }

  return 0;
  
}

static int cmd_x(char *args, char* arg_after) {
  
  word_t read[20];
  int max = atoi(args);
  int j = 0;
  int i = 0;
  bool suc = true; 
  bool* flag = &suc;
  
  u_int pos = expr(arg_after,flag);
  if (*flag == false)
    return -1; //fail

  while(j < max)
  {
      read[i] = pmem_read(pos+4*j,4);
      printf("0x%08x : %08x\n",pos + 4*j, read[i]);
      i ++;
      j ++;
  }
  
  


  return 0;
    
}

static int cmd_help(char *args, char* arg_after);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *,char* arg_after);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step n Times", cmd_s},
  { "x", "get the neicun", cmd_x},
  { "info", "get r or w", cmd_info}
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args, char* arg_after) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop(char *argv[]) {
  if (is_batch_mode) {
    cmd_c(NULL,NULL);
    return;
  }
  

  char str[500];
  int str_len = 0;
  FILE *fp;
  fp = fopen("file_test","r");
  char *result;
  int res;
  char *expr_value;
  int exp;

  bool flag = 0;
  while(feof(fp) == 0){
    fgets(str,500,fp);

    if(feof(fp) != 0)
      break;

    str_len = strlen(str);
    *(str+str_len-1) = '\0';

    result = strtok(str," ");
    expr_value = strtok(NULL," ");

    res = atoi(result);
    exp = expr(expr_value,&flag);

    if(res == exp)
      printf("correct!\n");
    
  }


  fclose(fp);
  

  /* Start engine. */

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;   //cmd and str?
    char *arg_after = NULL;
    if (args >= str_end)  args = NULL;
    else
    {
      args = strtok(NULL," ");
      arg_after = strtok(NULL," ");
      if(arg_after != NULL)
        *(arg_after + strlen(arg_after)) = ' ';
    }
#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args,arg_after) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
