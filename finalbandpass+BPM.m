file=fopen('cleanECG180Hz.txt','r');
clean=fscanf(file,'%f');
fclose(file);
file=fopen('noisyECG180Hz.txt','r');
noisy=fscanf(file,'%f');
fclose(file);
bandFilter=bandpass(noisy,[1 80],360);
b=(1/3)*ones(1,3); %b=window size
avg=filter(b,1,bandFilter);
subplot(3,1,1);
plot(bandFilter);
title('BANDPASS 1-80 HZ');
subplot(3,1,2);
plot(avg);
title('BANDPASS 1-80 HZ MOVING AVERAGE');

beat_count=0;
fs=360; % fs is sampling of input freq 180Hz
j=1;
for i=2:length(bandFilter)-1
    if (bandFilter(i) > bandFilter(i-1) && bandFilter(i) > bandFilter(i+1) && bandFilter(i) > 1)
        beat_count = beat_count + 1;
    end
    N=length(1:j);
    duration_in_seconds=N/fs;
    duration_in_minutes=duration_in_seconds/60;
    BPM(j)=beat_count/duration_in_minutes;
    j=j+1;
end
subplot(3,1,3);
plot(BPM);
title('CHANGING HEART RATE');