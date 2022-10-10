/*
 * file: pls_skim.c
 * auth: Billy Mallard
 * mail: wjm@berkeley.edu
 * date: 19-Jun-2008
 */

#include <stdio.h>
#include <stdlib.h>

#define BUF_LEN 128
#define NOISE_FLOOR 75.0
#define MAX_AVG_SIGMA 7.5
#define MIN_SIGMA 8.5

int main(int argc, char **argv)
{
	/* validate arguments */
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s [infile]\n", argv[0]);
		exit(1);
	}

	/* initialize vars */
	double sigma_tot = 0;
	int sigma_cnt = 0;
	double sigma_avg = 0;

	/* initialize fields */
	float dm = 0; // dispersion measure
	int sl = 0;   // smoothing level
	int ts = 0;   // time stamp
	float sg = 0; // sigma
	int f5 = 0;   // mystery!
	float rfi = 0; // rfi marker
	const int num_fields = 6;
	
	/* initialize misc */
	FILE *infile = NULL;
	int ret = 0;
	char *buf = (char *)calloc(BUF_LEN,sizeof(char));

	/*
	 * CALCULATE AVERAGE SIGMA
	 */
	
	/* open file */
	infile = fopen(argv[1], "r");

	/* read line from infile */
	ret = fscanf(infile, "%f %d %d %f %d %f\n", &dm, &sl, &ts, &sg, &f5, &rfi);

	while (ret != -1)
	{
		/* handle matching lines */
		if (ret == num_fields)
		{
			if (rfi == 0.0)
			{
				if (dm > NOISE_FLOOR)
				{
					sigma_tot += sg;
					sigma_cnt++;
				}
			}
		}
		/* handle non-matching lines */
		else if (ret == 0)
		{
			/* discard entire last line */
			fgets(buf, BUF_LEN, infile);
		}
		/* handle partially-matching lines */
		else
		{
			/* discard last partial line */
		}

		/* read line from infile */
		ret = fscanf(infile, "%f %d %d %f %d %f\n", &dm, &sl, &ts, &sg, &f5, &rfi);
	}

	/* close file */
	fclose(infile);

	/*
	 * SCAN FOR ANOMALIES
	 */
	
	sigma_avg = sigma_tot / sigma_cnt;
	if (sigma_avg < MAX_AVG_SIGMA)
	{
		/* re-open file */
		infile = fopen(argv[1], "r");

		/* read line from infile */
		ret = fscanf(infile, "%f %d %d %f %d %f\n", &dm, &sl, &ts, &sg, &f5, &rfi);

		while (ret != -1)
		{
			/* handle matching lines */
			if (ret == num_fields)
			{
				if (rfi == 0.0)
				{
					if (dm > NOISE_FLOOR)
					{
						if (sg > MIN_SIGMA)
						{
							printf("%7.1f%5d%13d%10.2f%5d%8.1f\n", dm, sl, ts, sg, f5, rfi);
						}
					}
				}
			}
			/* handle non-matching lines */
			else if (ret == 0)
			{
				/* discard entire last line */
				fgets(buf, BUF_LEN, infile);
			}
			/* handle partially-matching lines */
			else
			{
				/* discard last partial line */
			}

			/* read line from infile */
			ret = fscanf(infile, "%f %d %d %f %d %f\n", &dm, &sl, &ts, &sg, &f5, &rfi);
		}

		/* re-close file */
		fclose(infile);
	}
	
	return 0;
}
