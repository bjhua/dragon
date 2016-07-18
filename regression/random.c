#include <stdlib.h>

int main ()
{
  int i;
  for (i=0; i<100; i++){
    //    srand (clock());
    printf ("%d\n", rand ());
  }
  return 0;
}
