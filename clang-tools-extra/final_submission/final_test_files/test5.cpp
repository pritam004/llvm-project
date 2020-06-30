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
