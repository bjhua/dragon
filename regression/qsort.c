// This implementation is based on "Introduction to algorithm" 
// (second edition), chap.8.

int Array_print (int[] array, int left, int right)
{
  int i;

  for (i=left; i<right; i=i+1){
    printi (array[i]);
    prints (", ");
  }
  return 0;
}

int partition (int[] array, int left, int right)
{
  int pivot = array[left];
  int i = left-1;
  int j = right+1;
  int temp;
  
  while (1){
    do {
      j = j-1;
    } while (array[j]>pivot);
    do {
      i = i+1;
    } while (array[i]<pivot);
    if (i<j){
      temp = array[i];
      array[i]= array[j];
      array[j]=temp;
    }
    else return j;
  }
  return 0;
}

int qsort (int[] a, int left, int right)
{
  int index;
  
  if (left>=right)
    return 0;
    
  
  index = partition (a, left, right);
  qsort (a, left, index);
  qsort (a, index+1, right);
  return 0;
}

int dragon ()
{
  int i;
  int size = 10;
  
	int[] array = new int[size];
	
	
	for (i=0; i<size; i=i+1)
	  array[i] = size-1-i;
	
	prints ("\ninitial array:\n");
	Array_print (array, 0, size);
	
	
	prints ("\nquick sorting starts:\n");
	qsort (array, 0, size-1);
	prints ("\nthe sorted array is :\n");
	Array_print (array, 0, size);
	
  return 0;
}
