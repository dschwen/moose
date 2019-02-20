
mpirun -n 4 ../tensor_mechanics-opt -i bar_ad.i  Outputs/file_base=bar_ad_newton
mpirun -n 4 ../tensor_mechanics-opt -i bar_noad.i  Outputs/file_base=bar_noad_newton
mpirun -n 4 ../tensor_mechanics-opt -i bar_ad.i  Executioner/solve_type=PJFNK Outputs/file_base=bar_ad_out_pjfnk
mpirun -n 4 ../tensor_mechanics-opt -i bar_noad.i  Executioner/solve_type=PJFNK Outputs/file_base=bar_noad_out_pjfnk
