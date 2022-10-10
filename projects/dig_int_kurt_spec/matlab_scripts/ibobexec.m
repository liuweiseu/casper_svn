function [ command_output ] = ibobexec( ip, command )
%ibobexec Matlab function to execute a tinysh command on an ibob
warning off;
    %open udp socket with maximum commmand depth of 1 MB (easy to change)
    u = udp(ip, 'RemotePort',7, 'INPUT', 1024000);

    %connect socket
    fopen(u);
    
    %formulate command for robust operation (null + padding) - see udp memo
    S=strcat(255,'0000000',command,32,10);

    %execute command
    fwrite(u,S);

    contents=[];
    packtype=0;
    
    %wait until done packet - packtype == 2
    while(packtype ~= 2)
        curpacket = fread(u);
        contents=vertcat(contents,curpacket(9:end));
        if (numel(curpacket) > 2), packtype = curpacket(7); end   
    end
    
    %replace the line feeds with spaces for pretty printing
    contents=strrep(contents,10,32);

    %close up shop
    fclose(u);
    delete(u);
        
    
    command_output = char(contents);

end

