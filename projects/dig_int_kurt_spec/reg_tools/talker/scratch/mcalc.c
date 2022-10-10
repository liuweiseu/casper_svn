#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int rnd(float x);

int main(int argc,char **argv)
{
    if(argv[1] ==NULL)
    {
	printf("Usage: mcalc <value1> <operator: +,-,x,/,^> <value2>\n");
    }
    else
    {

	float val1 = atof(argv[1]);
	float val2 = atof(argv[3]);
	char *operator = argv[2];

	if(!strcmp(operator,"x"))
	{
	    if(argv[4] == NULL)
	    {
		printf("%f\n",val1*val2);
	    }
	    else
	    {
		printf("%d\n",rnd(val1*val2));
	    }
	}
   

	if(!strcmp(operator,"/"))
	{
	    if(argv[4] == NULL)
	    {
		printf("%f\n",val1/val2);
	    }
	    else
	    {
		printf("%d\n",rnd(val1/val2));
	    }
	}

	if(!strcmp(operator,"+"))
	{
	    if(argv[4] == NULL)
	    {
		printf("%f\n",val1+val2);
	    }
	    else
	    {
		printf("%d\n",rnd(val1+val2));
	    }
	}
	if(!strcmp(operator,"-"))
	{
	    if(argv[4] == NULL)
	    {
		printf("%f\n",val1-val2);
	    }
	    else
	    {
		printf("%d\n",rnd(val1-val2));
	    }
	}

	if(!strcmp(operator,"^"))
	{
	    printf("%f\n",pow((double)val1,(double)val2));
	}

	}
	return 0;
}

int rnd(float x)
{
    int y;

    if(x > 0)
    {
	y=(int)(x+.5f);
    }
    else
    {
	y=(int)(x-.5f);
    }

    return y;
}



