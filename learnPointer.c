#include <stdio.h>

unsigned int_to_int(unsigned k) {
    if (k == 0) return 0;
    if (k == 1) return 1;                       /* optional */
    return (k % 2) + 10 * int_to_int(k / 2);
}

int main () {

   int  var = 20;   /* actual variable declaration */
   int  *ip = 0;        /* pointer variable declaration */

   printf("experiment: %i\n", ip);

   ip = ip + 1;
   printf("experiment2: %i\n", ip);


   ip = &var;  /* store address of var in pointer variable*/

   

   printf("Address of var variable: %x\n", &var  );

   /* address stored in pointer variable */
   printf("Address stored in ip variable: %x\n", ip );

   /* access the value using the pointer */
   printf("Value of *ip variable: %d\n", *ip );

   printf("the binary form of 17 is: %d\n", int_to_int(19));

   return 0;
}