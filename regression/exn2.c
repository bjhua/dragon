int dragon ()
{
  int x;
  x = x;
  x = 99;
  printi (x);
  try {
    prints ("before\n");
    f ();
    prints ("after\n");
  }
  catch {
    prints ("caught\n");
  }

  prints ("after try\n");
  return 0;
}

int f ()
{
  return g ();
}

int g ()
{
  throw;
  return 0;
}

