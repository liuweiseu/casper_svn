#!/usr/bin/env python

'''
Plots the auto correlations for all four inputs.
'''
import gtk, gobject, poco, time, numpy, struct, sys, math, matplotlib
matplotlib.use('GTKAgg')
from matplotlib import rcParams
import pylab

bram_map=['dir_x/aa/real','dir_x/bb/real','dir_x/cc/real','dir_x/dd/real']

if __name__ == '__main__':
    from optparse import OptionParser

    p = OptionParser()
    p.set_usage('plot_auto.py CONFIG FILE')
    p.set_description(__doc__)
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

port = config['connection']['port']
n_chans = config['n_chans']
cen_freq = config['adc_clk']*1000.*config['ddc_mix_freq']
bandwidth = config['adc_clk']*1000./config['ddc_decimation']


print 'Connecting to IBOB on port %s.'%port
brd = poco.ibob_serial.IbobClient(port)

fig = pylab.figure(1)
manager = pylab.get_current_fig_manager()
cnt = 0

if opts.freq:
    channel_size=float(bandwidth)/n_chans
    print 'Bandwidth: %f. Cent freq: %f. Channel size: %f'%(bandwidth,cen_freq,channel_size)
    x=((numpy.array(range(0,n_chans))*channel_size)+cen_freq-bandwidth/2)
    xlab=('Input Frequency (MHz)')
else:
    x=range(0,n_chans)
    xlab=('Input channel number')


def updatefig(*args):
    global cnt
    cnt += 1
    print 'Grabbing spectrum number %i'%cnt
    #get the data...    
    inputs=[[],[],[],[]]
    self_reord=[]
    for input in range(4):
        inputs[input] = brd.read_uint(bram_map[input], size=n_chans)

    pylab.figure(num=1,figsize=(10,10))
    pylab.ioff()
    ymax=0
    ax=[]
    for input in range(4):
        self_reord.append (numpy.concatenate((inputs[input][512:1024], inputs[input][0:512])))
        ax.append(pylab.subplot(411+input))
        pylab.hold(opts.hold)

        if opts.log:
            pylab.semilogy(x,self_reord[input])
            pylab.ylabel('Power (log scale, arbitrary units)')

        else:
            pylab.plot(self_reord[input])
            pylab.ylabel('Power (linear scale, arbitrary units)')

        pylab.grid()
        #pylab.setp(ax[input].get_xticklabels(),visible=False)
        pylab.ylabel('Power (arbitrary units)')
        pylab.title('Power: Input %i. Spectrum number %i'%(input,cnt))
        if ymax<(ax[input].axes.yaxis.get_data_interval()[-1]): 
            ymax=(ax[input].axes.yaxis.get_data_interval()[-1])

    for i in range(4):
        ax[i].set_ylim(0,ymax*1.1)

    pylab.setp(ax[input].get_xticklabels(),visible=True)
    pylab.xlabel(xlab)
    pylab.xlim(x[0],x[n_chans-1])
    print 'Ymax: %i'%ymax
        
    pylab.draw()
    return True

gobject.idle_add(updatefig)
pylab.show()
print 'Done with all' 
