% std14profile.m - create sound speed profile from std014 site

% translate std014 profiles into 3-D space

clear all; close all;
load std14raw
latitude = unique(latitude) ; N = length(latitude) ;
longitude = unique(longitude) ; M = length(longitude) ;
depth = -svp0001(:,1)'; D = length(depth) ;

profile = reshape(profile,N,M)';
speed = zeros(D,N,M);
for n = 1:N
    for m = 1:M
        eval(sprintf('speed(:,n,m)=svp%04d(:,2);',profile(n,m)));
    end
end
plot( squeeze(speed(:,1,:)), depth ) ;

save std14profile latitude longitude depth speed

% create CDL file for import into netCDF

fo = fopen('std14profile.cdl','w');
fprintf(fo,'netcdf std14profile {\n');
fprintf(fo,'dimensions:\n');
fprintf(fo,'\ttime = 1, depth = %d, lat = %d, lon = %d;\n',D,N,M);
fprintf(fo,'variables:\n');
fprintf(fo,'\tfloat time(time), depth(depth), lat(lat), lon(lon), speed(time,depth,lat,lon);\n');
fprintf(fo,'\tdepth:units = "meters";\n');
fprintf(fo,'\tlat:units = "degrees_north";\n');
fprintf(fo,'\tlon:units = "degrees_east";\n');
fprintf(fo,'\tspeed:units = "meters/sec";\n');
fprintf(fo,'data:\n');
fprintf(fo,'\ttime = 0 ;\n');

fprintf(fo,'\tdepth = ');
for d=1:(D-1), fprintf(fo,'%.3f,',depth(d)); end
fprintf(fo,'%d;\n',depth(D));

fprintf(fo,'\tlat = ');
for n=1:(N-1), fprintf(fo,'%.3f,',latitude(n)); end
fprintf(fo,'%d;\n',latitude(N));

fprintf(fo,'\tlon = ');
for m=1:(M-1), fprintf(fo,'%.3f,',longitude(m)); end
fprintf(fo,'%d;\n',longitude(M));

fprintf(fo,'\tspeed = \n\t\t');
for d = 1:D
    for n = 1:N
        for m = 1:M
            fprintf(fo,'%.2f,',speed(d,n,m));
        end
        fprintf(fo,'\n\t\t');
    end
    fprintf(fo,'\n\t\t');
end
fprintf(fo,'\t;\n');

fprintf(fo,'}\n');
fclose(fo);
