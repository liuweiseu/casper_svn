'''
This Class provides a Python interface to TinySH over an RS232 link

Requires PySerial.

Author: Jason Manley
Date: 2008/11/20
Version: 0.0.1

'''
recv_buffer=4096*4

num_retries=10
bin_mode = chr(128)

import time, numpy, os, struct, serial

class IbobClient:
    def __init__(self,port):
        self.port = port
        self.cmds = []
        self.devs={}
        self.ser = serial.Serial(port=port,baudrate=115200,parity='N',stopbits=1,timeout=1,xonxoff=0,rtscts=0)
        self.flush_text_buffers()
        self._listdev()

    def flush_text_buffers(self):
        self.ser.write('\n')
        time.sleep(1)
        self.ser.flushInput()
        #self.ser.flushOutput()
        self.ser.write('\n')
        line = self.ser.read(9)
        if line != '\n\rIBOB % ': raise Exception("Bad Reply: '%s'"%line)

    def _listdev(self):
        """ List all the devices (registers, brams etc) available on the device and figure out their memory addresses. """
        self.ser.write('listdev\n')
        time.sleep(1)
        for i in range(3):
            line=self.ser.readline()
            #print line
        #time.sleep(5)
        while line != '\rIBOB % ' and line != '':
            #print line
            if line[2:11] !='<NO ADDR>' and line !='\n' and line != '\r': 
                name = line[16:-1]
                #print 'Trying to decode address of \n%s'%line,
                #for i in line:
                #    print ord(i),
                #print ''
                addr = int(line[2:12],16) 
                self.devs[name]=addr
            line = self.ser.readline()

    def _binary_read(self,size,addr,offset=0):
        """ Reads data from the IBOB in binary mode. 
        Offset and size are measured in 32 bit words. ie Requesting size of 1 will return 4 bytes.
        The IBOB addresses bytes, but the size is 32bit delimited."""
        num_pkts = int(numpy.ceil(size/(127.)))
        output = ''
        for i in range(num_pkts):
            st_offset = (i*127)+offset
            request_size = numpy.min([size-(i*127),127])
            cmd = chr((0<<7) + request_size)
            request_addr = struct.pack('>L',addr+(st_offset*4))
            self.ser.write(bin_mode + cmd + request_addr)
            output+=self.ser.read(request_size*4)
        return output

        #return struct.unpack('>L',ser.read(4))

    def _binary_write(self,size,addr,data):
        assert(size<128)
        assert(type(data)==str)
        cmd = chr((1<<7) + size)
        addr = struct.pack('>L',addr)
        self.ser.write(bin_mode + cmd + addr + data)
       
    def read_uint(self, register, size=1):
        out=struct.unpack('>%iL'%size,self._binary_read(size=size,addr=self.devs[register]))
        if size==1: return out[0]
        else: return out
    
    def read_int(self, register, size=1):
        out=struct.unpack('>%il'%size,self._binary_read(size=size,addr=self.devs[register]))
        if size==1: return out[0]
        else: return out
    
    def write_uint(self, register, data):
        self._binary_write(size=1,addr=self.devs[register],data=struct.pack('>L',data))
 
