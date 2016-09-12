All *.m files are written for Octave, so may not be compatible with Matlab.

To generate scan orders of AV1 hybrid transforms for PVQ:
1) Copy AV1 scan orders (from scan.c) to av_scan_order.m
2) Run av_scan_order.m
3) With variables defined in av_scan_order.m live in memory,
   Edit (Enable/Disable) setting for designated block size in 
    compute_zigzag_av1.m as shown below.

  %bsize = 4;
  %output = "zigzag4.c"
  %coeffs = [default_scan_4x4 row_scan_4x4 col_scan_4x4 default_scan_4x4];

   Run compute_zigzag_av1.m to generate scan file (*.c) for each block size.
   Repeat above for different block size
   Ex: compute_zigzag_av1.m
