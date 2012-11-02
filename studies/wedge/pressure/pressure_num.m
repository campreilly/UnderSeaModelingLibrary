%%%%%%
% numerical evaluation of eqn (24) and (25) from Deane/Tindle

function N = pressure_num( source_rho, source_theta, source_phi, ...
			target_rho, target_theta, target_phi, ...
			wave_number, wave_theta, wedge_theta, ...
			s, b, density, speed, atten)
	
	% pre-compute some frequently used numbers
    
	sin_target_theta = sin( target_theta ) ;
	cos_target_theta = cos( target_theta ) ;

	sin_theta = sin( wave_theta ) ;
	cos_theta = cos( wave_theta ) ;
    
	omega = wave_number .* target_rho .* sin_theta .* sin_target_theta ;
	omegaZ = wave_number .* target_rho .* cos_theta .* cos_target_theta ;

	% compute contribution from first term in the Bessel series

	loss = 1.0 ;
	for num_bottom = 1:b
		theta = 2 .* wedge_theta .* (num_bottom-1) ;
		angle = acos( cos_theta .* cos(theta) - sin_theta .* sin(theta) ) ;
		loss = loss .* reflection( angle, ...
			density, speed, atten ) ;
	end

	N = (1i * wave_number) .* exp( 1i*omegaZ ) .* (-1)^s ...
		.* loss .* besselj(0,omega) .* sin_theta ;
	
end
