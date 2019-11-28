file=fopen('noisyECG180Hz.txt','r');
noisy=fscanf(file,'%f');
fclose(file);

file=fopen('cleanECG180Hz.txt','r');
clean=fscanf(file,'%f');
fclose(file);
 
%bandFilter1=bandpass(noisy,[1 20],360);
bandFilter2=bandpass(noisy,[1 40],360);
bandFilter3=bandpass(noisy,[1 80],360);
bandFilter4=bandpass(noisy,[1 100],360);
%bandFilter5=bandpass(noisy,[1 120],360);

b=(1/3)*ones(1,3);
%avg1=filter(b,1,bandFilter1);
%avg2=filter(b,1,bandFilter2);
avg3=filter(b,1,bandFilter3);
avg4=filter(b,1,bandFilter4);
%avg5=filter(b,1,bandFilter5);


clean=clean+1;
xpram=5;
subplot(xpram,1,1);
plot(clean);
%subplot(xpram,1,2);
%plot(bandFilter1);
%subplot(xpram,1,2);
%plot(bandFilter2);
subplot(xpram,1,2);
plot(bandFilter3);
subplot(xpram,1,3);
plot(avg3);
%subplot(xpram,1,6);
%plot(bandFilter5);
subplot(xpram,1,4);
plot(bandFilter4);
subplot(xpram,1,5);
plot(avg4);