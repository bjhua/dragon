int dragon (int argc, string[] argv)
{
  int i;
  
  for (i=0; i<argc; i=i+1){
    prints (argv[i]);
    prints ("\n");
  }
  
  return 0;
}
