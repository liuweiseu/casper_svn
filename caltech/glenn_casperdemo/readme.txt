casperdemo - Demonstration of some CASPER blocks
Glenn Jones
Oct. 8, 2007

This is hopefully a self contained (assuming the CASPER libraries are already installed in Matlab/simulink) demonstration of some CASPER blocks. Currently, the various PFB and FFT blocks are included in different configurations. These should provide some "documentation by example".

After changing to the directory containing these files, open casperdemo.mdl and run dosim from the Matlab prompt. The simulation will take some time to run (5 minutes on my computer) and then produce some plots of the outputs. Explore the subsystems and stimuli (casperdemo_pre.m) and post processing (casperdemo_post.m) to get an idea of how to interface with the blocks.