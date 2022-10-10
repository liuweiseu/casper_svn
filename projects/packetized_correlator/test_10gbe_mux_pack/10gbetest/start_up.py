#! /usr/bin/env python
""" 

"""
import corr.borph, time, sys, numpy, os, corr.cn_conf

def check_special_result(results):
    """Checks the returned list to make sure that all registers were successfully written to. Returns 'ok' or string outlining failure."""
    for b in enumerate(bees):
        if results[b[0]]['status'] != ('SUCCESS'):
            return('''Failed to program %s''' %(servers[b[0]]))
    return('''ok''')

def write_all(device, value):
    """Writes a value to the register 'device' for all BEE FPGAs."""
    rv = []
    for b in bees: rv.append(b.write(device, value))
    return rv

def read_all(device):
    """Reads a value from register 'device' for all BEE FPGAs."""
    rv = []
    for b in bees: rv.append(b.read(device))
    return rv

def check_result(results):
    """Checks the returned list to make sure that all registers were successfully written to. Returns 'ok' or string outlining failure."""
    for b in enumerate(bees):
        for f in fpgas:
            if results[b[0]][f]['status'] != ('SUCCESS'):
                return('''Failed to program %s on %s''' %(f,servers[b[0]]))
    return('''ok''')

if __name__ == '__main__':
    from optparse import OptionParser

    p = OptionParser()
    p.set_usage('start_single.py [options] [servers] ')
    p.set_description(__doc__)
    p.add_option('-p', '--period', dest='period', type='int', default=(2**27), 
        help='Period interval of packets. Default 2^27 (~1 every second)')
    p.add_option('-l', '--payload_len', dest='payload_len', type='int', default=100, 
        help='length of packet in 64 bit words. default: 100')

    opts, args = p.parse_args(sys.argv[1:])


servers = [s for s in args]
borphport = 9999
fpgas = ['FPGA0']

bees = [corr.borph.BorphClient(s,borphport) for s in servers]

if servers == []:
    print 'No server specified.\n\nExiting.'
    sys.exit()

print('Using servers: '),
print(servers)


print ('''P_UP Setting the payload length to %i...'''%(opts.payload_len)),
print (check_result(write_all('p_up_payload_len', opts.payload_len)))

print ('''P_UP Setting the period to %i...'''%(opts.period)),
print (check_result(write_all('p_up_period', opts.period)))

print ('''P_UP Starting...'''),
print (check_result(write_all('p_up_run', 1)))


print ('''P_MUX_UP Setting the payload length to %i...'''%(opts.payload_len)),
print (check_result(write_all('p_mux_up_payload_len', opts.payload_len)))

print ('''P_MUX_UP Setting the period to %i...'''%(opts.period)),
print (check_result(write_all('p_mux_up_period', opts.period)))

print ('''P_MUX_UP Starting...'''),
print (check_result(write_all('p_mux_up_run', 1)))


