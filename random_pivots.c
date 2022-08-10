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
		Gönderilen diziyi thread sayýsý kadar böler ve sýralar.
		Pivot rastgele seçiliyor. Fakat þöyle bir sorun var rand fonksiyonu [0,32767] arasýnda deðer üretiyor. 
		Ben her ne kadar (rand()/RAND_MAX)*1.000.000 yapmýþ olsamda buradan çýkacak farklý sonuç sayýsý 1.000.000 adet deðil, hala 32767 adet oluyor.
		Yani bu rastgelelik uniform olmayan, bizim istemedðimiz bir rastgelelik oluyor.
		Bunu çözmenin bir yolunu bulamadým.Ýnternette farklý rastgele sayý fonksiyonu buldum fakat koduma implemente edemedim.
		Pivotun rastgele seçilmesini kaldýrmak için 88. ve 112. kod satýrlarýný silebilirsiniz.
		Sýnýr deðerleri info adlý dizi içerisinde tutuluyor.
		Örneðin : 100 elemanlý bir dizi ve 4 threadimiz var.
		Diziyi 2ye bölüyor.Pivot olarak rastgele bir eleman alýp bunu dizi içerisinde kendinden küçük olanlarý sola alýcak þekilde konumlandýrýyor(44).
		Bu aþamadan sonra elimizde 2 parça var bunlar [0-43],[45,99]. Parametreler güncelleniyor(info,len_info).Method parametrelerini güncellenmiþ haliyle tekrar çaðrýlýyor.
		info dizisinin içeriði þu þekilde: [-1,44,-1,-1,-1] , len_info=1
		2.çaðýrmada 2 parça olan dizimizin her bir parçasý 2 parçaya ayrýlýyor.
		infonun güncellenmiþ içeriði : [-1,13,44,76,-1]->[0,12],[14,43],[45,75],[77,99],len_info = 3
		Eðer 8 adet threadimiz olsaydý bu iterasyon bir kere daha gerçekleþecekti.
		infonun güncellenmiþ içeriði : [-1,8,13,15,44,55,76,88,-1]->[0,7],[9,12]....  ,len_info = 7
		
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
			printf("\n%d. ile %d. arasinda siralama hatasi\n X%d<%dX\n",i,i+1,array[i],array[i+1]);
			return;
		}
	}
	printf("\nDogru siralama\n");
}

int main()
{
	srand(time(NULL));
	FILE *ptr;
   	int tsay,n,i,j;
   	double start;
   	
	ptr = fopen("girdi.txt","r");
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
   		printf("Girdi dosyasýndaki ilk satirin(thread sayisi) maksimum degeri 32, ikinci satirin(ornek sayisi) maksimum degeri 1000000dur!!");
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
			Güncellenen partition_info dizisi ile her thread 1 parçayý sýralýyor.		
		*/
		int tid = omp_get_thread_num();
		quickSort(array,partition_info[tid]+1,partition_info[tid+1]-1);
	}

	printf("\n%f saniye\n",omp_get_wtime()-start);
	test(array,n);
	printf("\n**************Partition Boundaries *************");
	printArray(partition_info,tsay+1);
	

	ptr = fopen("cikti.txt","w");
	if(ptr == NULL)
   	{
      	printf("Cikti.txt dosyasi acilamadi!!");   
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
