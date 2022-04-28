#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_IMED_HEX, TK_IMED_DEC,
  TK_LEFT_BRA, TK_RIGHT_BRA      

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", TK_LEFT_BRA},
  {"\\)", TK_RIGHT_BRA},
  {"0x[0-9]{1,8}", TK_IMED_HEX },     //HEX 0x___
  {"[^0x][0-9]+", TK_IMED_DEC},  //DEC nn     //zhengzaibiaodashi youwenti
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("%d,regex compilation failed: %s\n%s", i,error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65535] __attribute__((used)) = {};
static int token_numbers = 0;
static int nr_token __attribute__((used))  = 0;

bool check_parenttheses(int p, int q)
{
  int cnt = 1;
  int position = q - 1;
  int flag = position - p;
  if(tokens[p].type == '(' && tokens[q].type == ')')
  {
    while(flag)
    {
      if(tokens[position].type == '(')
        cnt --;
      else if(tokens[position].type == ')')
        cnt ++;
      
      if(cnt <= 0)
        return false;
      
      flag --;
    }

    if(cnt == 1)
      return true;
    else
      return false;
  }
  else
    return false;
}

word_t eval(int p , int q)
{
  int position = 0;
  int bra_cnt = 0;
  char *ptr;

  if( p > q )
    return -1; //fail
  else if(p == q){
    if(tokens[p].type ==  TK_IMED_HEX)
      return strtol(tokens[p].str,&ptr,16);
    else
      return atoi(tokens[p].str);
  }

  if(check_parenttheses(p,q) == true){
    return eval(p+1,q-1);
  }
  
  if(tokens[q].type == '(')
  {
    return -1;  //fail
  }



  position = q;

  while(position)
  {

    if(tokens[position].type == ')')    //find the match '('
    {
      
      bra_cnt = 1;
      while(position)
      {
        if(tokens[position].type == ')')
        {
          bra_cnt ++;
        }
        if(tokens[position].type == '(')
        {
          bra_cnt --;
          if(bra_cnt == 0)
          {
            break;
          }
        }
        position --;
      }
    }

    
    if(tokens[position].type == '+')
      return eval(p,position-1)+eval(position+1,q);
    else if(tokens[position].type == '-')
      return eval(p,position-1)-eval(position+1,q);
    else if(tokens[position].type == '*')
      return eval(p,position-1)*eval(position+1,q);
    else if(tokens[position].type == '/')
      return eval(p,position-1)/eval(position+1,q);



    position --;
  }



  return -1; //faill
}



static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
case TK_NOTYPE: ; break;
case '+':tokens[token_numbers].type='+';token_numbers++; break;
case TK_IMED_HEX:tokens[token_numbers].type=TK_IMED_HEX;strncpy(tokens[token_numbers].str,substr_start + 2 ,pmatch.rm_eo - pmatch.rm_so - 2);token_numbers++; break;
case TK_IMED_DEC:tokens[token_numbers].type=TK_IMED_DEC;strncpy(tokens[token_numbers].str,substr_start,pmatch.rm_eo - pmatch.rm_so);token_numbers++; break;
case '-':tokens[token_numbers].type='-';token_numbers++; break;
case '*':tokens[token_numbers].type='*';token_numbers++; break;
case '/':tokens[token_numbers].type='/';token_numbers++; break;
case TK_LEFT_BRA:tokens[token_numbers].type='(';token_numbers++ ; break;
case TK_RIGHT_BRA:tokens[token_numbers].type=')';token_numbers++ ; break;


            default: ;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  int front = 0, rear = token_numbers - 1;
  word_t result = eval(front,rear);

  token_numbers = 0;
  return result;
}




