#include<stdio.h>

void foo(int *x,int *y)
	{
	
	printf("Value of x inside nested function foo = %d\n",*x);
	*x=4;
	*y=9;
	}
int main()
{
	int x=3;
	int *y=&x;
	/*foo is a nested function
	i.e. the below syntax is equivalent to :
	void foo(){
	printf("Value of x inside nested function foo = %d\n",x);
	x=4;
	}
	*/
	
	printf("Value of x before calling nested function foo = %d\n",x);/*shouldprint 3 after transformation*/
	foo(&x,y);
	printf("Value of x after calling nested function foo = %d\n",x);/*should print 4* after transformation*/
}
