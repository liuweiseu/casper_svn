import ConfigParser,exceptions
"""
Library for parsing CASPER correlator configuration files

Author: Jason Manley
Revision: 2008-02-08

Revs:
2008-12-04: JRM Stripped-out most of it for use with PoCo
2008-02-08: JRM Replaced custom tokeniser with string.split
                Changed EQ to EQ_polys
                Changed max_payload_len to rx_buffer_size

"""
LISTDELIMIT = ','
PORTDELIMIT = ':'


class CorrConf:    
    def __init__(self, config_file):
        self.config_file=config_file
        self.cp = ConfigParser.ConfigParser()
        self.cp.read(config_file)
        self.config=dict()
        self.read_all()

    def __getitem__(self,item):
        return self.config[item]

    def file_exists(self):
        try:
            f = open(self.config_file)
        except IOError:
            exists = False
        else:
            exists = True
            f.close()
        return exists

    def read_all(self):
        if not self.file_exists():
            return 'ERR: Config file not found'
        self.config['correlator']=dict()
    
        #Get the servers:
        self.config['connection']=dict()
        try:
            port=self.cp.get('connection','port')
            self.config['connection']['port']=port          
        except ConfigParser.NoOptionError:
            return 'ERR: %s not found'%'port parameter'
        except exceptions.ValueError:
            return 'ERR: value %s'%'port parameter'
        except ConfigParser.NoSectionError:
            return 'ERR: %s section not found'%'connection'
        except:
            return 'ERR: unknown error in %s section'%'connection'

        #get the correlator config stuff:
        try:
            self.read_int('correlator','n_chans')
            self.read_str('correlator','fft_shift')
            self.read_int('correlator','acc_len')
            self.read_float('correlator','adc_clk')
            self.read_int('correlator','n_ants')
            self.read_float('correlator','ddc_mix_freq')
            self.read_int('correlator','ddc_decimation')
        except:
            return 'ERR Processing the correlator section'
                
        #get the receiver section:
        try:
            self.config['receiver']=dict()
            self.read_int('receiver','t_per_file')
            self.read_str('receiver','db_file')
        except:
            return 'ERR Processing the receiver section'


        #equalisation section:
        self.config['equalisation']=dict()
        self.read_int('equalisation','n_coeff_tables')
        self.read_int('equalisation','auto_eq')
        self.config['auto_eq']=bool(self.config['auto_eq'])
        self.config['equalisation']['eq_polys']=[]
        for ant in range(self.config['n_ants']):
            ant_eq_str=self.get_line('equalisation','EQ_poly_%i'%(ant))
            if (ant_eq_str):
                self.config['equalisation']['eq_polys'].append([int(coef) for coef in ant_eq_str.split(LISTDELIMIT)])
            else:
                return 'ERR EQ_poly_%i'%(ant)
                break
        
        if self.config['n_ants'] == 4:
            self.config['n_stokes']=1
        elif self.config['n_ants'] == 2:
            self.config['n_stokes']=4

        # get the antenna info:
        self.config['antennas']=dict()

        self.config['antennas']['pos']=[]
        for ant in range(self.config['n_ants']):
            ant_pos_str=self.get_line('antennas','pos_%i'%(ant))
            if (ant_pos_str):
                self.config['antennas']['pos'].append([float(coef) for coef in ant_pos_str.split(LISTDELIMIT)])
            else:
                return 'ERR pos_%i'%(ant)
                break

        self.config['antennas']['location']=[]
        ant_loc_str=self.get_line('antennas','location')
        if (ant_loc_str):
            location = ant_loc_str.split(LISTDELIMIT)
            if len(location) == 3:
                location[2]=float(location[2])
                self.config['antennas']['location']= location
            else: 
                return 'ERR setting array location'
        else:
            return 'ERR setting array location'

        self.config['antennas']['order']=[]
        ant_ord_str=self.get_line('antennas','order')
        if (ant_ord_str):
            order = ant_ord_str.split(LISTDELIMIT)
            if len(order) == self.config['n_ants']:
                try:
                    self.config['antennas']['order']= [int(ant) for ant in order]
                except:
                    return 'ERR setting antenna ordering'                    
            else: 
                return 'ERR setting antenna ordering'
        else:
            return 'ERR setting antenna ordering'

        
        return 'OK'



    def get_line(self,section,variable):
        try:
            return self.cp.get(section,variable)
        except:
            return None

    def read_int(self,section,variable):
        self.config[variable]=self.cp.getint(section,variable)

    def read_str(self,section,variable):
        self.config[variable]=self.cp.get(section,variable)

    def read_float(self,section,variable):
        self.config[variable]=self.cp.getfloat(section,variable)
        
    def calc_consts(self):
        print 'Calculating constants and populating as necessary'
