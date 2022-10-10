/***********
 c2m.c: convert packetized correlator data to miriad uv file format
 
 
 Global Variables:
 
 int tno:					integer pointer to the current uv file being edited
 CorrConfig corr_config:	meta data loaded from the command line config file
 
 ***********/

#include "c2m.h"

void parseCommandLine(int argc, char **argv)
{
	/************************/
	/* Parse CMD Arguements */
	/************************/

	int i;
	char *config_file;

	corr_config = initCorrConf();
	
	printf("Version %s\n", VERSION_ID);
	if (argc < 2)
	{
		printf("Usage: %s [options] CONFIG_FILE\n", argv[0]);
		printf("\t -v : verbose mode\n");
		printf("\t -s : Only do a single capture (one MIRIAD file).\n");
		printf("\t      Default: continuously create new MIRIAD files.\n");
		exit(0);
	}

	// Parse command line arguments
	for (i=1; i<argc; i++)
	{
		if (strcmp(argv[i], "-v") == 0)
		{
			corr_config.verbose_mode = 1;
		}
		else if (strcmp(argv[i], "-s") == 0)
		{
			corr_config.single_capture = 1;
		}
		else if (strcmp(argv[i], "-h") == 0)
		{
			printf("Usage: %s [options] CONFIG_FILE\n", argv[0]);
			printf("\t -v : verbose mode\n");
			printf("\t -s : Only do a single capture (one MIRIAD file).\n");
			printf("\t      Default: continuously create new MIRIAD files.\n");
			exit(0);
		}
		else
		{
			config_file = argv[i];
		}
	}

	//parse the config_file and write to the global corr_config struct
	calculateCorrelatorParameters(config_file);
}

void calculateCorrelatorParameters(char *config_file)
{    
    /******************************************/
    /* Load the parameters of the config file */
    /******************************************/

	/********************/
	/* Open config file */
	/********************/

    if (FileLoader(config_file) == 0) {
    	printf("Error: Config file did not successfully load\n");
    	exit(0);
    }
    
    corr_config.n_bls = corr_config.n_ants * (corr_config.n_ants+1.0)/2;
    corr_config.int_size = 2 * corr_config.n_stokes * corr_config.n_bls * corr_config.n_chans;

	/************************/
	/* Get the current time */
	/************************/

	corr_config.start_t = time(NULL);
	struct tm* tm_time = gmtime(&corr_config.start_t); //GMT to calculate the MJD
	double current_mjd, current_mjd_real;
	conv_time(tm_time, &current_mjd_real, &current_mjd);
	printf("MJD: %f\n", current_mjd_real);

	Now current_time;
	current_time.n_mjd = current_mjd;
	current_time.n_lng = corr_config.location.longitude;
	double current_lst;
	now_lst(&current_time, &current_lst); //use libastro to calculate the LST based on the MJD from cal_mjd()
	printf("LST: %i:%i:%f\n", (int) floor(current_lst) ,
			(int) floor((current_lst-floor(current_lst))*60), current_lst*3600
					- floor(current_lst)*3600 - floor((current_lst
					- floor(current_lst))*60)*60);
	printCorrConfig(corr_config);
}

void uv_init()
{
	/***************************/
	/* Initialize UV Items     */
	/***************************/

	int i;

	uv_open("zen.uv.tmp");

	//Derive channel frequencies from adc_clk_rate
	double bandwidth, sdf, sfreq, veldop=0.0, vsource=0.0;
	float epoch0=2000;
	double antpos[corr_config.n_ants * 3];
	//int pol;
	int nspect = 1, ischan = 1;

	bandwidth = corr_config.adc_clk / corr_config.ddc_decimation;
	sdf = bandwidth / corr_config.n_chans;
	sfreq = (corr_config.n_chans/2)*sdf;
	//sfreq = (corr_config.adc_clk * corr_config.ddc_mix_freq)
	//		+ ((corr_config.n_chans/2)*sdf);
	corr_config.int_time = corr_config.clk_per_sync / (bandwidth * pow(10.0, 9.0))
			* corr_config.acc_len;
	
	//sfreq = prms['adc_clk_rate']*prms['ddc_mix_freq'] + (prms['schan']*sdf)
	
	double freqs[1 + 3*corr_config.n_ants];

	freqs[0] = corr_config.n_ants;
	for (i=0; i<corr_config.n_ants; i++)
	{
		freqs[1+(i*3)] = corr_config.n_chans;
		freqs[2+(i*3)] = sfreq;
		freqs[3+(i*3)] = sdf;
	}

	int item_hdl;
	int in, offset = 0;
	double db;

	haccess_c(tno, &item_hdl, "freqs", "write", &iostat);
	in = (int) freqs[0];
	hwritei_c(item_hdl, &in, offset, H_INT_SIZE, &iostat);
	offset += 8;
	for (i=1; i<(1 + 3*corr_config.n_ants); i++)
	{
		if (i % 3 == 1)
		{
			in = (int) freqs[i];
			hwritei_c(item_hdl, &in, offset, H_INT_SIZE, &iostat);
		}
		else
		{
			db = freqs[i];
			hwrited_c(item_hdl, &db, offset, H_DBLE_SIZE, &iostat);
		}
		offset += 8;
	}
	hdaccess_c(item_hdl, &iostat);

	//Derive Bandpass
	float bps[corr_config.n_ants][corr_config.n_chans];
	int c, coeff;
	float cx[2];

	for (i=0; i<corr_config.n_ants; i++)
	{
		for (c=0; c<corr_config.n_chans; c++)
		{
			bps[i][c] = 0;
			for (coeff=0; coeff<MAX_COEFF; coeff++)
			{
				if (!corr_config.eq_poly[i][coeff]==0)
				{
					bps[i][c] += corr_config.eq_poly[i][coeff] * pow(c, (coeff*2));
				}
			}
			bps[i][c] = 1.0/bps[i][c];
		}
	}

	haccess_c(tno, &item_hdl, "bandpass", "write", &iostat);
	hwriteb_c(item_hdl,cmplx_item,0,H_CMPLX_SIZE,&iostat);
	offset = H_CMPLX_SIZE;
	for (i=0; i<corr_config.n_ants; i++)
	{
		for (c=0; c<corr_config.n_chans; c++)
		{
			cx[0] = bps[i][c];
			cx[1] = 0;
			hwritec_c(item_hdl, &cx, offset, H_CMPLX_SIZE, &iostat);
			offset += H_CMPLX_SIZE;
		}
	}
	hdaccess_c(item_hdl, &iostat);
	
	
	//write uv header items
	/*************************
	 unused header items
	 'vartable': 'a',
	 'interval': 'd',
	 'leakage' : 'c',
	 'freq0'   : 'd',
	 *************************/
	 
	haccess_c(tno, &item_hdl, "nfeeds", "write", &iostat); check(iostat);
    hwriteb_c(item_hdl,int_item,0,ITEM_HDR_SIZE,&iostat);
    offset = mroundup(ITEM_HDR_SIZE,H_INT_SIZE);
    in = (int) 1;
    hwritei_c(item_hdl, &in, offset, H_INT_SIZE, &iostat); check(iostat);
    offset = H_INT_SIZE;
    hdaccess_c(item_hdl, &iostat);

	haccess_c(tno, &item_hdl, "ntau", "write", &iostat); check(iostat);
    hwriteb_c(item_hdl,int_item,0,ITEM_HDR_SIZE,&iostat);
    offset = mroundup(ITEM_HDR_SIZE,H_INT_SIZE);
    in = (int) 0;
    hwritei_c(item_hdl, &in, offset, H_INT_SIZE, &iostat); check(iostat);
    offset = H_INT_SIZE;
    hdaccess_c(item_hdl, &iostat);

	haccess_c(tno, &item_hdl, "nchan0", "write", &iostat); check(iostat);
    hwriteb_c(item_hdl,int_item,0,ITEM_HDR_SIZE,&iostat);
    offset = mroundup(ITEM_HDR_SIZE,H_INT_SIZE);
    in = (int) corr_config.n_chans;
    hwritei_c(item_hdl, &in, offset, H_INT_SIZE, &iostat); check(iostat);
    offset = H_INT_SIZE;
    hdaccess_c(item_hdl, &iostat);
    
    haccess_c(tno, &item_hdl, "nsols", "write", &iostat); check(iostat);
    hwriteb_c(item_hdl,int_item,0,ITEM_HDR_SIZE,&iostat);
    offset = mroundup(ITEM_HDR_SIZE,H_INT_SIZE);
    in = (int) 0;
    hwritei_c(item_hdl, &in, offset, H_INT_SIZE, &iostat); check(iostat);
    offset = H_INT_SIZE;
    hdaccess_c(item_hdl, &iostat);
    
    haccess_c(tno, &item_hdl, "ngains", "write", &iostat); check(iostat);
    hwriteb_c(item_hdl,int_item,0,ITEM_HDR_SIZE,&iostat);
    offset = mroundup(ITEM_HDR_SIZE,H_INT_SIZE);
    in = (int) corr_config.n_ants * (0 + 1); //ngains = nants * (ntau + nfeeds)
    hwritei_c(item_hdl, &in, offset, H_INT_SIZE, &iostat); check(iostat);
    offset = H_INT_SIZE;
    hdaccess_c(item_hdl, &iostat);
    
    haccess_c(tno, &item_hdl, "nspect0", "write", &iostat); check(iostat);
    hwriteb_c(item_hdl,int_item,0,ITEM_HDR_SIZE,&iostat);
    offset = mroundup(ITEM_HDR_SIZE,H_INT_SIZE);
    in = (int) corr_config.n_ants;
    hwritei_c(item_hdl, &in, offset, H_INT_SIZE, &iostat); check(iostat);
    offset = H_INT_SIZE;
    hdaccess_c(item_hdl, &iostat);


	/***************************/
	/* Initialize UV Variables */
	/***************************/

	uvputvra_c(tno,"source","zenith");
	uvputvra_c(tno,"operator",VERSION_ID);
	uvputvra_c(tno,"version",VERSION);
	uvputvra_c(tno,"telescop","CASPER-N");

	//antpos
	for (i=0; i<(corr_config.n_ants); i++)
	{
		antpos[i] = corr_config.position[i].x;
	}
	for (i=0; i<(corr_config.n_ants); i++)
	{
		antpos[i+corr_config.n_ants] = corr_config.position[i].y;
	}
	for (i=0; i<(corr_config.n_ants); i++)
	{
		antpos[i+2*corr_config.n_ants] = corr_config.position[i].z;
	}
	uvputvrd_c(tno,"antpos",&antpos, 3*corr_config.n_ants);

	uvputvrd_c(tno,"freq",&sfreq,1);
	uvputvrr_c(tno,"inttime",&corr_config.int_time,1);
	uvputvri_c(tno,"nants",&corr_config.n_ants, 1);
	uvputvri_c(tno,"nchan",&corr_config.n_chans, 1);
	uvputvri_c(tno,"nspect",&nspect, 1);
	uvputvrd_c(tno,"sfreq",&sfreq,1);
	uvputvrd_c(tno,"sdf",&sdf,1);
	uvputvri_c(tno,"ischan",&ischan, 1);
	uvputvri_c(tno,"nschan",&corr_config.n_chans, 1);
	uvputvrd_c(tno,"restfreq",&sfreq,1);
	uvputvri_c(tno,"npol",&corr_config.n_stokes, 1);
	uvputvrr_c(tno,"epoch",&epoch0,1);
	uvputvrr_c(tno,"veldop",&veldop,1);
	uvputvrr_c(tno,"vsource",&vsource,1);
	uvputvrd_c(tno,"longitu",&corr_config.location.longitude,1);
	uvputvrd_c(tno,"latitud",&corr_config.location.latitude,1);
	uvputvrd_c(tno,"dec",&corr_config.location.latitude,1);
	uvputvrd_c(tno,"obsdec",&corr_config.location.latitude,1);

	///// empty initially
	//uvputvrd_c(tno,"ra",&current_lst,1);
	//uvputvrd_c(tno,"obsra",&current_lst,1);
	//uvputvrd_c(tno,"lst",&current_lst,1);
	//uvputvri_c(tno,"pol",&pol, 1);
	//corr
	//baseline
	//coord
	//tscale
	//time
	
	printCorrConfig(corr_config);
}

void uv_open(const char *name)
/* Open a Miriad UV file for writing. */
{
	int result = file_exists(name);
	char *remove_str = "rm -rf ";
	char *cmd_str = cat_str(remove_str, name);

	printf("Starting file: %s\n", name);
	if (result == 1)
	{
		system(cmd_str);
	}
	uvopen_c(&tno, name, "new");

	//write to the history file
	haccess_c(tno, &history[tno], "history", "write", &iostat);
	hwritea_c(history[tno], VERSION_ID, strlen(VERSION_ID)+1, &iostat);
	hdaccess_c(history[tno], &iostat);

	wrhda_c(tno, "obstype", "mixed-auto-cross");
}

void uv_close()
{	
	char *cmd_str = "mv zen.uv.tmp";
	//char *cmd_str = cat_str(move_str, name);
	char mjd_str[23];
	
	//rename file on close
	time_t end_t;
	end_t = time(NULL);
	struct tm* tm_time = gmtime(&end_t); //GMT to calculate the MJD
	double current_mjd, current_mjd_real;
	conv_time(tm_time, &current_mjd_real, &current_mjd);

	sprintf(mjd_str," zen.%.5f.uv",current_mjd_real+2400000.5);
	char *cmd_str_final = cat_str(cmd_str, mjd_str);
	
	printf("Closing file: %s\n", mjd_str);
	uvclose_c(tno);
	system(cmd_str_final);
}

void uv_write_int(double data[], uint64_t timestamp) { 
//data is a 2D array of shape
// n_ants x (2 *n_stokes*n_bls*n_chans per x_eng)
	int t = (int) timestamp;
	file_data(t, data);
	
	if (difftime(time(NULL),corr_config.start_t) > corr_config.t_per_file) {
		uv_close();
		if (corr_config.single_capture == 0){
			corr_config.start_t = time(NULL);
			uv_init();
		} else {
			printf("Grabbed a single file... exiting.\n");
			corr_config.quit = 1;
		}
	}
}

void file_data(int timestamp, double data[]) { 
	
	printf("Recording data for timestamp: %d\n",timestamp);
	int s, b;
	//int n_bls = corr_config.n_ants + (corr_config.n_ants * (corr_config.n_ants - 1) )/ 2;
	int ij[2];
	int bl_order[corr_config.n_bls];
	double d[corr_config.n_chans*2];
	double gain = corr_config.xeng_acc_len * corr_config.acc_len;
	int i;

	get_bl_order(corr_config.n_ants, &bl_order);

	//double zbls[3][corr_config.n_bls];
	double zbls[corr_config.n_bls][3];
	//generate_zbls(&zbls);
	generate_zbls(zbls);
	
	for (s=0; s<corr_config.n_stokes; s++) {
		for (b=0; b<corr_config.n_bls; b++) {
			bl2ij(bl_order[b], &ij);
			//printf("i:%i j:%i\n",ij[0],ij[1]);
			//this is where I need to cut up Billy's block of memeroy
			//d = data[:,b,s,:] as type (float)
			//scale the data - fixed-point format is 32_6 by default
			for (i=0; i<corr_config.n_chans; i++){
				d[i] = d[i]/gain;
				d[i+corr_config.n_chans] = d[i]/gain;
				
				// //for now just write empty data to the d array
				// d[i]=0.0;
				// d[i+corr_config.n_chans]=0.0;
			}
			
			//printf("%i %i %i %s",timestamp,ij[0],ij[1],corr_config.stokes[s]);
			//Write to MIRIAD file
			PREAMBLE p;
			gen_preamble(timestamp, ij[0], ij[1], &p);
			p.uvw.uu = zbls[b][0];
			p.uvw.vv = zbls[b][1];
			p.uvw.ww = zbls[b][2];
			//p.uvw.uu = zbls[0][b];
			//p.uvw.vv = zbls[1][b];
			//p.uvw.ww = zbls[2][b];
			
			//printf("zbls: %f %f %f\n",zbls[b][0],zbls[b][1],zbls[b][2]);
			//printf(" uvw: %f %f %f\n",p.uvw.uu,p.uvw.vv,p.uvw.ww);
			
			record(p, str2pol(corr_config.stokes[s]), d);
			
			//Write to optional database file
		}
	}
}

void record(PREAMBLE p, int pol, double data[])
/*Record to file 'data' for antennas i, j, stokes parameter s for an integration which 
finished at time 't' (Unix time). */
{
	double preamble[PREAMBLE_SIZE];
	int flags[corr_config.n_chans];
	int i;
	for (i=0; i<corr_config.n_chans; i++){ flags[i] = 0;}

	Now current_time;
	current_time.n_mjd = p.t;
	current_time.n_lng = corr_config.location.longitude;
	double lst;
	now_lst(&current_time, &lst); //use libastro to calculate the LST based on the MJD from cal_mjd()

	uvputvri_c(tno,"pol",&pol, 1);
	uvputvrd_c(tno,"ra",&lst,1);
	uvputvrd_c(tno,"obsra",&lst,1);
	uvputvrd_c(tno,"lst",&lst,1);

	// Fill up the preamble
    	preamble[0] = p.uvw.uu;
    	preamble[1] = p.uvw.vv;
    	preamble[2] = p.uvw.ww;
    	preamble[3] = lst;
    	preamble[4] = MKBL(p.i,p.j);
	int sum1=0,sum2=0;
	for (i=0; i<corr_config.n_chans; i++) {
		sum1=+ data[i];
		sum2=+flags[i];
	}
	//printf("data:%i flags:%i\n",sum1,sum2);
	//printf("u:%f, v:%f, w:%f, lst:%f, bl:%f\n",preamble[0],preamble[1],preamble[2],preamble[3],preamble[4]);
    
    	//uvwrite_c(tno, preamble, data, flags, corr_config.n_chans);
    	uvwrite_c(tno, preamble, (float *)data, (int *)flags, corr_config.n_chans);

    //uvwrite_c(self->tno, preamble,
    //        (float *)data->data, (int *)flags->data, corr_config.n_chans);
	
	//self.uv.write(p, data, numpy.zeros_like(data))
	//self.raw_write(preamble,data.astype(n.complex64),flags.astype(n.int32))
	// {"raw_write", (PyCFunction)UVObject_write, METH_VARARGS,
    //    "_write(preamble,data,flags)\nWrite the provided preamble, data, 
    //    flags to file.  See _read() for definitions of preamble, data, flags."},
    
    
}

/* Wrapper over uvwrite_c to deal with numpy arrays, conversion of baseline
 * codes, and accepts preamble as a tuple.
 */
/*PyObject * UVObject_write(UVObject *self, PyObject *args) {
    PyArrayObject *data=NULL, *flags=NULL, *uvw=NULL;
    int i, j;
    double preamble[PREAMBLE_SIZE], t;
    // Parse arguments and typecheck
    if (!PyArg_ParseTuple(args, "(O!d(ii))O!O!", 
        &PyArray_Type, &uvw, &t, &i, &j,
        &PyArray_Type, &data, &PyArray_Type, &flags)) return NULL;
    if (RANK(uvw) != 1 || DIM(uvw,0) != 3) {
        PyErr_Format(PyExc_ValueError,
            "uvw must have shape (3,) %d", RANK(uvw));
        return NULL;
    } else if (RANK(data)!=1 || RANK(flags)!=1 || DIM(data,0)!=DIM(flags,0)) {
        PyErr_Format(PyExc_ValueError,
            "data and flags must be 1 dimensional and have the same shape");
        return NULL;
    }
    CHK_ARRAY_TYPE(uvw, NPY_DOUBLE);
    CHK_ARRAY_TYPE(data, NPY_CFLOAT);
    // Check for both int,long, b/c label of 32b number is platform dependent
    if (TYPE(flags) != NPY_INT && \
            (sizeof(int) == sizeof(long) && TYPE(flags) != NPY_LONG)) {
        PyErr_Format(PyExc_ValueError, "type(flags) != NPY_LONG or NPY_INT");
        return NULL;
    }
    // Fill up the preamble
    preamble[0] = IND1(uvw,0,double);
    preamble[1] = IND1(uvw,1,double);
    preamble[2] = IND1(uvw,2,double);
    preamble[3] = t;
    preamble[4] = MKBL(i,j);
    // Here is the MIRIAD call
    try {
        uvwrite_c(self->tno, preamble,
            (float *)data->data, (int *)flags->data, DIM(data,0));
    } catch (MiriadError &e) {
        PyErr_Format(PyExc_RuntimeError, e.get_message());
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}*/

/*
    def record(self, t, i, j, pol, data):
        """Record to file 'data' for antennas i, j, stokes parameter s for
        an integration which finished at time 't' (Unix time)."""
        t -= self['inttime'][1] / 2
        p = self.gen_preamble(t, i, j)
        self.aa.set_jultime(p[-2])
        lst = self.aa.sidereal_time()
        self.uv['pol'] = aipy.miriad.str2pol[pol]
        self.uv['ra'] = lst
        self.uv['obsra'] = lst
        self.uv['lst'] = lst
        self.uv.write(p, data, numpy.zeros_like(data))
    
    def write(self, preamble, data, flags=None):
        """Write the next data record.  data must be a complex, masked
        array.  preamble must be (uvw, t, (i,j)), where uvw is an array of 
        u,v,w, t is the Julian date, and (i,j) is an antenna pair."""
        if data is None: return
        if not flags is None: flags = n.logical_not(flags)
        elif len(data.mask.shape) == 0:
            flags = n.ones(data.shape)
            data = data.unmask()
        else:
            flags = n.logical_not(data.mask)
            #data = data.filled(0)
            data = data.data
        self.raw_write(preamble,data.astype(n.complex64),flags.astype(n.int32))
                
    def file_data(self, t):
        #print 'Recording data for timestamp %f' % t           
        self.data.shape = (self.n_chans, self.n_bls, self.n_stokes, 2)
        # Write spectrum for each stokes, baseline to file.
        for s,pol in enumerate(self.stokes):
            for b, bl in enumerate(self.bl_order):
                i, j = corr.sim_cn_data.bl2ij(bl)
                
                d = self.data[:,b,s,:].astype(numpy.float)
                #scale the data - fixed-point format is 32_6 by default
                d /= self.gain
                #turn the two independant values into a single complex number:
                dfloat = d[...,0] - d[...,1]*1j
                # Write to MIRIAD file
                self.mrec.record(t, i, j, pol, dfloat)
                # Optionally write to database file as well
                if not self.ndb is None:
                    self.ndb.write('%i-%i,%s' % (i, j, pol), dfloat)
        # Clear the data buffer
        self.data.shape = (self.n_chans, self.n_bls * self.n_stokes * 2)
        self.data *= 0
*/

void conv_time(struct tm* t, double *current_mjd_real, double *current_mjd)
/* Convert standard unix time (seconds since whenever) to a Modified Julian date. */
{
	double mjd1, mjd2;
	double decimal_day = (double) t->tm_mday;
	decimal_day += (double) t->tm_hour/(24);
	decimal_day += (double) t->tm_min/(24*60);
	decimal_day += (double) t->tm_sec/(24*60*60);
	int current_month = t->tm_mon + 1;
	int current_year = t->tm_year + 1900;
	double current_day = decimal_day;

	calc_mjd(current_month, current_day, current_year, &mjd1);
	cal_mjd(current_month, current_day, current_year, &mjd2);
	*current_mjd_real = mjd1;
	*current_mjd = mjd2;
}

void gen_preamble(int t, int i, int j, PREAMBLE *p)
/* Generate preamble information (u, v, w, t, bl) for a given time and two antennas (i, j). */
{
	PREAMBLE p0;
	/*p0.uvw.uu = 1.0;
	p0.uvw.vv = 2.0;
	p0.uvw.ww = 3.0;*/
	
	//convert the integer seconds to a double mjd
	time_t t0 = (time_t) t - (corr_config.int_time/2);
	struct tm* p_time = gmtime(&t0);
	double p_mjd_real, p_mjd;
	conv_time(p_time, &p_mjd_real, &p_mjd);
	p0.t = p_mjd;
	
	//store baselines
	p0.i = i;
	p0.j = j;
	
	//point empty preamble to new preamble
	*p = p0;
}

void eq2top_m(double ha, double dec, double *m)
/*Return the 3x3 matrix converting equatorial coordinates to topocentric
	at the given hour angle (ha) and declination (dec).*/
{
	double sin_H = sin(ha);
	double cos_H = cos(ha);
	double sin_d = sin(dec);
	double cos_d = cos(dec);
	
	double map[3][3] = { 		sin_H, 		  cos_H,   0.0,
						 -sin_d*cos_H,  sin_d*sin_H, cos_d,
						  cos_d*cos_H, -cos_d*sin_H, sin_d };
	m = &map;
}

//void generate_zbls(double *z) {
double *generate_zbls(double z[][3]) {
	double bls[3][corr_config.n_bls];
	int i,j;
	int bl_number=0;
	double m[3][3];
	
	for (i=0; i<corr_config.n_ants; i++) {
		for (j=i; j<corr_config.n_ants; j++) {
			bls[0][bl_number]=corr_config.position[j].x-corr_config.position[i].x;
			bls[1][bl_number]=corr_config.position[j].y-corr_config.position[i].y;
			bls[2][bl_number]=corr_config.position[j].z-corr_config.position[i].z;
			//printf("x:%f y:%f z:%f\n",bls[0][bl_number],bls[1][bl_number],bls[2][bl_number]);
			bl_number++;
		}
	}
	eq2top_m(0.0, corr_config.location.latitude, &m);
	//printf("%f %f %f %f %f %f %f %f %f\n",m[0][0],m[0][1],m[0][2],m[1][0],m[1][1],m[1][2],m[2][0],m[2][1],m[2][2]);

	int k;
	double sum;
	//static double zbls[3][corr_config.n_bls];
	for(i = 0; i<3; i++) {
		for(j = 0; j<corr_config.n_bls; j++) {
			sum = 0;
			for(k = 0; k<3; k++) {
				sum += m[i][k] * bls[k][j];
			}
		//zbls[i][j] = sum;
		z[j][i] = sum;
		//printf("%f ",zbls[i][j]);
		}
	}
	//z = &zbls;
}

/*
	def eq2top_m(ha, dec):
    	"""Return the 3x3 matrix converting equatorial coordinates to topocentric
    	at the given hour angle (ha) and declination (dec)."""
    	sin_H, cos_H = n.sin(ha), n.cos(ha)
    	sin_d, cos_d = n.sin(dec), n.cos(dec)
    	zero = n.zeros_like(ha)
    	map =  n.array([[    sin_H    ,       cos_H  ,       zero  ],
    	                [ -sin_d*cos_H,   sin_d*sin_H,      cos_d  ],
    	                [  cos_d*cos_H,  -cos_d*sin_H,      sin_d  ]])
    	if len(map.shape) == 3: map = map.transpose([2, 0, 1])
    	return map

	def update(self):
        """Update variables derived from antenna parameters/active channels."""
        bls,dlys,offs = [],[],[]
        self.bl_order = {}
        # Calculate deltas between antenna pairs
        for i, ai in enumerate(self.ants):
            for j, aj in enumerate(self.ants[i:]):
                bl = self.ij2bl(i, j+i)
                bls.append(aj - ai)
                dlys.append(aj.delay - ai.delay)
                offs.append(aj.offset - ai.offset)
                self.bl_order[bl] = len(bls) - 1
        self.bls,self.dlys,self.offs = n.array(bls),n.array(dlys),n.array(offs)
        # Compute (static) zenith baselines
        m = coord.eq2top_m(0., self.lat)
        self.zbls = n.dot(m, self.bls.transpose()).transpose()
        
    def get_baseline(self, i, j, src='z'):
        """Return the baseline corresponding to i,j in various coordinate 
        projections: src='e' for current equatorial, 'z' for zenith 
        topocentric, 'r' for unrotated equatorial, or a RadioBody for
        projection toward that source."""
        b = self.bl_order[self.ij2bl(i,j)]
        if type(src) == str:
            if src == 'e': return self.ebls[b]
            elif src == 'z': return self.zbls[b]
            elif src == 'r': return self.bls[b]
            else: raise ValueError('Unrecognized source:' + src)
        if src.alt < 0: raise PointingError('%s below horizon' % src.src_name)
        return n.dot(src.map, self.bls[b])

    def gen_preamble(self, t, i, j):
        """Generate preamble information (u, v, w, t, bl) for a given
        time and two antennas (i, j)."""
        uvw = self.aa.get_baseline(i, j)
        #uvw = self.aa.get_projected_baseline(i, j)
        return (uvw, self.conv_time(t), (i,j))
*/

int str2pol(char * pol)
/* Return the MIRIAD integer value for different polarizations*/
{
	if (strcmp(pol,"I") == 0) { return 1; }
	else if (strcmp(pol,"Q") == 0) { return 2; }
	else if (strcmp(pol,"U") == 0) { return 3; }
	else if (strcmp(pol,"V") == 0) { return 4; }
	else if (strcmp(pol,"rr") == 0) { return -1; }
	else if (strcmp(pol,"ll") == 0) { return -2; }
	else if (strcmp(pol,"rl") == 0) { return -3; }
	else if (strcmp(pol,"lr") == 0) { return -4; }
	else if (strcmp(pol,"xx") == 0) { return -5; }
	else if (strcmp(pol,"yy") == 0) { return -6; }
	else if (strcmp(pol,"xy") == 0) { return -7; }
	else if (strcmp(pol,"yx") == 0) { return -8; }	
}

char * cat_str(char * arg1, const char * arg2)
/* create a new string out of two input strings */
{
	char *return_str;
	return_str = (char *)calloc(strlen(arg1) + strlen(arg2) + 1, sizeof(char));
	strcat(return_str, arg1);
	strcat(return_str, arg2);
	return return_str;
}

int file_exists(const char *filename)
/* returns 1 if file exists, 0 if file does not exist */
{
	struct stat;
	int i = access(filename, F_OK);
	if (i == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void bl2ij(int bl, int *ij) {
/*	Convert from Miriad's baseline notation (counting from 1, a 16 bit 
	number) to i, j baseline notation (counting from 1). */
	ij[0] = ((bl >> 8) & 255) - 1;
	ij[1] = (bl & 255) - 1;
}

void il2bl(int i, int j, int *bl) {
/* 	Convert i, j baseline notation (counting from 0) to Miriad's baseline
	notation (counting from 1, a 16 bit number).*/
	*bl = ((i+1) << 8) | (j+1);
}

void get_bl_order(int n_ants, int *bl_order){
/*	Return the order of baselines output by an x engine in the casper_n
	correlator.  Baselines are in Miriad notation. */
	int i,j,jp,k;
	int j_range = (n_ants/2);
	int order1[n_ants*(j_range+1)][2], order2[n_ants*(j_range+1)][2];
	int o1=0, o2=0;
	for (i=0; i<n_ants; i++) {
		for (j=0; j<j_range+1; j++) {
			jp = j_range - j;
			k = (i-jp) % n_ants;
			if (k < 0) { k = k+n_ants;}
			if (i >= k) {
				order1[o1][0] = k;
				order1[o1][1] = i;
				o1++;
			} else {
				order2[o2][0] = i;
				order2[o2][1] = k;
				o2++;
			}
		}
	}
	int does_exist = 0, o = 0;
	for (i=0; i<o2; i++) {
		does_exist = 0;
		for (j=0; j<o1; j++) {
			if ((order2[i][0] == order1[j][0]) && (order2[i][1] == order1[j][1])) {
				does_exist = 1;
			}
		}
		if (does_exist == 0) {
			order1[o1+o][0] = order2[i][0];
			order1[o1+o][1] = order2[i][1];
			o++;
		}
	}
	for (i=0; i<(o1+o); i++) {
		il2bl(order1[i][0], order1[i][1], &bl_order[i]);
	}
}

void printCorrConfig(CorrConf arg){
/* Print the contents of a CorrConfig structure*/
	int i, j;

	for (i=0; i<4; i++)
	{
		printf("server%i: %s\n", i, arg.servers[i]);
	}
	printf("time server: %s\n", arg.time_server);
	printf("bitstream: %s\n", arg.bitstream);
	printf("adc clk rate: %f\n", arg.adc_clk);
	printf("N chans: %i\n", arg.n_chans);
	printf("N ants: %i\n", arg.n_ants);
	printf("FFT Shift: %s\n", arg.fft_shift);
	printf("ACC_LEN: %i\n", arg.acc_len);
	printf("XENG_ACC_LEN: %i\n", arg.xeng_acc_len);
	printf("clk_per_sync: %i\n", arg.clk_per_sync);
	printf("ddc_mix_freq: %f\n", arg.ddc_mix_freq);
	printf("ddc_decimation: %f\n", arg.ddc_decimation);
	printf("feng_bits: %i\n", arg.feng_bits);
	printf("feng_fix_pnt_pos: %i\n", arg.feng_fix_pnt_pos);
	printf("ten_gbe_pkt_len: %i\n", arg.ten_gbe_pkt_len);
	printf("ten_gbe_port: %i\n", arg.ten_gbe_port);
	printf("ten_gbe_ip: %i\n", arg.ten_gbe_ip);
	printf("x_eng_clk: %i\n", arg.x_eng_clk);
	printf("t_per_file: %i\n", arg.t_per_file);
	printf("db_file: %s\n", arg.db_file);
	printf("rx_buffer_size: %i\n", arg.rx_buffer_size);
	printf("max_payload_len: %i\n", arg.max_payload_len);
	printf("udp_port: %i\n", arg.udp_port);
	printf("tolerance: %f\n", arg.tolerance);
	printf("auto_eq: %i\n", arg.auto_eq);
	for (i=0; i<arg.n_ants; i++)
	{
		printf("eq_poly%i: ", i);
		for (j=0; j<MAX_COEFF; j++)
		{
			printf(" %i", arg.eq_poly[i][j]);
		}
		printf("\n");
	}
	for (i=0; i<arg.n_ants; i++)
	{
		printf("order%i: %i\n", i, arg.order[i]);
	}
	printf("location lat: %f long: %f\n", arg.location.latitude, arg.location.longitude);
	for (i=0; i<arg.n_ants; i++)
	{
		printf("pos%i: x: %f y: %f z: %f\n", i, arg.position[i].x,
				arg.position[i].y, arg.position[i].z);
	}
	printf("signle capture: %i\n", arg.single_capture);
	printf("verbose mode: %i\n", arg.verbose_mode);
	printf("n_int_buffer: %i\n", arg.n_int_buffer);
	printf("timestamp scale factor: %f\n", arg.timestamp_scale_factor);
	printf("timestamp rounding: %i\n", arg.timestamp_rounding);
	printf("read ahead allow: %i\n", arg.read_ahead_allow);
	printf(ctime(&arg.start_t));
	printf("quit: %i\n", arg.quit);
	printf("stokes: 0:%s 1:%s 2:%s 3:%s\n", arg.stokes[0], arg.stokes[1],
			arg.stokes[2], arg.stokes[3]);
}

/*
ant pos to uvw:
from uvgen.f:
	call keyt('lat',alat,'dms',40.d0*pi/180)
	sinl = sin(alat)
	cosl = cos(alat)
	
	call keyt('radec',sra,'hms',0.d0)
	call keyt('radec',sdec,'dms',30.d0*dpi/180.)
	doellim = keyprsnt('ellim')
	call keyt('ellim',elev,'dms',15.d0*pi/180.)
	sind = sin(sdec)
	cosd = cos(sdec)
	
c
c  Read the antenna positions file.
c
	call output(' ')
	call output('Array information:')
	call output('------------------')
	call output('Antenna positions :')
	call hiswrite(unit,'UVGEN: Antenna positions :')
	nant = 0
c
	call tinOpen(antfile,' ')
	dowhile(tinNext().gt.0)
	  nant = nant + 1
	  if(nant.gt.MAXANT)call bug('f','Too many antennas')
	  call tinGetr(b1(nant),0.0)
	  call tinGetr(b2(nant),0.0)
	  call tinGetr(b3(nant),0.0)
c
c  Convert to equatorial coordinates.
c
	  if(utns .lt. 0.) then
	    x = b1(nant)
	    z = b3(nant)
	    b1(nant) = -x * sinl + z * cosl
	    b3(nant) =  x * cosl + z * sinl
 	  end if

c  Compute visibility for each baseline.
c
	    do n = 2, nant
	      do m = 1, n-1
	        preamble(5) = antbas(m,n)
	        bxx = b1(n) - b1(m)
	        byy = b2(n) - b2(m)
	        bzz = b3(n) - b3(m)
		bxy = bxx * sinha + byy * cosha
		byx =-bxx * cosha + byy * sinha
	        preamble(1) = bxy
	        preamble(2) =  byx*sind + bzz*cosd
		preamble(3) = -byx*cosd + bzz*sind
		baseline = 3.e-4 * sqrt(bxx*bxx + byy*byy + bzz*bzz)
*/
