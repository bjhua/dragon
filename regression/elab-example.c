class x {x x; y y;}
class y = {x x; y y;}

int f ()
{
  int z = 1;
  printi (z);
  {
    int z = 2;
    printi (z);
  }
  g ();
  return 0;
}

int g ()
{
//  @ f ();
  return 0;
}

int dragon ()
{
  f ();
  return 0;
}



