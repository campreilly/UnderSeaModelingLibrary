% a roll routine based on the rotation matrix in platform/orientation and
% the rotate vector routine found there.
%
% yaw, pitch roll in degrees
% v is a 3xN of north/east/up vectors in world coordaites
% v_r are those vectors rotated into the body coordaintes
function [v_r] = rotate_vector_l2g(yaw, pitch, roll, v)

    cosp = cosd(pitch);
    cosr = cosd(roll);
    cosy = cosd(yaw);
    sinr = sind(roll);
    sinp = sind(pitch);
    siny = sind(yaw);

	Rroll =[...
		[1.0, 0.0, 0.0,];...
		[0.0, cosr, sinr,];...
		[0.0, -sinr, cosr]];

	Rpitch =[...
		[cosp, 0.0, -sinp,];...
		[0.0, 1.0, 0.0,];...
		[sinp, 0.0, cosp]];

	Ryaw =[...
		[cosy, -siny, 0.0,];...
		[siny, cosy, 0.0,];...
		[0.0, 0.0, 1.0]];
    
    temp = Rpitch * Rroll;
    R = Ryaw * temp;
    
    for n=1:size(v,2)       
        v_r(:,n) = R * v(:,n);        
    end

end