int dragon ()
{
  try{
    prints ("ready to throw\n");
    throw;
    prints ("after thrown\n");
  }
  catch{
    prints ("caught\n");
  }
  prints ("end\n");
  return 0;
}
