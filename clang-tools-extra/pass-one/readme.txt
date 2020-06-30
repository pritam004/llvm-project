##steps to execute the programs


##pre-requisites

1.clang 11(install clang tools and clang tools extra) also compatible with llvm 9.0.1 ,as instructed
2.make
3.Cmake
4.Ninja


##description about files

1.There are three folders named pass-one,pass-two and pass-three respectively.
2.These folders contain a cpp file and a CmakeList.
3.Copy these three folders into clang-tools-extra folder
4.Update the CmakeList in the clang-tools-extra folder with the following

addsubdirectory(pass-one)
addsubdirectory(pass-two)
addsubdirectory(pass-three)

5.In the build folder of clang  (for instructions on installing clang please visit https://clang.llvm.org/docs/LibASTMatchersTutorial.html)
open terminal and type the following commands

ninja pass-one
ninja pass-two
ninja pass-three

or equivalently

make the three passes

[few warnings will be shown ignore those]

I have done the project using ninja but make can also be used to do the same

6.There is a folder named final_test_files containg some sample test cases.
Make sure to give the accurate path to the bin directory while running.
Explanation about the test cases are provided in this file case wise.

7.Makefile in final_test_files

This contains the sequence of passes and few cleanup statements.

to execute this file:

make -i file=[path_to_bin]


[ some errors and warnings will be displayed but the final transformed code and object code will be produced ]


On executing this the final resulting source code is stored in result[i].cpp and result[i].o is the compiled binary
which can be executed as ./result[i].o

**Here i refers to the testcase no eg result2.cpp and result2.o**
the output can be seen by running ./result[i].o

8.To run a specific test case that is not in the file i have provided

make -f singleprog file=[path to the file to be executed] bin=[path to bin folder] -i

**singleprog file is in final_test_cases folder



9.Few temporary files will be created namely
 data_var.txt [list of labels and var names]
 vnames.txt
 vcnames.txt
 res.cpp[result after first pass]
 res1.cpp[result after second pass]
that will be removed by the make file.



for further clarifications mail me @ pritamnath@iisc.ac.in


#################################################################################################################################
                                                        TEST CASES

#################################################################################################################################


TEST CASE 1:

QUESTION : First test should be the code that was sent along with the assignment.


CODE:


#include<stdio.h>

int main()
{
        int x=3;
        /*foo is a nested function
        i.e. the below syntax is equivalent to :
        void foo(){
        printf("Value of x inside nested function foo = %d\n",x);
        x=4;
        }
        */
        foo:
        {

        printf("Value of x inside nested function foo = %d\n",x);
        x=4;
        }
        printf("Value of x before calling nested function foo = %d\n",x);/*shouldprint 3 after transformation*/
        foo();
        printf("Value of x after calling nested function foo = %d\n",x);/*should print 4* after transformation*/
}


FINAL CODE AFTER TRANSFORMATION :


#include<stdio.h>

void foo(int *x)
	{

	printf("Value of x inside nested function foo = %d\n",*x);
	*x=4;
	}
int main()
{
	int x=3;
	/*foo is a nested function
	i.e. the below syntax is equivalent to :
	void foo(){
	printf("Value of x inside nested function foo = %d\n",x);
	x=4;
	}
	*/

	printf("Value of x before calling nested function foo = %d\n",x);/*shouldprint 3 after transformation*/
	foo(&x);
	printf("Value of x after calling nested function foo = %d\n",x);/*should print 4* after transformation*/
}


OUTPUT:

Value of x before calling nested function foo = 3
Value of x inside nested function foo = 3
Value of x after calling nested function foo = 4



~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


TEST CASE 2 :

QUESTION:In addition, does your implementation handle recursively nested function blocks?

ANSWER: The code can handle any level of nesting.The following code shows a level5 nesting.

CODE:

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


FINAL TRANSFORMED CODE :

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

OUTPUT :

before labels 1 1
inside label 1 2
inside label 2 3
inside label 3 4
inside label 4 5
inside label 5 6
outside labels 6

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TEST CASE 3:


QUESTION:Does your asst handle structs that are locally defined (i.e. defined inside the function)?

ANSWER: Yes, It does handle local structs.It hoists the structure outside the func.

CODE:

int main()
{
struct hello
{
        int a;
        int b;
};

        struct hello s;
        //int a;

label:{
                s.a=3;
                s.b=5;




        }
        label();
}


FINAL TRANSFORMED CODE:

struct hello
{
        int a;
        int b;
};
void label(struct hello  *s){
		s->a=3;
		s->b=5;




	}
int main()
{


	struct hello s;
	//int a;


	label(&s);
}


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TEST CASE 4:

QUESTION:If I have a local captured variable (i.e. the var is defined inside the
function and used inside the closure) and a global var with same name, will your code still work?

ANSWER: Yes it works.

CODE:

#include<stdio.h>


int x=1;

int main()
{

        printf("this is the global variable = %d \n",x);
        int x=3;
        /*foo is a nested function
        i.e. the below syntax is equivalent to :
        void foo(){
        printf("Value of x inside nested function foo = %d\n",x);
        x=4;
        }
        */
        foo:
        {

        printf("Value of x inside nested function foo = %d\n",x);
        x=4;
        }
        printf("Value of x before calling nested function foo = %d\n",x);/*shouldprint 3 after transformation*/
        foo();
        printf("Value of x after calling nested function foo = %d\n",x);/*should print 4* after transformation*/
}


FINAL TRANSFORMED CODE:

#include<stdio.h>


int x=1;

void foo(int *x)
	{

	printf("Value of x inside nested function foo = %d\n",*x);
	*x=4;
	}
int main()
{

	printf("this is the global variable = %d \n",x);
	int x=3;
	/*foo is a nested function
	i.e. the below syntax is equivalent to :
	void foo(){
	printf("Value of x inside nested function foo = %d\n",x);
	x=4;
	}
	*/

	printf("Value of x before calling nested function foo = %d\n",x);/*shouldprint 3 after transformation*/
	foo(&x);
	printf("Value of x after calling nested function foo = %d\n",x);/*should print 4* after transformation*/
}


OUTPUT :

this is the global variable = 1
Value of x before calling nested function foo = 3
Value of x inside nested function foo = 3
Value of x after calling nested function foo = 4


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TEST CASE 5:

QUESTION:Do you pass all the variables to every closure or only the captured vars?

Can you handle multiple closures in same/different functions (not necessarily nested)?

ANSWER: only the captured variables are passed.

The other functionality is also captured in this example.
CODE :

void func()
{
        int a=0;

        int b[10];
label1:{
               a+=1;
               b[5]=3;
       }

label2:
       {
               a=1;
       }

       label1();
       label2();


}

int main()
{
}


FINAL TRANSFORMED CODE:

void label1(int *a,int *b){
	       *a+=1;
	       b[5]=3;
       }
void label2(int *a)
       {
	       *a=1;
       }
void func()
{
	int a=0;

	int b[10];




       label1(&a,b);
       label2(&a);


}

int main()
{
}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TEST CASE 6:
QUESTION:Do you explicitly pass global vars to the closure as well? Note that you don't need that.

ANSWER:  No, global vars are not passed.
CODE:


#include<stdio.h>


int y=1;

int main()
{

        printf("this is the global variable = %d \n",y);
        int x=3;
        /*foo is a nested function
        i.e. the below syntax is equivalent to :
        void foo(){
        printf("Value of x inside nested function foo = %d\n",x);
        x=4;
        }
        */
        foo:
        {

        printf("Value of x inside nested function foo = %d\n",x);
        x=4;
        printf("Value of y inside nested func =%d\n",y);

        }
        printf("Value of x before calling nested function foo = %d\n",x);/*shouldprint 3 after transformation*/
        foo();
        printf("Value of x after calling nested function foo = %d\n",x);/*should print 4* after transformation*/
}


FINAL TRANSFORMED CODE :

#include<stdio.h>


int y=1;

void foo(int *x)
	{

	printf("Value of x inside nested function foo = %d\n",*x);
	*x=4;
	printf("Value of y inside nested func =%d\n",y);

	}
int main()
{

	printf("this is the global variable = %d \n",y);
	int x=3;
	/*foo is a nested function
	i.e. the below syntax is equivalent to :
	void foo(){
	printf("Value of x inside nested function foo = %d\n",x);
	x=4;
	}
	*/

	printf("Value of x before calling nested function foo = %d\n",x);/*shouldprint 3 after transformation*/
	foo(&x);
	printf("Value of x after calling nested function foo = %d\n",x);/*should print 4* after transformation*/
}



OUTPUT :

this is the global variable = 1
Value of x before calling nested function foo = 3
Value of x inside nested function foo = 3
Value of y inside nested func =1
Value of x after calling nested function foo = 4



~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TEST CASE 7:

QUESTION:In addition, if your implementation has any extra features, please  mention these and add tests for them.

ANSWER:  My code can handle pointer type variables. It can handle all data types.

CODE :
#include<stdio.h>

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
	foo:
	{

	printf("Value of x inside nested function foo = %d\n",x);
	x=4;
	*y=9;
	}
	printf("Value of x before calling nested function foo = %d\n",x);/*shouldprint 3 after transformation*/
	foo();
	printf("Value of x after calling nested function foo = %d\n",x);/*should print 4* after transformation*/
}





FINAL TRANSFORMED CODE :

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


OUTPUT:

Value of x before calling nested function foo = 3
Value of x inside nested function foo = 3
Value of x after calling nested function foo = 9
