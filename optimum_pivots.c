#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<omp.h>
#define X 4
#define N 100
#define B 100
/*
	In this version of the application, the pivot values ??have been chosen more optimally to reduce the running time of the program.
	I think this version works well in the scenario where the numbers are random.
*/
void swap(int *a,int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}


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
int partition(int arr[], int low, int high)
{
	int pivot = arr[high];
    int i = (low - 1);
 	int j;
    for (j = low; j < high ; j++)
    {
        if (arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i+1],&arr[high]);
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

int bestPivot(int min,int max,int partition,int number)
{
	return min + ((max-min)/partition*number);
}

int partition_with_particular_pivot(int arr[], int low, int high,int pivot)
{
    int i = (low - 1);
 	int j;
    for (j = low; j <= high ; j++)
    {
        if (arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    return i;
}

void divide_array(int array[],int len,int tsay,int info[],int step,int min,int max)
{
	
	int part_index,pivot;
	if(step==0)
	{
		pivot = bestPivot(min,max,2,1);
		part_index = partition_with_particular_pivot(array,0,len-1,pivot);
		step++;
		info[step] = part_index;
		divide_array(array,len,tsay,info,step,min,max);
	}
	else if(step==tsay-1)
	{
		return;
	}
	else
	{
		omp_set_dynamic(0);
		#pragma omp parallel num_threads(step+1) private(part_index,pivot)
		{
			int tid = omp_get_thread_num();
			int end = len-1;
			if(info[tid+1]!=-1)
				end = info[tid+1];
			int start = info[tid]+1;
			pivot = bestPivot(min,max,(step+1)*2,tid*2+1);
			part_index = partition_with_particular_pivot(array,start,end,pivot);
			#pragma omp barrier
				info[tid+step+1] = part_index;
		}
		step+=(step+1);
		quickSort(info,1,step);
		divide_array(array,len,tsay,info,step,min,max);	
	}
}


void test(int array[],int len)
{
	int i;
	for(i=0;i<len-1;i++)
	{
		if(array[i]>array[i+1])
		{
			printf("\n%d,%d,Sorting ERROR %d>%d\n",i,i+1,array[i],array[i+1]);
			return;
		}
	}
	printf("\nTest Passed.\n");
}

int main()
{
	srand(time(NULL));
	FILE *ptr;
   	int tsay,n,i,j,max=0;
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

   	for(i=0;i<n;i++)
	{
		fscanf(ptr,"%d",&array[i]);
		if(array[i]>max)
			max = array[i];
	}
	fclose(ptr);
	
   	if(tsay>32 || n>1000000)
   	{
   		printf("The maximum value of the first line (number of threads) in the input file is 32, and the maximum value of the second line (number of samples) is 1000000!!");
   		return -1;
  	}

	start = omp_get_wtime();
	for(i=0;i<=tsay;i++)
	{
		partition_info[i]=-1;
	}

	divide_array(array,n,tsay,partition_info,0,0,max);
	partition_info[tsay]=n-1;
	#pragma omp parallel num_threads(tsay)
	{
		int tid = omp_get_thread_num();
		quickSort(array,partition_info[tid]+1,partition_info[tid+1]);
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
