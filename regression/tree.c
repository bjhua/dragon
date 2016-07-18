class Tree_t
{
  int data; 
  Tree_t left;
  Tree_t right;
}

//              2
//            /   \
//           1     3
Tree_t Tree_cook ()
{
  return new Tree_t(2,
                    new Tree_t (1,
                                null,
                                null),
                    new Tree_t (3,
                                null,
                                null));
}

int Tree_preorder (Tree_t x)
{
  if (x==null)
    return 0;
  
  printi (x.data);
  prints (" ");
  Tree_preorder (x.left);
  Tree_preorder (x.right);
  return 0;
}

int dragon ()
{
  Tree_t tree = Tree_cook ();
  
  Tree_preorder (tree);
 
  return 0;
}
