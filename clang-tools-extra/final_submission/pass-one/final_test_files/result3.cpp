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


