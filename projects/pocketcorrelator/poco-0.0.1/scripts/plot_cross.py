#!/usr/bin/env python

'''
Plots the cross correlation for a user-defined input.
'''
import gtk, gobject, poco, time, numpy, struct, sys, math, matplotlib
matplotlib.use('GTKAgg')
from matplotlib import rcParams
import pylab


if __name__ == '__main__':
    from optparse import OptionParser

    p = OptionParser()
    p.set_usage('plot_cross.py [options] CONFIG FILE')
    p.set_description(__doc__)
    p.add_option('-b', '--baseline', dest='baseline', type='string', default='ab',
        help='''Specify the baseline to plot (ab,bc,cd etc). Default is ab.''')
    p.add_option('-l', '--log', dest='log', action='store_true', default=False,
        help='''Plot the power in logarithmic scale (requires some non-zero value signal).''')
    p.add_option('--hold', dest='hold', action='store_true', default=False,
        help='''Turn on hold. This will plot subsequent specra on top of each other.''')
    p.add_option('-f', '--freq', dest='freq', action='store_true', default=False,
        help='''Plot the input frequency on the X-axis rather than the channel number.''')

    opts, args = p.parse_args(sys.argv[1:])


    if args==[]:
        print 'Please specify a configuration file! \nExiting.'
        exit()

    config = poco.poco_conf.CorrConf(args[0])
    config_status = config.read_all()
    print '\n\nParsing config file %s...%s'%(args[0],config_status)
    sys.stdout.flush()
    if not config_status == 'OK': exit()

#sanity check:
assert opts.baseline[0] in ['a','b','c','d']
assert opts.baseline[1] in ['a','b','c','d']

if opts.baseline[0] == opts.baseline[1]:
    print 'You asked for an auto correlation. Try plot_auto.py instead.\n'
    exit()

if opts.baseline[0]>opts.baseline[1]:
    opts.baseline = opts.baseline[1]+opts.baseline[0]

print 'Trying to plot cross correlations amplitude and phase for baseline %s.'%opts.baseline

bram_map=['dir_x/%s/real'%opts.baseline,'dir_x/%s/imag'%opts.baseline]

port = config['connection']['port']
n_chans = config['n_chans']
cen_freq = config['adc_clk']*1000.*config['ddc_mix_freq']
bandwidth = config['adc_clk']*1000./config['ddc_decimation']


print 'Connecting to IBOB on port %s.'%port
brd = poco.ibob_serial.IbobClient(port)
cnt = 0

fig = pylab.figure(1)
manager = pylab.get_current_fig_manager()

if opts.freq:
    channel_size=float(bandwidth)/n_chans
    print 'Bandwidth: %f. Cent freq: %f. Channel size: %f'%(bandwidth,cen_freq,channel_size)
    x=((numpy.array(range(0,n_chans))*channel_size)+cen_freq-bandwidth/2)
    xlab=('Input Frequency (MHz)')
else:
    x=range(0,n_chans)
    xlab=('Input channel number')


def updatefig(*args):
    #get the data...    
    global x
    global xlab
    inputs=[[],[]]
    for input in range(2):
        inputs[input] = brd.read_int(bram_map[input], size=n_chans)
    
    cx=(numpy.array(inputs[0])+numpy.array(inputs[1])*1j)
    #Reverse the spectrum as the complex FFT outputs positive half of spectrum before negative half
    cx_reord=(numpy.concatenate((cx[512:1024], cx[0:512])))

    pylab.subplot(211)
    pylab.ioff()
    pylab.hold(opts.hold)

    if opts.log: 
        pylab.semilogy(x,numpy.abs(cx_reord))
        pylab.ylabel('Power (log scale, arbitrary units)')
    else: 
        pylab.plot(x,numpy.abs(cx_reord))
        pylab.ylabel('Power (linear scale, arbitrary units)')

    pylab.xlim(x[0],x[n_chans-1])
    pylab.title('Correlations for baseline %s.'%opts.baseline)
    pylab.grid()

    pylab.subplot(212)
    pylab.ioff()
    pylab.hold(opts.hold)
    pylab.plot(x,numpy.unwrap(numpy.angle(cx_reord,deg=True)))
    pylab.xlim(x[0],x[n_chans-1])
    pylab.ylabel('Phase (unwrapped degrees)')
    pylab.grid()

    pylab.xlabel(xlab)
    manager.canvas.draw()
    return True

gobject.idle_add(updatefig)
pylab.show()
print 'Done with all'
 
