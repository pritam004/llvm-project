#include<stdio.h>
void label1(int *a){
	       *a=2;
	       printf("\ninside label 1 %d",*a);


	}
void label2(int *a){
	       *a=3;
	       printf("\ninside label 2 %d",*a);


       }
void label3(int *a){
	       *a=4;
		printf("\ninside label 3 %d",*a);


       }
void label4(int *a){
	       *a=5;
	       printf("\ninside label 4 %d",*a);
       


	      
	}
void label5(int *a)


	       {
		       *a=6;
		       printf("\ninside label 5 %d",*a);
	       }
void fun()
{
	int a=1;
	printf("\nbefore labels 1 %d",a);

       label1(&a);
       label2(&a);
       label3(&a);
       label4(&a);
       label5(&a);
       printf("\noutside labels %d",a);

}

int main()
{
	fun();
	return 0;

}
