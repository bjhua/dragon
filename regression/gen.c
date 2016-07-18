#include <stdio.h>

int main (int argc, char **argv)
{
  int i, numStms;

  if (argc<2){
    fprintf (stderr, "Usage: gen <numStms>\n");
    return;
  }
  printf ("// Automatically generated. Don't modify.\n"
          "int dragon (int argc, string[] argv)\n"
          "{\n"
          "  int i = 0;\n"
          "  int r;\n");
  numStms = atoi (argv[1]);
  for (i=0; i<numStms; i++)
    printf ("  i = i+1;\n");
  printf ("  r = printi (i);\n"
          "  return i;\n}\n\n");
  return 0;
}
