cursys = gcb;
disp(cursys);

delete_lines(cursys);

counterBitwidth = log2(numSpecs)+log2(length);
smallRowLength = 16*4;  % 16 bursts of 4*72 bits  
bigRowLength = 512/4.; 	% 16 blocks in 1Row : 128 blocks in  1Row*4banks*2ranks  = 512 spectrums     

% # of bits necessary to go through one small row! counts through the columns!
if length < 16
    smallRowBits = log2(length)+2;
else 
    smallRowBits = log2(smallRowLength);
end

% # of bits necessary to go through one big row! counts through the columns!
if numSpecs < 512
    bigRowBits = log2(numSpecs)-2;
else
    bigRowBits = log2(bigRowLength);
end

smallColLength = ceil(length/16.);
if smallColLength > 1
	smallColBits = log2(smallColLength);
else 
	smallColBits = 0;
end

bigColLength = ceil(numSpecs/512.);
if bigColLength > 1
    bigColBits = log2(bigColLength);
else
    bigColBits = 0;
end

%inports = find_system(cursys, 'lookUnderMasks','on', 'FollowLinks','on', 'SearchDepth',1, 'BlockType','Inport');
%outports = find_system(cursys, 'lookUnderMasks','on', 'FollowLinks','on', 'SearchDepth',1, 'BlockType','Outport');

reuse_block(cursys, 'sync', 'built-in/inport', {'Position', [30 65 60 79], 'Port', '1'});
reuse_block(cursys, 'write_addr', 'built-in/outport', {'Position', [765   150   795  170], 'Port', '1'});
reuse_block(cursys, 'read_addr', 'built-in/outport', {'Position', [765   550   795  570], 'Port', '2'});
reuse_block(cursys, 'sync_out', 'built-in/outport', {'Position', [765 450 795 470], 'Port', '3'});
reuse_block(cursys, 'Counter', 'xbsIndex_r3/Counter', ...
        {'Position', [95    41   145   94], 'n_bits', 'counterBitwidth', 'cnt_type', 'Free Running', ...
        'arith_type', 'Unsigned', 'en', 'off', 'rst', 'on'}); 

    
%///////////////////////////////////////////
%
% Write address components!
%
%///////////////////////////////////////////

reuse_block(cursys, 'smallRowCount', 'xbsIndex_r3/Slice', ...
       {'Position', [165    227   195    243], 'mode', 'Lower Bit Location + Width', ...
       'nbits', 'smallRowBits'});        
reuse_block(cursys, 'bigRowCount', 'xbsIndex_r3/Slice', ...
       {'Position', [165    127   195    143], 'mode', 'Lower Bit Location + Width', ...
       'nbits', 'bigRowBits','bit0', 'smallRowBits+smallColBits'});           
reuse_block(cursys, 'concat0', 'xbsIndex_r3/Concat', ...
       {'Position', [665    107   715    213], 'num_inputs', '7'});  
reuse_block(cursys, 'rankCount', 'xbsIndex_r3/Slice', ...
       {'Position', [365    77   395    93], 'mode', 'Upper Bit Location + Width', ...
       'nbits', '1'});     
reuse_block(cursys, 'bankCount', 'xbsIndex_r3/Slice', ...
       {'Position', [365    97   395    113], 'mode', 'Upper Bit Location + Width', ...
       'nbits', '2', 'bit1', '-1'});  
reuse_block(cursys, 'rowUpperBits', 'xbsIndex_r3/Slice', ...
       {'Position', [365    117  395    133], 'mode', 'Lower Bit Location + Width', ...
       'nbits', '4'});  
reuse_block(cursys, 'cast2', 'xbsIndex_r3/Convert', ...
       {'Position', [265    77   295    93], 'arith_type', 'Unsigned', 'n_bits', '7', 'bin_pt', '0' ...
       ,'latency', '0'});
reuse_block(cursys, 'cast0', 'xbsIndex_r3/Convert', ...
       {'Position', [565    317  595    333], 'arith_type', 'Unsigned', 'n_bits', '14', 'bin_pt', '0' ...
       ,'latency', '0'});
reuse_block(cursys, 'cast1', 'xbsIndex_r3/Convert', ...
       {'Position', [465    197  495    213], 'arith_type', 'Unsigned', 'n_bits', '6', 'bin_pt', '0' ...
       ,'latency', '0'});

%/////////////////////////////////////////// 

reuse_block(cursys, 'lowerBits', 'xbsIndex_r3/Constant', ...
       {'Position', [265    257   295    273], 'Const', '0', 'n_bits', '3','explicit_period', 'on'});   
reuse_block(cursys, 'const1', 'xbsIndex_r3/Constant', ...
       {'Position', [565    67   595    83], 'Const', '0', 'n_bits', '2'...
        ,'explicit_period', 'on'});   
        
%///////////////////////////////////////////




%///////////////////////////////////////////
%
% Read address components!
%
%///////////////////////////////////////////

reuse_block(cursys, 'concat7', 'xbsIndex_r3/Concat', ...
       {'Position', [665    607   715    713], 'num_inputs', '8'});  
reuse_block(cursys, 'bigRowCount_read', 'xbsIndex_r3/Slice', ...
       {'Position', [165    677   195    693], 'mode', 'Lower Bit Location + Width', ...
       'nbits', 'bigRowBits', 'bit0' , '2'});
reuse_block(cursys, 'littleCount_read', 'xbsIndex_r3/Slice', ...
       {'Position', [165    727   195    743], 'mode', 'Lower Bit Location + Width', ...
       'nbits', '2'});
reuse_block(cursys, 'smallRowCount_read', 'xbsIndex_r3/Slice', ...
       {'Position', [165    577   195    593], 'mode', 'Lower Bit Location + Width', ...
       'nbits', 'smallRowBits-2','bit0','bigRowBits+bigColBits+2'}); 
reuse_block(cursys, 'cast3', 'xbsIndex_r3/Convert', ...
       {'Position', [235    577   265    593], 'arith_type', 'Unsigned', 'n_bits', '4', 'bin_pt', '0' ...
       ,'latency', '0'});
reuse_block(cursys, 'cast4', 'xbsIndex_r3/Convert', ...
       {'Position', [235    677   265    693], 'arith_type', 'Unsigned', 'n_bits', '7', 'bin_pt', '0' ...
       ,'latency', '0'});       
reuse_block(cursys, 'rankCount_read', 'xbsIndex_r3/Slice', ...
       {'Position', [335    627   365    643], 'mode', 'Upper Bit Location + Width', ...
       'nbits', '1'});     
reuse_block(cursys, 'bankCount_read', 'xbsIndex_r3/Slice', ...
       {'Position', [335    677   365    693], 'mode', 'Upper Bit Location + Width', ...
       'nbits', '2', 'bit1', '-1'});  
reuse_block(cursys, 'rowUpperBits_read', 'xbsIndex_r3/Slice', ...
       {'Position', [335    727   365    743], 'mode', 'Lower Bit Location + Width', ...
       'nbits', '4'});
reuse_block(cursys, 'cast5', 'xbsIndex_r3/Convert', ...
       {'Position', [435    477   465    493], 'arith_type', 'Unsigned', 'n_bits', '14', 'bin_pt', '0' ...
       ,'latency', '0'});
   




% Dynamic components!
       
if bigColBits > 0 && smallColBits > 0     
    
    % Write!
    reuse_block(cursys, 'bigColCount', 'xbsIndex_r3/Slice', ...
           {'Position', [165    77   195    93], 'mode', 'Lower Bit Location + Width', ...
            'nbits', 'bigColBits','bit0', 'smallRowBits+smallColBits+bigRowBits'}); 
    reuse_block(cursys, 'smallColCount', 'xbsIndex_r3/Slice', ...
           {'Position', [165    177   195    193], 'mode', 'Lower Bit Location + Width', ...
            'nbits', 'smallColBits','bit0', 'smallRowBits'});   
    reuse_block(cursys, 'concat1', 'xbsIndex_r3/Concat', ...
       {'Position', [465    307   515    350], 'num_inputs', '2'});          
    add_line(cursys, 'Counter/1', 'bigColCount/1');
    add_line(cursys, 'Counter/1', 'smallColCount/1');
    add_line(cursys, 'bigColCount/1', 'concat1/1');
    add_line(cursys, 'smallColCount/1', 'concat1/2');
    add_line(cursys, 'concat1/1', 'cast0/1');
    
    
    % Read!
    reuse_block(cursys, 'bigColCount_read', 'xbsIndex_r3/Slice', ...
       {'Position', [165    627   195    643], 'mode', 'Lower Bit Location + Width', ...
       'nbits', 'bigColBits', 'bit0', 'bigRowBits+2'});
    reuse_block(cursys, 'smallColCount_read', 'xbsIndex_r3/Slice', ...
       {'Position', [165    527   195    543], 'mode', 'Lower Bit Location + Width', ...
       'nbits', 'smallColBits','bit0','bigRowBits+bigColBits+smallRowBits' });
   	reuse_block(cursys, 'concat2', 'xbsIndex_r3/Concat', ...
       {'Position', [335    477   365    493], 'num_inputs', '2'});        
    add_line(cursys, 'Counter/1', 'bigColCount_read/1');
    add_line(cursys, 'Counter/1', 'smallColCount_read/1');
    add_line(cursys, 'smallColCount_read/1', 'concat2/2');
    add_line(cursys, 'bigColCount_read/1', 'concat2/1');
    add_line(cursys, 'concat2/1', 'cast5/1');   
       
       
       
elseif   bigColBits == 0 && smallColBits > 0      
    
    % Write!
    reuse_block(cursys, 'smallColCount', 'xbsIndex_r3/Slice', ...
           {'Position', [165    177   195    193], 'mode', 'Lower Bit Location + Width', ...
            'nbits', 'smallColBits','bit0', 'smallRowBits'});   
    add_line(cursys, 'Counter/1', 'smallColCount/1');
    add_line(cursys, 'smallColCount/1', 'cast0/1'); 
    
    % Read!
    reuse_block(cursys, 'smallColCount_read', 'xbsIndex_r3/Slice', ...
       {'Position', [165    527   195    543], 'mode', 'Lower Bit Location + Width', ...
       'nbits', 'smallColBits','bit0','bigRowBits+bigColBits+smallRowBits'});
    add_line(cursys, 'Counter/1', 'smallColCount_read/1');
    add_line(cursys, 'smallColCount_read/1', 'cast5/1');
    
        
elseif  bigColBits >0 && smallColBits == 0 
    
    % Write!
    reuse_block(cursys, 'bigColCount', 'xbsIndex_r3/Slice', ...
           {'Position', [165    77   195    93], 'mode', 'Lower Bit Location + Width', ...
            'nbits', 'bigColBits','bit0', 'smallRowBits+smallColBits+bigRowBits'});            
    add_line(cursys, 'Counter/1', 'bigColCount/1');
    add_line(cursys, 'bigColCount/1', 'cast0/1');
    
    % Read!
    reuse_block(cursys, 'bigColCount_read', 'xbsIndex_r3/Slice', ...
       {'Position', [165    627   195    643], 'mode', 'Lower Bit Location + Width', ...
       'nbits', 'bigColBits', 'bit0', 'bigRowBits+2'}); 
    add_line(cursys, 'Counter/1', 'bigColCount_read/1');
    add_line(cursys, 'bigColCount_read/1', 'cast5/1');
    
    
else 
    
    % Write!
    reuse_block(cursys, 'const0', 'xbsIndex_r3/Constant', ...
       {'Position', [365    317   395    333], 'Const', '0', 'n_bits', '2'...
        ,'explicit_period', 'on'});
    add_line(cursys, 'const0/1', 'cast0/1');
    
    % Read!
    add_line(cursys, 'const0/1', 'cast5/1');
        
end       
       
add_line(cursys, 'sync/1', 'Counter/1');


add_line(cursys, 'Counter/1', 'smallRowCount/1');
add_line(cursys, 'lowerBits/1', 'concat0/7');
add_line(cursys, 'cast0/1', 'concat0/3');
add_line(cursys, 'cast1/1', 'concat0/6');
add_line(cursys, 'rankCount/1', 'concat0/4');
add_line(cursys, 'smallRowCount/1', 'cast1/1');
add_line(cursys, 'rowUpperBits/1', 'concat0/5');
add_line(cursys, 'bankCount/1', 'concat0/2');
add_line(cursys, 'const1/1', 'concat0/1');
add_line(cursys, 'bigRowCount/1', 'cast2/1');
add_line(cursys, 'cast2/1', 'bankCount/1');
add_line(cursys, 'cast2/1', 'rankCount/1');
add_line(cursys, 'cast2/1', 'rowUpperBits/1');
add_line(cursys, 'Counter/1', 'bigRowCount/1');
add_line(cursys, 'concat0/1', 'write_addr/1');
add_line(cursys, 'sync/1', 'sync_out/1');


% Read address!
add_line(cursys, 'Counter/1', 'smallRowCount_read/1');
add_line(cursys, 'Counter/1', 'bigRowCount_read/1');
add_line(cursys, 'Counter/1', 'littleCount_read/1');
add_line(cursys, 'smallRowCount_read/1', 'cast3/1');
add_line(cursys, 'bigRowCount_read/1', 'cast4/1');
add_line(cursys, 'cast4/1', 'rankCount_read/1');
add_line(cursys, 'cast4/1', 'bankCount_read/1');
add_line(cursys, 'cast4/1', 'rowUpperBits_read/1');
add_line(cursys, 'concat7/1', 'read_addr/1');
add_line(cursys, 'const1/1', 'concat7/1');
add_line(cursys, 'bankCount_read/1', 'concat7/2');
add_line(cursys, 'cast5/1', 'concat7/3');
add_line(cursys, 'rankCount_read/1', 'concat7/4');
add_line(cursys, 'rowUpperBits_read/1', 'concat7/5');
add_line(cursys, 'cast3/1', 'concat7/6');
add_line(cursys, 'littleCount_read/1', 'concat7/7');
add_line(cursys, 'lowerBits/1', 'concat7/8');








clean_blocks(cursys);
%clean_ports(cursys, inports);
%clean_ports(cursys, outports);




