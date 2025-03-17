function [passBin, codesFinal, orientation, codes] = checkOrs9(imc)

check = [];
passBin = [];
codes = [];

for cc = 1:4
    imcr = rot90(imc,cc);
    check(cc) = checkCode9(imcr);
    codes(cc,:) = reshape(imcr', 1 ,9);
    disp(codes(cc,:));
end

if sum(check)~=1
    passBin = 0;
elseif sum(check) == 1
    passBin=1;
end

codesFinal = codes(check==1,:);
orientation = find(check ==1);

end
