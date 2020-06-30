#include<stdio.h>
void fun()
{
	int a=1;
	printf("\nbefore labels 1 %d",a);
label1:{
	       a=2;
	       printf("\ninside label 1 %d",a);

label2:{
	       a=3;
	       printf("\ninside label 2 %d",a);

label3:{
	       a=4;
		printf("\ninside label 3 %d",a);

label4:{
	       a=5;
	       printf("\ninside label 4 %d",a);
label5:


	       {
		       a=6;
		       printf("\ninside label 5 %d",a);
	       }       


	      
	}
       }
       }
	}
       label1();
       label2();
       label3();
       label4();
       label5();
       printf("\noutside labels %d",a);

}

int main()
{
	fun();
	return 0;

}
