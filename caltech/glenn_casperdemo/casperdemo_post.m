%% testbench_post.m
% Rename this file to <your model name>_post.m
% Automatically called by dosim after simulation completes

%% Add your code to post process the data as you like.

values = simout.signals.values;     % Grab the output values and make them easier to access.
syncout = values(:,1);      %Grab the sync signal.

nchan = 16;         %Set to number of channels in filterbank
[blah dvalid] = max(syncout);   %find first sync pulse
dlen = length(syncout);
maxspectra = floor((dlen-dvalid)/nchan);    %find the number of complete spectra
pfbout = reshape(values(dvalid+(1:(nchan*maxspectra)),2),nchan,maxspectra);
fftout = reshape(values(dvalid+(1:(nchan*maxspectra)),3),nchan,maxspectra);
pfboutpow = abs(pfbout);
fftoutpow = abs(fftout);
subplot(2,2,1);
plot(db(pfboutpow/max(max(pfboutpow)))')
title('Complex PFB')
axis([0 maxspectra -100 0])
subplot(2,2,3);
imagesc(db(pfboutpow/max(max(pfboutpow)))')
caxis([-50 0])
xlabel('freq bin')
ylabel('time')

subplot(2,2,2);
plot(db(fftoutpow/max(max(fftoutpow)))')
title('Complex FFT')
axis([0 maxspectra -50 0])
subplot(2,2,4);
imagesc(db(fftoutpow/max(max(fftoutpow)))')
caxis([-100 0]);
xlabel('freq bin')
ylabel('time')

values = realpfbout.signals.values;
syncout = values(:,1);      %Grab the sync signal.

%% Add your code to post process the data as you like.

nchan = 16;         %Set to number of channels in filterbank
[blah dvalid] = max(syncout);   %find first sync pulse
dlen = length(syncout);
maxspectra = floor((dlen-dvalid)/nchan);    %find the number of complete spectra
pfboutr = reshape(values(dvalid+(1:(nchan*maxspectra)),2),nchan,maxspectra);
fftoutr = reshape(values(dvalid+(1:(nchan*maxspectra)),3),nchan,maxspectra);
pfboutrpow = abs(pfboutr);
fftoutrpow = abs(fftoutr);
figure;
subplot(2,2,1);
plot(db(pfboutrpow/max(max(pfboutrpow)))')
title('Real PFB')
axis([0 maxspectra -100 0])
subplot(2,2,3);
imagesc(db(pfboutrpow/max(max(pfboutrpow)))')
caxis([-50 0])
xlabel('freq bin')
ylabel('time')

subplot(2,2,2);
plot(db(fftoutrpow/max(max(fftoutrpow)))')
title('Real FFT')
axis([0 maxspectra -50 0])
subplot(2,2,4);
imagesc(db(fftoutrpow/max(max(fftoutrpow)))')
caxis([-100 0]);
xlabel('freq bin')
ylabel('time')

figure
subplot(1,2,1);
plot(db(abs(fft(reshape(testch,nchan,length(testch)/nchan))))')
title('Expected result from successive FFTs of testch')
a=axis;
a(1:3) = [0 length(testch)/nchan -50];
axis(a);
subplot(1,2,2);
plot(db(abs(fft(reshape(real(testch),nchan,length(testch)/nchan))))')
title('Expected result from successive FFTs of real(testch)')
a=axis;
a(1:3) = [0 length(testch)/nchan -50];
axis(a);

nchan = 32;
values = cplxpfbwide.signals.values;
syncout = values(:,1);
[blah dvalid] = max(syncout);   %find first sync pulse
dlen = length(syncout);
maxspectra = floor((dlen-dvalid)/nchan)*ninp;    %find the number of complete spectra
pfboutw = reshape(reshape(values(dvalid+(1:(nchan*maxspectra/ninp)),2:5).',1,nchan*maxspectra),nchan,maxspectra);
figure
subplot(1,2,1);
pfbwpow = abs((pfboutw))/max(max(abs(pfboutw)));
plot(db(pfbwpow'));
title('PFB with multiple simultaneous inputs')
axis([0 maxspectra -50 0]);
subplot(1,2,2);
imagesc(db(pfbwpow'));
caxis([-50 0]);

