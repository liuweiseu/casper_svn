#!/usr/bin/env python

'''
Plots the cross correlation for a user-defined input.
'''
import os, gtk, gobject, poco, time, numpy, struct, sys, math, matplotlib
matplotlib.use('GTKAgg')
from matplotlib import rcParams
import pylab, time


if __name__ == '__main__':
    from optparse import OptionParser

    p = OptionParser()
    p.set_usage('plot_cross.py [options] CONFIG FILE')
    p.set_description(__doc__)
    p.add_option('-l', '--log', dest='log', action='store_true', default=False,
        help='''Plot the power in logarithmic scale (requires some non-zero value signal).''')
    p.add_option('--hold', dest='hold', action='store_true', default=False,
        help='''Turn on hold. This will plot subsequent specra on top of each other.''')

    # Added By Sandeep
    # Integer Delay
    p.add_option('-A', '--anta', dest='anta', type='int', default=0, help='''Antenna "a" Integer Delay''')
    p.add_option('-B', '--antb', dest='antb', type='int', default=0, help='''Antenna "b" Integer Delay''')
    p.add_option('-I', '--incr', dest='incr', type='int', default=0, help='''Integer Delay increment''')
    # Fractional Delay 
    p.add_option('-a', '--antaf', dest='antaf', type='int', default=0, help='''Antenna "a" Fractional Delay''')
    p.add_option('-b', '--antbf', dest='antbf', type='int', default=0, help='''Antenna "b" Fractional Delay''')
    p.add_option('-i', '--incrf', dest='incrf', type='int', default=0, help='''Fractional Delay increment''')
    opts, args = p.parse_args(sys.argv[1:])

    if args==[]:
        print 'Please specify a configuration file! \nExiting.'
        exit()

    config = poco.poco_conf.CorrConf(args[0])
    config_status = config.read_all()
    print '\n\nParsing config file %s...%s'%(args[0],config_status)
    sys.stdout.flush()
    if not config_status == 'OK': exit()

#print 'Trying to plot cross correlations amplitude and phase for baseline %s.'%opts.baseline
bram_map=['dir_x/ac/real','dir_x/ac/imag','dir_x/bd/real','dir_x/bd/imag', 'dir_x/aa/real', 'dir_x/bb/real', 'dir_x/cc/real', 'dir_x/dd/real']
ants	=['a','b']

port = config['connection']['port']
n_chans = config['n_chans']
cen_freq = config['adc_clk']*1000.*config['ddc_mix_freq']
bandwidth = config['adc_clk']*1000./config['ddc_decimation']


print 'Connecting to IBOB on port %s.'%port
brd = poco.ibob_serial.IbobClient(port)
cnt = 0

# Set the Delay for a given Antenna
print ('''Setting Delays to Antennas''')
sys.stdout.flush()
print ('''Setting Integer Delay to Ant A %i...''' %(opts.anta))
brd.write_uint('int_delay_a',opts.anta)
print ('''Setting Integer Delay to Ant B %i...''' %(opts.antb))
brd.write_uint('int_delay_b',opts.antb)
print ('''Setting Fract Delay to Ant A %i...''' %(opts.antaf))
brd.write_uint('fract_delay_a',opts.antaf)
print ('''Setting Fract Delay to Ant B %i...''' %(opts.antbf))
brd.write_uint('fract_delay_b',opts.antbf)
print 'All done with setting of delay values.\n'

fig = pylab.figure(1)
manager = pylab.get_current_fig_manager()

x=range(0,n_chans)
xlab=('Input channel number')

os.system('echo "*" `date`')
t0 = time.time()
print 'time t0 is %f' %(t0)
toggle = 0

# Sync Reset To  IBOB (sync_rst) 
print '''Syncing the ibobs & setting FFT shift...'''
sys.stdout.flush()
brd.write_uint('sync_rst', 1)
brd.write_uint('sync_rst', 0)

def updatefig(*args):
    #get the data...    
    global x
    global xlab
    global cnt
    global toggle
    cnt += 1
    print 'Grabbing spectrum number %i'%cnt
    #time.sleep(2)
    #print 'Sleeping in %i for 2 Seconds'%cnt	# Adjust Sleep According to your Accumulation Setting
    inputs=[[],[],[],[],[],[],[],[]]
    cx=[] 
    cx_intrleav=[]
    self_intrleav=[]
    self=[[],[]]

    for input in range(8):
        inputs[input] = brd.read_int(bram_map[input], size=n_chans)
    
    if toggle == 0 :
      	t3 = time.time()
    	if (opts.anta > 0):
       		brd.write_uint('int_delay_a',opts.anta)
       		opts.anta += opts.incr
    	if (opts.antb > 0):
       		brd.write_uint('int_delay_b',opts.antb)
       		opts.antb += opts.incr
    	if (opts.antaf > 0):
       		brd.write_uint('fract_delay_a',opts.antaf)
       		opts.antaf += opts.incrf
    	if (opts.antbf > 0):
       		brd.write_uint('fract_delay_b',opts.antbf)
       		opts.antbf += opts.incrf
      	toggle = 1
    else :
      toggle = 0


#========================= FOR CROSS CORRELATION ============================#
    cx_intrleav = list((numpy.array(inputs[0][0:n_chans/4])+numpy.array(inputs[1][0:n_chans/4])*1j))
    cx = list((numpy.array(inputs[2][0:n_chans/4])+numpy.array(inputs[3][0:n_chans/4])*1j))
   
    #================== Interleaving Logic===================#
    j=0
    for i in range(1,(n_chans/2+1),2):                         # This is for interleaving Logic.
    	cx_intrleav.insert(i,cx[j])
        j=j+1
    #================== Interleaving Logic===================#

    # This is for Removing the discrepancy of Log for the input=0   Added By ~Sandeep
    for i in range(n_chans/2):
        if ( numpy.abs(cx_intrleav[i])<= 0):
                cx_intrleav[i] = 1.e-1
#========================= FOR CROSS CORRELATION ============================#



#========================= FOR SELF CORRELATION ============================#
    k=4 #self[1] =>input[4] & input[5] and self[1] => input[6] & input[7]
    for input in range(2):
        self_intrleav = list(inputs[input+k][0:n_chans/4])       # Converts tuple into a list for manipulation.
        j=0
        for i in range(1,(n_chans/2+1),2):                       # This is for interleaving Logic.
                self_intrleav.insert(i,inputs[input+k+1][j])
                j=j+1
        k=k+1
        self[input]=self_intrleav
#========================= FOR SELF CORRELATION ============================#
    for i in range(2):	
	pylab.subplot(411+i)
        pylab.ioff()
        pylab.hold(opts.hold)
        if opts.log:
        #pylab.semilogy(x,numpy.abs(cx_reord))
                pylab.semilogy(numpy.abs(self[i]))
                pylab.ylabel('Power (log scale, arbitrary units)')
        else:
            #pylab.plot(x,numpy.abs(cx_reord))
                pylab.plot(numpy.abs(self[i]))
                pylab.ylabel('Power (linear scale, arbitrary units)')
        #pylab.xlim(x[0],x[n_chans/2-1])
        #pylab.title('Correlations for baseline %s.'%opts.baseline)
        pylab.grid()
        pylab.title('Power: SELF Spectrum %s'%(ants[i]))



    pylab.subplot(413)
    pylab.ioff()
    pylab.hold(opts.hold)
    if opts.log: 
    #pylab.semilogy(x,numpy.abs(cx_reord))
      	pylab.semilogy(numpy.abs(cx_intrleav))
       	pylab.ylabel('Power (log scale, arbitrary units)')
    else: 
        #pylab.plot(x,numpy.abs(cx_reord))
       	pylab.plot(numpy.abs(cx_intrleav))
       	pylab.ylabel('Power (linear scale, arbitrary units)')
    #pylab.xlim(x[0],x[n_chans/2-1])
    #pylab.title('Correlations for baseline %s.'%opts.baseline)
    pylab.grid()
    pylab.title('Power: Cross-AB. Spectrum number %i'%(cnt))

    pylab.subplot(414)
    pylab.ioff()
    pylab.hold(opts.hold)
    pylab.plot(numpy.angle(cx_intrleav,deg=True))
    #pylab.plot(numpy.unwrap(numpy.angle(cx_intrleav,deg=True)))
    #pylab.xlim(x[0],x[n_chans/2-1])
    #pylab.xlim(x[0],x[n_chans/4-1])
    pylab.ylabel('Phase (unwrapped degrees)')
    pylab.grid()

    pylab.xlabel(xlab)
    manager.canvas.draw()
    return True

gobject.idle_add(updatefig)
pylab.show()
print 'Done with all'
 
