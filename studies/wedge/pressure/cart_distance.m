%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%5
%% Cartesian distance calculation

function r = cart_distance(source_x, source_z,...
			target_x, target_z)

	r = sqrt( (source_x .- target_x).^2 + (source_z .- target_z).^2);

end
