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

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

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

static int cmd_c(char *args, char* arg_after[20]) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args, char* arg_after[20]) {
  return -1;
}

static int cmd_s(char *args, char* arg_after[20]) {
  if(args != NULL)
  {
    int i = atoi(args);
    cpu_exec(i);
  }
  else
    cpu_exec(1);
  return 0;
}

static int cmd_x(char *args, char* arg_after[20]) {
  return -1;
}

static int cmd_info(char *args, char* arg_after[20]) {
  if(!strcmp(args,"r"))
  {
    for(int i = 0; i < 32; i++)
      {
        printf("x[%d] is %u\n", i , cpu.gpr[i]._32);
      }
    printf("pc is %u\n",cpu.pc);
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

// static int cmd_x(char *args, char* arg_after[20]) {
  
//   word_t read[20];
//   int j = atoi(args);
//   int i = 0;
//   while(j --)
//   {
//     read[i] = pmem_read(atoi(arg_after[0]),1);
//     printf("%02x : %02x",atoi(arg_after[0])+i,read[i]);
//     i ++;
//   }
    
// }

static int cmd_help(char *args, char* arg_after[20]);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *,char* arg_after[20]);
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

static int cmd_help(char *args, char* arg_after[20]) {
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

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL,NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;   //cmd and str?
    char *arg_after[20] = {"ini"};
    int arg_cnt = 0;
    if (args >= str_end)  args = NULL;
    else
    {
      // arg[arg_cnt] = strtok(NULL,str);
        // arg_cnt ++;
        // arg[arg_cnt] = strtok(NULL,str);
      args = strtok(NULL,str);
      if(args != NULL)
      {
        while ((arg_after[arg_cnt] = strtok(NULL,str)) != NULL)
        {
          arg_cnt ++;
        }
      }
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
