function imc = createCode(num)

%%
%Create plot of matrix from binary data

%length of tag code
nBit = 3;

%ID number

%Convert ID number to binary
bin = dec2bin(num);

%Take length of binary code
L = numel(bin);
im = zeros(1, nBit);

for i = 1:L

im(nBit-L+i) = str2num(bin(i));

end
disp(im);
im = reshape(im,3,1)';
disp(im)
%checksum
check = [];




%for aa = 1:3   
    %check(aa)  = mod(sum(im(aa,:)), 2);
       
%end
check(1) = mod(sum(im(1,:)),2);
check(2) = mod(sum(sum(im(:,1:2))), 2);
check(3) = mod(sum(sum(im(:,3))), 2);
       
check2 = fliplr(check);

imc = [im; check;check2];
end