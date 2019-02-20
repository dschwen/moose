
../tensor_mechanics-opt -i bar_ad.i --n-threads=8 Executioner/solve_type=PJFNK Outputs/file_base=bar_ad_out_pjfnk
../tensor_mechanics-opt -i bar_noad.i --n-threads=8 Executioner/solve_type=PJFNK Outputs/file_base=bar_noad_out_pjfnk
../tensor_mechanics-opt -i bar_ad.i --n-threads=8 Outputs/file_base=bar_ad_newton
../tensor_mechanics-opt -i bar_noad.i --n-threads=8 Outputs/file_base=bar_noad_newton
