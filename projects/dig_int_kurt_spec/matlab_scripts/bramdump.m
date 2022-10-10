function [ bram_contents ] = bramdump( name_of_bram )
%bramdump - dump contents of 32bit x 2048 value ibob bram into matlab var
    u = udp('192.168.1.150', 'RemotePort',7, 'INPUT', 1024000);
    fopen(u);
    set(u,'DatagramTerminateMode', 'off');
    S=strcat('bramdump',32,name_of_bram,13)
    fwrite(u,S);
    
    while(get(u,'BytesAvailable') < 163841)
    end
    
    out = sscanf(char(fread(u,163841)), '%*s / %d -> %*s / 0b%s / %*d');
    
    fclose(u);
    delete(u);

    for i=0:2047,
        contents(i+1,1) = out( (i * 33) + 1 );
        contents(i+1,2) = bin2dec( char(out(((i*33) + 2):((i*33) + 33)))' ); 
    end
        
    bram_contents = contents;
end

