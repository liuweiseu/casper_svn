/* start read.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "c2m.h"

#define CR 13            /* Decimal code of Carriage Return char */
#define LF 10            /* Decimal code of Line Feed char */
#define EOF_MARKER 26    /* Decimal code of DOS end-of-file marker */
#define MAX_REC_LEN 1024 /* Maximum size of input buffer */

/* Read functions */
int T_fread(FILE *InputFile);

/* Output functions */
void PrintLine(
                char *TextReadFromFile,
                long  CurrentLineNumber,
                long  LengthOfLine,
                long  OffsetOfStartOfCurrentLine,
                long *OffsetOfEndOfPreviousLine,
                int   OffsetError
              );


/******************************************************************************/
//CorrConf FileLoader(char *argv) /* Simple wrapper to load a file to be read*/
int FileLoader(char *argv) /* Simple wrapper to load a file to be read*/
/******************************************************************************/
{
  long  lFileLen;                /* Length of file */
  //CorrConf corr_config;

  FILE *inputFilePtr;            /* Pointer to input file */
  
  inputFilePtr = fopen(argv, "r");  /* Open in TEXT mode */

  if (inputFilePtr == NULL )             /* Could not open file */
  {
    printf("Error opening %s: %s (%u)\n", argv, strerror(errno), errno);
    return 0;
  }

  fseek(inputFilePtr, 0L, SEEK_END);     /* Position to end of file */
  lFileLen = ftell(inputFilePtr);        /* Get file length */
  rewind(inputFilePtr);                  /* Back to start of file */

  //corr_config = T_fread(inputFilePtr); /* Read the file and print output */
  T_fread(inputFilePtr); /* Read the file and print output */

  fclose(inputFilePtr); /* Close the file */
  return 1;             /* Exit with success code */
} /* end FileLoader() */

/******************************************************************************/
//CorrConf T_fread(FILE *input) /* Use:       Read text file using fread()           */
int T_fread(FILE *input) /* Use:       Read text file using fread()           */
                         /*                                                   */
                         /* Arguments: FILE *input                            */
                         /*              Pointer to input file                */
                         /*                                                   */
                         /* Return:    int                                    */
                         /*              0 = error                            */
                         /*              1 = success                          */
/******************************************************************************/
{
  /*
  *  This function reads the ENTIRE FILE into a character array and
  *  then parses the array to determine the contents of each line.
  *  This is lightning-fast, but may not work for large files. (See the
  *  notes preceding the call to calloc() in this function.)
  *
  *  This routine combines the functionality of the main() and T_fgetc()
  *  functions in this program (although, unlike T_fgetc(), it parses
  *  the lines from memory rather than directly from disk). I wrote it
  *  this way so I could keep everything in one source file and easily
  *  share the output routines.
  *  
  *  As in the T_fgetc() function, this function will "collapse" any
  *  blank lines. This may not be appropriate in a real application.
  */

  int   isNewline;              /* Boolean indicating we've read a CR or LF */
  long  lFileLen;               /* Length of file */
  long  lIndex;                 /* Index into cThisLine array */
  long  lLineCount;             /* Current line number */
  long  lLineLen;               /* Current line length */
  long  lStartPos;              /* Offset of start of current line */
  long  lTotalChars;            /* Total characters read */
  char  cThisLine[MAX_REC_LEN]; /* Contents of current line */
  char *cFile;                  /* Dynamically allocated buffer (entire file) */
  char *cThisPtr;               /* Pointer to current position in cFile */
  
  //CorrConf corr_config = initCorrConf();
  char *parse_str, *last_parse_str;
  int i, cPos=0, cEq=0;

  fseek(input, 0L, SEEK_END);  /* Position to end of file */
  lFileLen = ftell(input);     /* Get file length */
  rewind(input);               /* Back to start of file */

  /*
  *  The next line attempts to reserve enough memory to read the
  *  entire file into memory (plus 1 byte for the null-terminator).
  *
  *  The program will simply quit if the memory isn't available.
  *  This normally won't happen on computers that use virtual
  *  memory (such as Windows PCs), but a real application should
  *  make provisions for reading the file in smaller blocks.
  *
  *  We could use malloc() to allocate the memory, but calloc()
  *  has the advantage of initializing all of the bits to 0, so
  *  we don't have to worry about adding the null-terminator
  *  (Essentially, every character initially IS a null-terminator).
  *
  *  Note that we don't call the free() function to release the
  *  memory allocated by calloc(). It should not be necessary in
  *  this case because cFile is a local variable and will be
  *  deallocated automatically when this function ends.
  */

  cFile = calloc(lFileLen + 1, sizeof(char));

  if(cFile == NULL )
  {
    printf("\nInsufficient memory to read file.\n");
    return 0;
  }

  printf("Reading config File...");
  fread(cFile, lFileLen, 1, input); /* Read the entire file into cFile */

  lLineCount  = 0L;
  lTotalChars = 0L;

  cThisPtr    = cFile;              /* Point to beginning of array */

  while (*cThisPtr)                 /* Read until reaching null char */
  {
    lIndex    = 0L;                 /* Reset counters and flags */
    isNewline = 0;
    lStartPos = lTotalChars;

    while (*cThisPtr)               /* Read until reaching null char */
    {
      if (!isNewline)               /* Haven't read a CR or LF yet */
      {
        if (*cThisPtr == CR || *cThisPtr == LF) /* This char IS a CR or LF */
          isNewline = 1;                        /* Set flag */
      }

      else if (*cThisPtr != CR && *cThisPtr != LF) /* Already found CR or LF */
        break;                                     /* Done with line */

      cThisLine[lIndex++] = *cThisPtr++; /* Add char to output and increment */
      ++lTotalChars;

    } /* end while (*cThisPtr) */

    cThisLine[lIndex] = '\0';     /* Terminate the string */
    ++lLineCount;                 /* Increment the line counter */
    lLineLen = strlen(cThisLine); /* Get length of line */

    /* Print the detail for this line */
    PrintLine(cThisLine, lLineCount, lLineLen, lStartPos, NULL, 0);
    
    if ((strncmp(cThisLine,"#",1)!=0) && (strncmp(cThisLine,"[",1)!=0)){
		
		parse_str = strtok(cThisLine, "\n");
		parse_str = strtok(parse_str, "=");
		while( parse_str != NULL) {
			//printf( "%s\n",parse_str);
			last_parse_str = parse_str;
			parse_str = strtok(NULL, "=");
			if (strncmp(last_parse_str,"serve",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				parse_str = strtok(parse_str, ",");
				do {
					corr_config.servers[i] = cat_str("",parse_str);
					i++;
				} while((parse_str = strtok(NULL, ","))!=NULL);
				corr_config.n_servers = i;
			} else if (strncmp(last_parse_str,"times",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.time_server = cat_str("",parse_str);
			} else if (strncmp(last_parse_str,"bitst",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.bitstream = cat_str("",parse_str);
			} else if (strncmp(last_parse_str,"n_cha",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.n_chans = atoi(parse_str);
			} else if ((strncmp(last_parse_str,"n_ant",5)==0) && (strncmp(last_parse_str,"n_ants_per",10)!=0)){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.n_ants = atoi(parse_str);
			} else if (strncmp(last_parse_str,"fft_s",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.fft_shift = cat_str("",parse_str);
			} else if (strncmp(last_parse_str,"acc_l",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.acc_len = atoi(parse_str);
			} else if (strncmp(last_parse_str,"xeng_",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.xeng_acc_len = atoi(parse_str);
			} else if (strncmp(last_parse_str,"adc_c",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.adc_clk = atof(parse_str);
			} else if (strncmp(last_parse_str,"n_sto",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.n_stokes = atoi(parse_str);
			} else if (strncmp(last_parse_str,"x_per",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.x_per_fpga = atoi(parse_str);
			} else if (strncmp(last_parse_str,"clk_p",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.clk_per_sync = atoi(parse_str);
			} else if (strncmp(last_parse_str,"ddc_m",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.ddc_mix_freq = atof(parse_str);
			} else if (strncmp(last_parse_str,"ddc_d",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.ddc_decimation = atoi(parse_str);
			} else if (strncmp(last_parse_str,"feng_b",6)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.feng_bits = atoi(parse_str);
			} else if (strncmp(last_parse_str,"feng_f",6)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.feng_fix_pnt_pos = atoi(parse_str);
			} else if (strncmp(last_parse_str,"10gbe_pk",8)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.ten_gbe_pkt_len = atoi(parse_str);
			} else if (strncmp(last_parse_str,"10gbe_po",8)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.ten_gbe_port = atoi(parse_str);
			} else if (strncmp(last_parse_str,"10gbe_ip",8)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.ten_gbe_ip = atoi(parse_str);
			} else if (strncmp(last_parse_str,"x_eng",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.x_eng_clk = atoi(parse_str);
			} else if (strncmp(last_parse_str,"t_per",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.t_per_file = atoi(parse_str);
			} else if (strncmp(last_parse_str,"db_fi",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.db_file = cat_str("",parse_str);
			} else if (strncmp(last_parse_str,"rx_bu",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.rx_buffer_size = atoi(parse_str);
			} else if (strncmp(last_parse_str,"max_p",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.max_payload_len = atoi(parse_str);
			} else if (strncmp(last_parse_str,"udp_p",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.udp_port = atoi(parse_str);
			} else if (strncmp(last_parse_str,"toler",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.tolerance = atof(parse_str);
			} else if (strncmp(last_parse_str,"auto_",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				corr_config.auto_eq = atoi(parse_str);
			} else if (strncmp(last_parse_str,"EQ_po",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				parse_str = strtok(parse_str, ",");
				i = 0;
				while (parse_str != NULL) {
					corr_config.eq_poly[cEq][i] = atoi(parse_str);
					parse_str = strtok(NULL, ",");
					i++;
				}
				cEq++;
			} else if (strncmp(last_parse_str,"order",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				parse_str = strtok(parse_str, ",");
				for (i=0; i<corr_config.n_ants; i++) {
					corr_config.order[i] = atoi(parse_str);
					parse_str = strtok(NULL, ",");
				}
			} else if (strncmp(last_parse_str,"locat",5)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				
				int sign_change = 0;
				double pos_component;
				
				//latitude
				parse_str = strtok(parse_str, ":");
				corr_config.location.latitude = atof(parse_str)*60.0*60.0;
				parse_str = strtok(NULL, ":");
				corr_config.location.latitude = corr_config.location.latitude + atof(parse_str)*60.0;
				parse_str = strtok(NULL, ",");
				corr_config.location.latitude = corr_config.location.latitude + atof(parse_str);
				corr_config.location.latitude = degrad(corr_config.location.latitude/3600.0);
				
				//longitude
				parse_str = strtok(NULL, ":");
				pos_component = atof(parse_str)*60.0*60.0;
				if (0.0 > pos_component) { sign_change = 1;}
				corr_config.location.longitude = fabs(pos_component);
				parse_str = strtok(NULL, ":");
				corr_config.location.longitude = corr_config.location.longitude + atof(parse_str)*60.0;
				parse_str = strtok(NULL, ":");
				corr_config.location.longitude = corr_config.location.longitude + atof(parse_str);
				corr_config.location.longitude = (sign_change * -1) * corr_config.location.longitude * (PI/(180*60*60));
				//corr_config.location.longitude = degrad(corr_config.location.longitude/3600.0);
				
				//printf("%f %f\n",corr_config.location.longitude,corr_config.location.latitude);
				
			} else if (strncmp(last_parse_str,"pos_",4)==0){
				if (strncmp(parse_str," ",1)==0) { parse_str++; }
				parse_str = strtok(parse_str, ",");
				corr_config.position[cPos].x = atof(parse_str);
				parse_str = strtok(NULL, ",");
				corr_config.position[cPos].y = atof(parse_str);
				parse_str = strtok(NULL, ",");
				corr_config.position[cPos].z = atof(parse_str);
				cPos++;
			}
		}

	}
  } /* end while (cThisPtr <= cEndPtr) */

  printf("Done\n");
  return 1;

} /* end T_fread() */

/******************************************************************************/
void PrintLine(char *szReadLine,  long lLineCount,    long lLineLen,
               long lThisFilePos, long *lLastFilePos, int  isFilePosErr)
/******************************************************************************/
/* Use:       Print detail for current line                                   */
/*                                                                            */
/* Arguments: char *szReadLine   = Read buffer containg text line             */
/*            long  lLineCount   = Current line number                        */
/*            long  lLineLen     = Current line length                        */
/*            long  lThisFilePos = Offset of start of current line            */
/*            long *lLastFilePos = Offset of end of current line              */
/*            int   isFilePosErr = True if start of current line is not       */
/*                                   1 greater than end of last line          */
/*                                                                            */
/* Return:    void                                                            */
/******************************************************************************/
{
  char *cPtr; /* Pointer to current character */

  if (strncmp(szReadLine,"#",1)!=0){ /* skip commented lines */
  /* Print the characters, including null terminator */
  for (cPtr = szReadLine; cPtr <= szReadLine + lLineLen; cPtr++)
  {
    switch (*cPtr)
    {
      case 0:                 /* Null terminator */
        break;

      case CR:                /* Carriage return */
        break;
	
      case LF:                /* Line feed */
        break;

      case EOF_MARKER:        /* DOS end-of-file marker */
        break;

      default:                /* A 'real' character */
        break;

    } /* end switch (*cPtr) */
  } /* end for (cPtr) */
  
  }
  return;

} /* end PrintLine() */

/******************************************************************************/
CorrConf initCorrConf() { //initialize a default CorrConf struct
/******************************************************************************/
	CorrConf defaultConfig;
	int i;
	
	defaultConfig.n_chans = 2048;
	defaultConfig.n_ants = 8;
	defaultConfig.acc_len = 8192;
	defaultConfig.adc_clk = 0.600;
	defaultConfig.xeng_acc_len = 128;
	defaultConfig.n_stokes = 4;
	defaultConfig.ddc_mix_freq = 0.25;
	defaultConfig.ddc_decimation = 4;
	defaultConfig.clk_per_sync = pow(2,18);
	defaultConfig.t_per_file = 3600;
	defaultConfig.db_file = "";
	for(i=0; i<defaultConfig.n_ants; i++){
		defaultConfig.eq_poly[i][0] = 1;
	}
	defaultConfig.single_capture = 0;
	defaultConfig.verbose_mode = 0;
	defaultConfig.udp_port = 8888;
	defaultConfig.rx_buffer_size = 8192;
	defaultConfig.n_int_buffer = 4;
	defaultConfig.timestamp_scale_factor = 1.0/1000000;
	defaultConfig.timestamp_rounding = 0;
	defaultConfig.read_ahead_allow = 30 / defaultConfig.timestamp_scale_factor;
	defaultConfig.quit = 0;
	defaultConfig.stokes[0] = "xy";
	defaultConfig.stokes[1] = "yx";
	defaultConfig.stokes[2] = "xx";
	defaultConfig.stokes[3] = "yy";
	
	return defaultConfig;
	
} /* End initCorrConf()*/

/* end parse_conf.c */
