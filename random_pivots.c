#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<omp.h>
void printArray(int arr[],int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		if(i%5==0)
			printf("\n");
		printf("%10d",arr[i]);
	}
}
void swap(int *a,int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}
int partition(int arr[], int low, int high)
{
    int pivot = arr[high];
    int i = (low - 1);
 	int j;
    for (j = low; j <= high ; j++)
    {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}


void quickSort(int arr[], int low, int high) 
{ 
    if (low < high) 
    { 
        int pi = partition(arr, low, high); 
        quickSort(arr, low, pi - 1); 
        quickSort(arr, pi + 1, high); 
    } 
} 


void divide_array(int *array,int len,int tsay,int info[],int len_info)
{
	/*
		It divides and sorts the sent thread according to the number of threads.
		
	*/
	
	int part_index,pivot_index;
	if(len_info==0)
	{
		pivot_index = (int)(((double) rand()/RAND_MAX)*(len-1));
		swap(&array[pivot_index],&array[len-1]);
		part_index = partition(array,0,len-1);
		len_info++;
		info[len_info] = part_index;
		divide_array(array,len,tsay,info,len_info);
	}
	else if(len_info==tsay-1)
	{
		return;
	}
	else
	{
		omp_set_dynamic(0);
		int random_seed = rand();
 		#pragma omp parallel num_threads(len_info+1) private(part_index,pivot_index)
		{
			int tid = omp_get_thread_num();
			srand(random_seed+tid+1);
			int end = len-1;
			if(info[tid+1]!=-1)
				end = info[tid+1]-1;
			int start = info[tid]+1;
			pivot_index = (int)(((double) rand()/RAND_MAX)*(end-start));
			pivot_index+=start;
			swap(&array[pivot_index],&array[end]);
			part_index = partition(array,start,end);
			#pragma omp barrier
				info[tid+len_info+1] = part_index;
		}
		len_info+=(len_info+1);
		quickSort(info,1,len_info);
		divide_array(array,len,tsay,info,len_info);	
	}
}

void test(int array[],int len)
{
	int i;
	for(i=0;i<len-1;i++)
	{
		if(array[i]>array[i+1])
		{
			printf("\nBetween %d. and %d. ERROR!\n X%d<%dX\n",i,i+1,array[i],array[i+1]);
			return;
		}
	}
	printf("\nTest Passed.\n");
}

int main()
{
	srand(time(NULL));
	FILE *ptr;
   	int tsay,n,i,j;
   	double start;
   	
	ptr = fopen("input.txt","r");
	if(ptr == NULL)
   	{
      	printf("Error!");   
	  	return -1;    
   	}

   	fscanf(ptr,"%d",&tsay);
   	int *partition_info = malloc(sizeof(int)*(tsay+1));
   	fscanf(ptr,"%d",&n);
   	int *array = malloc(sizeof(int)*n);

   	if(tsay>32 || n>1000000)
   	{
   		printf("The maximum value of the first line (number of threads) in the input file is 32, and the maximum value of the second line (number of samples) is 1000000!!");
   		return -1;
  	}

    	for(i=0;i<n;i++)
	{
		fscanf(ptr,"%d",&array[i]);
	}
	fclose(ptr);
	
	start = omp_get_wtime();
	for(i=0;i<=tsay;i++)
	{
		partition_info[i]=-1;
	}

	divide_array(array,n,tsay,partition_info,0);
	partition_info[tsay]=n;
	#pragma omp parallel num_threads(tsay)
	{
		/*
			With the updated partition info array, each thread sort 1 partition.		
		*/
		int tid = omp_get_thread_num();
		quickSort(array,partition_info[tid]+1,partition_info[tid+1]-1);
	}

	printf("\n%f seconds\n",omp_get_wtime()-start);
	test(array,n);
	printf("\n**************Partition Boundaries *************");
	printArray(partition_info,tsay+1);
	

	ptr = fopen("output.txt","w");
	if(ptr == NULL)
   	{
      	printf("Could not open output.txt file!!");   
	 	return -1;    
   	}
	fprintf(ptr,"%d\n",tsay);
	fprintf(ptr,"%d\n",n);

	for(i=0;i<n;i++)
	{
		fprintf(ptr,"%d\n",array[i]);
	}
	fclose(ptr);
	free(array);
	return 0;
}
