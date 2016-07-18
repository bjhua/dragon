int f (int[] a)
{
  int x = 9999;
  
  a[0] = a[9] = x;
  return 0;
}

int dragon ()
{
  int[] a = new int[10];
  int i;
  
  for (i=0; i<10; i=i+1){
    for (i = 1; i<10; i=i+1){
      a[i = i] = i;
    }
  }

  f (a);
  for (i=0; i<10; i=i+1){
    printi (a[i]);
    prints (" ");
  }
  return 0;
}
