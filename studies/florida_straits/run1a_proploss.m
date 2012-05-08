% [ proploss, eigenrays ] = load_proploss('run1a_proploss.nc');
range = 3:1:30 ;
figure; subplot(211)
plot(range,-proploss.intensity,'ro')
set(gca,'xlim',[0 80])
set(gca,'Ylim',[-130 -60])
set(gca,'XTick',[])
set(gca,'YTick',[])

subplot(212)
plot(range,-proploss.source_az,'ro')
set(gca,'YLim',[-20 40])
set(gca,'xlim',[0 80])
set(gca,'XTick',[])
set(gca,'YTick',[])