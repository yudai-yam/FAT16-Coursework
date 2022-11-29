#include <stdio.h>

int main(void)
{
  char * strs[1][3];   // Define an array of character pointers 1x3

  char *a = "string 1";
  char *b = "string 2";
  char *c = "string 3";

  strs[0][0] = a;
  strs[0][1] = b;
  strs[0][2] = c;

  printf("String in 0 1 is : %s\n", strs[0][1]);
  printf("String in 0 0 is : %s\n", strs[0][0]);
  printf("String in 0 2 is : %s\n", strs[0][2]);

  return 0;
}