/*
 * file: pls_fix.c
 * auth: Billy Mallard
 * mail: wjm@berkeley.edu
 * date: 11-Jun-2008
 */

#include <stdio.h>
#include <stdlib.h>

#define BUF_LEN 128

int main(int argc, char **argv)
{
	/* validate arguments */
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s [infile] [outfile]\n", argv[0]);
		exit(1);
	}

	/* initialize fields */
	float dm = 0;
	int f2 = 0;
	int ts = 0;
	float f4 = 0;
	int f5 = 0;
	char *buf = (char *)calloc(BUF_LEN,sizeof(char));

	/* open files */
	FILE *infile = fopen(argv[1], "r");
	FILE *outfile = fopen(argv[2], "w");

	unsigned char skip_flag = 0;

	/* read line from infile */
	int ret = fscanf(infile, "%f %d %d %f %d\n", &dm, &f2, &ts, &f4, &f5);

	while (ret != -1)
	{
		/* handle matching lines */
		if (ret == 5)
		{
			/* fix data as needed */
			if (dm >= 329.0 && dm < 658.0)
			{
				ts *= 2;
			}
			else if (dm >= 658.0 && dm < 1314.0)
			{
				ts *= 4;
			}
			else if (dm >= 1314.0)
			{
				ts *= 8;
			}

			/* write line to outfile */
			fprintf(outfile, "%7.1f%5d%13d%10.2f%5d\n", dm, f2, ts, f4, f5);
		}
		/* handle non-matching lines */
		else if (ret == 0)
		{
			/* read raw line from infile */
			fgets(buf, BUF_LEN, infile);

			if (skip_flag == 0)
			{
				/* write raw line to outfile */
				fputs(buf, outfile);
			}
			else
			{
				/* do not write line to outfile */
				
				/* reset flag */
				skip_flag = 0;
			}
		}
		/* handle partially-matching lines */
		else
		{
			/* discard last partial line */
			
			/* set flag to skip rest of line */
			skip_flag = 1;
		}

		/* read line from infile */
		ret = fscanf(infile, "%f %d %d %f %d\n", &dm, &f2, &ts, &f4, &f5);
	}

	/* close files */
	fclose(infile);
	fclose(outfile);

	return 0;
}
