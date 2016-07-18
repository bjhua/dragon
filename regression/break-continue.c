int dragon ()
{
	int i=0;

  prints ("break\n");
  
	while (i<10){
	  printi (i);
	  prints ("\n");
	  if (i==5)
	    break;
	  i = i+1;
	}
	
	prints ("continue\n");
	
	for (i=0; i<10; i=i+1){
	  if (i%3!=0)
	    continue;
	  printi (i);
	  prints ("\n");
  }
  
	return 0;
}
