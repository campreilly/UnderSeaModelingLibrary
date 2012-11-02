%%%%%%%%%
% function to calculate the pressure at a given location with specific details

function L = flat_image( target_r, source_theta, wave_number, wave_theta, ...
			s, b, density, speed, atten)
	
	% constants used for all loss calculations
	sin_theta = sin( wave_theta ) ;
	cos_theta = cos( wave_theta ) ;

	loss = 1.0 ;
	for num_bottom = 1:b
		theta = 2 .* source_theta .* (num_bottom-1) ;
		angle = acos( cos_theta .* cos(theta) - sin_theta .* sin(theta) ) ;
		loss = loss .* reflection( angle, density, speed, atten ) ;
	end
	
	L = ( (-1)^s .* loss .* exp(1i .* wave_number .* target_r) ) / target_r;
end
