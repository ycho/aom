#!/usr/bin/env octave

% make sure tools are in the path
pin = program_invocation_name;
addpath(pin(1:(length(pin)-length(program_name))));

if (1)
args = argv();
else %for debugging or dev
a = ["4"; "1"; "zig4.h"; "~/workspace/aom/test.txt"];
args = cellstr(a);
end

bsize = str2num(args{1});
% 0 : DCT-DCT
% 1 : ADST-DCT
% 2 : DCT-ADST
% 3 : ADST-ADST

if (1)
output = args{2};
files = args(3:length(args));
else %for debugging or dev, provide tx_type as 2nd arg on command line
tx_type = str2num(args{2});
output = args{3};
files = args(4:length(args));
end

coeffs = zeros(1, bsize^2);
total = 0;

for tx_type=0:3
for i=1:length(files)
  t = importdata(files{i}, " ");
  % select relevant rows
  rows = find((t(:,1) == bsize) & (t(:,2) == tx_type));
  % pick out relevant coeffs
  t = t(rows,3:2+bsize^2);
  total = total + size(t, 1);
  %coeffs = coeffs .+ sum(t .^ 2);
  coeffs = coeffs .+ sum(abs(t));
end

% gen_zigzag expects a column vector, so this must be transposed
if (total ~= 0)
   v = coeffs' / total;
else
   display('# of samples is zero!.');
   return;
end
% generate zigzag
if bsize == 4
  gen_zigzag4(v, tx_type, output);
elseif bsize == 8
  gen_zigzag8(v, tx_type, output);
elseif bsize == 16
  gen_zigzag16(v, tx_type, output);
%elseif bsize == 32
%  gen_zigzag32(v, output);
else
  printf("error: invalid block size\n");
end
end %for tx_type=0:3
