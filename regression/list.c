class list
{
  int data; 
  list next;
}

list List_create()
{
  return new list (1, new list (2, new list (3, new list (4, null))));
}

int List_print (list l)
{
  if (l == null)
    return 0;
    
  printi (l.data);
  prints (", ");
  List_print (l.next);
  return 0;
}

int dragon (int argc, string[] argv)
{
  list l = List_create ();
  List_print (l);
  return 0;
}
