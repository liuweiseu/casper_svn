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

def get_gbe_conf(port,start_addr,fpga):
    """Generates a 10GbE configuration string starting from ip w.x.y.z"""
    gbe_conf = """begin\n\tmac = 00:12:6D:AE:%02X:%02X\n\tip = %i.%i.%i.%i\n\tgateway = %i.%i.%i.1\n\tport = %i\nend\n"""
    ip = start_addr + fpga
    ip_1 = (ip/(2**24))
    ip_2 = (ip%(2**24))/(2**16)
    ip_3 = (ip%(2**16))/(2**8)
    ip_4 = (ip%(2**8))
    return gbe_conf % (ip_3,ip_4,ip_1,ip_2,ip_3,ip_4,ip_1,ip_2,ip_3,port)

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
    p.set_usage('init_corr.py [options] [servers] ')
    p.set_description(__doc__)

    opts, args = p.parse_args(sys.argv[1:])


servers = [s for s in args]
borphport = 9999
fpgas = ['FPGA0']
gbe_start_ip = 192*(2**24)+168*(2**16)+4*(2**8)+10
gbe_port = 8888
gbe_target_ip=192*(2**24)+168*(2**16)+4*(2**8)+38

bees = [corr.borph.BorphClient(s,borphport) for s in servers]

if servers == []:
    print 'No server specified.\n\nExiting.'
    sys.exit()

print('Using servers: '),
print(servers)


# Set UDP TX data port
print ('''Setting the 10GbE target port to %i'''%(gbe_port)),
print (check_result(write_all('gbe_port', gbe_port)))


# Set UDP Target ip
print ('''Setting the 10GbE target IP address for P_DOWN...'''),
print (check_result(write_all('p_down_ip_addr', gbe_target_ip)))
print ('''Setting the 10GbE target IP address for P_UP...'''),
print (check_result(write_all('p_up_ip_addr', gbe_target_ip)))

# Set UDP Target ip
print ('''Setting the 10GbE target IP address for P_MUX_DOWN...'''),
print (check_result(write_all('p_mux_down_ip_addr', gbe_target_ip)))
print ('''Setting the 10GbE target IP address for P_MUX_UP...'''),
print (check_result(write_all('p_mux_up_ip_addr', gbe_target_ip)))

# Configure the 10 GbE data ports
print('''Configuring the 10GbE cores and verifying...'''),
f = 0
for b,bee in enumerate(bees):
    for f,fpga in enumerate(fpgas):
        new_gbe_conf=get_gbe_conf(port=gbe_port,start_addr=gbe_start_ip,fpga=(b*4)+f)
        out=bee.binary_write('gbe', new_gbe_conf, fpgas=[fpga])
        if (out[fpga]['status'] != 'SUCCESS'):
            print('''Failed to program %s on %s''' %(fpga,servers[b])),out 
        if (bee.binary_read('gbe',num=-1)[fpga]['data'] != new_gbe_conf): 
            print('''Failed verification for %s on %s''' %(fpga,servers[b]))
        f += 1
print('ok')
print 'Gbe now set to:'
print bee.binary_read('gbe',num=-1)[fpga]['data']
