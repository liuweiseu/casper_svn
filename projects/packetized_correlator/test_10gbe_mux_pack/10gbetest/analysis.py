#!/usr/bin/env python

import numpy,pylab,os,sys,csv

datafile='bumblebee_pkt_loss.txt'
clk=200000000
n_pkts=1000000

f=open(datafile,'r')
c=csv.reader(f,delimiter=' ',skipinitialspace=True)

data=[]

while True:
    try: 
        out=(([int(i) for i in c.next()]))
        print out
        if out:
            data.append({'pkt_size':out[0],'pkt_period':out[1],'meas_rate':out[2],'ave_drops_per_mill':numpy.mean(out[4:8])})
    except StopIteration:
        print 'No more data' 
        break
    except ValueError:
        print 'skipped a line'
        continue

pkt10={'tx_rate_mbps':[],'rx_rate_mbps':[],'tx_rate_pkts':[],'rx_rate_pkts':[]}
pkt100={'tx_rate_mbps':[],'rx_rate_mbps':[],'tx_rate_pkts':[],'rx_rate_pkts':[]}
pkt1000={'tx_rate_mbps':[],'rx_rate_mbps':[],'tx_rate_pkts':[],'rx_rate_pkts':[]}

for d in data:
    if d['pkt_size']==10:
        pkt10['tx_rate_mbps'].append(clk/d['pkt_period']*d['pkt_size']*64/1024/1024)
        pkt10['rx_rate_mbps'].append((clk/d['pkt_period']*d['pkt_size']*64/1024/1024)*(n_pkts-d['ave_drops_per_mill'])/n_pkts)
        pkt10['tx_rate_pkts'].append(clk/d['pkt_period'])
        pkt10['rx_rate_pkts'].append(clk/d['pkt_period']*(n_pkts-d['ave_drops_per_mill'])/n_pkts)

    elif d['pkt_size']==100:
        pkt100['tx_rate_mbps'].append(clk/d['pkt_period']*d['pkt_size']*64/1024/1024)
        pkt100['rx_rate_mbps'].append((clk/d['pkt_period']*d['pkt_size']*64/1024/1024)*(n_pkts-d['ave_drops_per_mill'])/n_pkts)
        pkt100['tx_rate_pkts'].append(clk/d['pkt_period'])
        pkt100['rx_rate_pkts'].append(clk/d['pkt_period']*(n_pkts-d['ave_drops_per_mill'])/n_pkts)

    elif d['pkt_size']==1000:
        pkt1000['tx_rate_mbps'].append(clk/d['pkt_period']*d['pkt_size']*64/1024/1024)
        pkt1000['rx_rate_mbps'].append((clk/d['pkt_period']*d['pkt_size']*64/1024/1024)*(n_pkts-d['ave_drops_per_mill'])/n_pkts)
        pkt1000['tx_rate_pkts'].append(clk/d['pkt_period'])
        pkt1000['rx_rate_pkts'].append(clk/d['pkt_period']*(n_pkts-d['ave_drops_per_mill'])/n_pkts)

pylab.plot(pkt10['tx_rate_mbps'],pkt10['rx_rate_mbps'],label='Pkt size=80 Bytes')
pylab.plot(pkt100['tx_rate_mbps'],pkt100['rx_rate_mbps'],label='Pkt size=800 Bytes')
pylab.plot(pkt1000['tx_rate_mbps'],pkt1000['rx_rate_mbps'],label='Pkt size=8000 Bytes')
pylab.xlabel('Transmit Rate (Mbps)')
pylab.ylabel('Receive Rate (Mbps)')
pylab.legend(loc='upper left')
pylab.title('Data throughput for various packet payload sizes')

pylab.figure()
pylab.plot(pkt10['tx_rate_pkts'],pkt10['rx_rate_pkts'],label='Pkt size=80 Bytes')
pylab.plot(pkt100['tx_rate_pkts'],pkt100['rx_rate_pkts'],label='Pkt size=800 Bytes')
pylab.plot(pkt1000['tx_rate_pkts'],pkt1000['rx_rate_pkts'],label='Pkt size=8000 Bytes')
pylab.xlabel('Transmit Rate (Packets per second)')
pylab.ylabel('Receive Rate (Packets per second)')
pylab.legend(loc='upper left')
pylab.title('Packet throughput for various packet payload sizes')

pylab.show()

