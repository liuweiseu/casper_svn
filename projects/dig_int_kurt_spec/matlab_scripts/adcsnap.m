function [ command_output ] = ibobexec( ip, command )
%ibobexec Matlab function to execute a tinysh command on an ibob
warning off;
    %open udp socket with maximum commmand depth of 1 MB (easy to change)
    v = udp(ip, 'RemotePort',7, 'INPUT', 10240000);

    %connect socket
    fopen(v);
    
    %formulate command for robust operation (null + padding) - see udp memo
    S=strcat(255,'0000000',command,32,10);

    %execute command
    fwrite(v,S);

    contents=[];
    packtype=0;
  
  pause(2)
  
    %wait until done packet - packtype == 2
    while(packtype ~= 2)
        curpacket = fread(v);
        contents=vertcat(contents,curpacket(9:end));
        if (numel(curpacket) > 2), packtype = curpacket(7); end   
    end
    
    %replace the line feeds with spaces for pretty printing
    %contents=strrep(contents,10,32);
%get(v)
    %close up shop
    fclose(v);
    delete(v);
        
    
    %command_output = char(contents);
    
     out = sscanf(char(contents), '%*s / %d -> %*s / 0b%s / %*s');
    

size(contents)
size(out)
   for i=0:2047,
   bram(i+1,1) = out( (i * 33) + 1 );
    %contents(i+1,2) = bin2dec( char(out(((i*33) + 2):((i*33) + 33)))' ); 
    bram(i+1,2) = bin2dec( char(out(((i*33) + 2):((i*33) + 33)))' );
   
   %out( (i * 33) + 1 )
   
   a = char(out(((i*33) + 2):((i*33) + 9)))';
   b= char(out(((i*33) + 10):((i*33) + 17)))';
   c = char(out(((i*33) + 18):((i*33) + 25)))';
   d = char(out(((i*33) + 26):((i*33) + 33)))';
   
   
   adcval(i+1,1) = b2scomp(a);
   adcval(i+1,2) = b2scomp(b);
   adcval(i+1,3) = b2scomp(c);
   adcval(i+1,4) = b2scomp(d);
   
   %strcat('first 8 bits ',char(out(((i*33) + 2):((i*33) + 9)))')
   %strcat('second 8 bits ',char(out(((i*33) + 10):((i*33) + 17)))')
   %strcat('third 8 bits ',char(out(((i*33) + 18):((i*33) + 25)))')
   %strcat('fourth 8 bits ',char(out(((i*33) + 26):((i*33) + 33)))')
   
   end
command_output = adcval;
end

