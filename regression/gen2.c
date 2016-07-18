#include <stdio.h>
#include <time.h>

int main (int argc, char **argv)
{
  int i, numDecs;

  if (argc<2){
    fprintf (stderr, "Usage: gen2 <numDecs>\n");
    return;
  }
  printf ("// Automatically generated!. Don't modify.\n"
          "int dragon (int argc, string[] argv)\n"
          "{\n");
  numDecs = atoi (argv[1]);
  for (i=0; i<numDecs; i++)
    printf ("  int x%d%d%d = 1;\n", rand (), i, i+1, clock());
    
  // just want to veriry the output
  printf ("  printi (999);\n"
          "  return 0;\n}\n\n");
  return 0;
}
