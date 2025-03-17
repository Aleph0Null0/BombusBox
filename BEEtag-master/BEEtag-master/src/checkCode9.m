function [pass] = checkCode9(imc)

    im = imc(1,:);
    check = imc(2,:);
    check2 = imc(3,:);
    pass = 1;

    if check(1) ~= mod(sum(im(1,:)),2)
        pass = 0;

    elseif check(2) ~= mod(sum(im(:,1:2)), 2)
        pass = 0;

    elseif check(3) ~= mod(sum(im(:,3)), 2)
        pass = 0;

    elseif check2 ~= fliplr(check)
        pass = 0;
    end
    %disp(pass);
end