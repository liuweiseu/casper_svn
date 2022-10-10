% testbench_pre.m
% Rename this file to <your model name>_pre.m
% Called by dosim before executing simulation to set up workspace stimuli

%%%% Setup a test chirp function useful for analyzing filterbanks.
nchanmax = 32;     %Set to greater than the number of filterbank channels
                    %This ensures the chirp sweeps smoothly across the
                    %channels.
ts = 1;             %Sampling interval. Should be 1
fs = 1/(2*ts);      %Nyquist frequency
ns = (2^9)*nchanmax;    %Total number of samples. 2^9 factor determined 
                        %experimentally to give enough points per channel
t = (((0:(ns-1)))*ts)'; %Time vector
flo = 0;       %Low chirp frequency
beta   = (.9*fs-flo).*(max(t).^(-1));   %This code copied from built in chirp function
testch = exp(i*2*pi * ( beta./(2).*(t.^(2)) + flo.*t)); % adapted to produce a complex sinusoid

%%%% SYNC SIGNAL
sync_in = zeros(length(t),1);   %Sync signal
sync_in(50) = 1;                 %Need at least one sync pulse to get things going

%%%% REAL INPUTS
% Initialize dinr to be an array with at least as many rows as simulation
% time steps, and as many columns as real inputs needed.
dinr = [.99*real(testch), .99*real(testch)];

%%%% COMPLEX INPUTS
% Initialize dinr to be an array with at least as many rows as simulation
% time steps, and as many columns as real inputs needed.
% NOTE: Real and imaginary parts of complex inputs should be in [-1,1)
% range because they are represented in n_(n-1) format.
ninp = 4;   %number of simultaneous inputs
nblock = length(testch)/ninp;   %number of blocks of simultaneous inputs
cplxblock = repmat(reshape(testch,ninp,nblock).',[ninp, 1]); % Reshape blocks the data and repmat fills it out to the length of the single input cases
dinc = .99*[testch, testch, cplxblock];


%%%% Form the input structures that Simulink requires
sync_input.time = [];
sync_input.signals(1).values = sync_in;

real_inputs.time = [];  %Time is implicit
real_inputs.signals(1).values = dinr;

cplx_inputs.time = [];
cplx_inputs.signals(1).values = dinc;