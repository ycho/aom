#!/usr/bin/env octave

% make sure tools are in the path
pin = program_invocation_name;
addpath(pin(1:(length(pin)-length(program_name))));

%bsize = 4;
%output = "zigzag4.c"
%coeffs = [default_scan_4x4 row_scan_4x4 col_scan_4x4 default_scan_4x4];

%bsize = 8;
%output = "zigzag8.c"
%coeffs = [default_scan_8x8 row_scan_8x8 col_scan_8x8 default_scan_8x8];

bsize = 16;
output = "zigzag16.c"
coeffs = [default_scan_16x16 row_scan_16x16 col_scan_16x16 default_scan_16x16];

% tx_type
% 0 : DCT-DCT
% 1 : ADST-DCT
% 2 : DCT-ADST
% 3 : ADST-ADST

for tx_type=0:3

scan_order = coeffs(:,tx_type+1);

v = zeros(bsize*bsize, 1);  %will be filled with simulated coeff magnintude

for i = 1:bsize*bsize
  v(scan_order(i) + 1) = bsize*bsize - i + 1; % fill with decresing coeff value 
                                              % with av1's scan order
end

% generate zigzag
if bsize == 4
  gen_zigzag4_av1(v, tx_type, output);
elseif bsize == 8
  gen_zigzag8_av1(v, tx_type, output);
elseif bsize == 16
  gen_zigzag16_av1(v, tx_type, output);
%elseif bsize == 32
%  gen_zigzag32(v, output);
else
  printf("error: invalid block size\n");
end
end %for tx_type=0:3
