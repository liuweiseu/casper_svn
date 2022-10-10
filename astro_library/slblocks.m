function blkStruct = slblocks
%SLBLOCKS Defines the Simulink library block representation
%   for the Xilinx Blockset.

% Copyright (c) 1998 Xilinx Inc. All Rights Reserved.

blkStruct.Name    = ['BEE2 Radio Astronomy Blockset'];
blkStruct.OpenFcn = '';
blkStruct.MaskInitialization = '';

blkStruct.MaskDisplay = ['disp(''BEE2 Radio Astronomy Blockset'')'];

% Define the library list for the Simulink Library browser.
% Return the name of the library model and the name for it
%
Browser(1).Library = 'pfb_library';
Browser(1).Name    = 'BEE2 PFB Blockset';

Browser(2).Library = 'fft_library';
Browser(2).Name    = 'BEE2 FFT Blockset';

Browser(3).Library = 'ddc_library';
Browser(3).Name    = 'BEE2 DDC Blockset';

Browser(4).Library = 'cic_library';
Browser(4).Name    = 'BEE2 CIC Blockset';

Browser(5).Library = 'correlator_library';
Browser(5).Name    = 'BEE2 Correlator Blockset';

Browser(6).Library = 'delay_lib';
Browser(6).Name    = 'BEE2 Delay Blockset';

Browser(7).Library = 'reorder_library';
Browser(7).Name    = 'BEE2 Reorder Blockset';

Browser(8).Library = 'adcscope_lib';
Browser(8).Name    = 'BEE2 ADC Scope Blockset';

blkStruct.Browser = Browser;

% End of slblocks.m
