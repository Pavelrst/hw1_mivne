#!bin/bash

make

./sim_main tests/example1.img 20 > ./my_out1.txt
./sim_main tests/example2.img 20 > ./my_out2.txt
./sim_main tests/example3.img 20 > ./my_out3.txt
./sim_main tests/example4.img 20 > ./my_out4.txt

echo TEST1
diff ./my_out1.txt ../CompArch-hw1-test_results/res_example1.img
echo TEST2
diff ./my_out2.txt ../CompArch-hw1-test_results/res_example2.img
echo TEST3
diff ./my_out3.txt ../CompArch-hw1-test_results/res_example3.img
echo TEST4
diff ./my_out4.txt ../CompArch-hw1-test_results/res_example4.img