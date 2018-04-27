#!bin/bash

make

echo TEST_RUN_1 - No flags
./sim_main face_tests/example1.img 43 > ./face_my_out1.txt
./sim_main face_tests/example2.img 43 > ./face_my_out2.txt
./sim_main face_tests/example3.img 43 > ./face_my_out3.txt
./sim_main face_tests/example4.img 43 > ./face_my_out4.txt
./sim_main face_tests/example5.img 80 > ./face_my_out5.txt
./sim_main face_tests/example6.img 80 > ./face_my_out6.txt
./sim_main face_tests/example7.img 80 > ./face_my_out7.txt
./sim_main face_tests/example8.img 80 > ./face_my_out8.txt
./sim_main face_tests/example9.img 80 > ./face_my_out9.txt
./sim_main face_tests/example10.img 80 > ./face_my_out10.txt
./sim_main face_tests/example11.img 80 > ./face_my_out11.txt
./sim_main face_tests/example12.img 20 > ./face_my_out12.txt
./sim_main face_tests/example13.img 20 > ./face_my_out13.txt
./sim_main face_tests/example14.img 20 > ./face_my_out14.txt
./sim_main face_tests/example15.img 20 > ./face_my_out15.txt

echo FACE_TEST_DIFF_1 - No flags
diff ./face_my_out1.txt ./face_results/example1.txt
diff ./face_my_out2.txt ./face_results/example2.txt
diff ./face_my_out3.txt ./face_results/example3.txt
diff ./face_my_out4.txt ./face_results/example4.txt
diff ./face_my_out5.txt ./face_results/example5.txt
diff ./face_my_out6.txt ./face_results/example6.txt
diff ./face_my_out7.txt ./face_results/example7.txt
diff ./face_my_out8.txt ./face_results/example8.txt
diff ./face_my_out9.txt ./face_results/example9.txt
diff ./face_my_out10.txt ./face_results/example10.txt
diff ./face_my_out11.txt ./face_results/example11.txt
diff ./face_my_out12.txt ./face_results/example12.txt
diff ./face_my_out13.txt ./face_results/example13.txt
diff ./face_my_out14.txt ./face_results/example14.txt
diff ./face_my_out15.txt ./face_results/example15.txt

echo TEST_RUN_2 - S flag
./sim_main face_tests/example1.img 43 -s > ./face_my_out1_s.txt
./sim_main face_tests/example2.img 43 -s > ./face_my_out2_s.txt
./sim_main face_tests/example3.img 43 -s > ./face_my_out3_s.txt
./sim_main face_tests/example4.img 43 -s > ./face_my_out4_s.txt
./sim_main face_tests/example5.img 80 -s > ./face_my_out5_s.txt
./sim_main face_tests/example6.img 80 -s > ./face_my_out6_s.txt
./sim_main face_tests/example7.img 80 -s > ./face_my_out7_s.txt
./sim_main face_tests/example8.img 80 -s > ./face_my_out8_s.txt
./sim_main face_tests/example9.img 80 -s > ./face_my_out9_s.txt
./sim_main face_tests/example10.img 80 -s > ./face_my_out10_s.txt
./sim_main face_tests/example11.img 80 -s > ./face_my_out11_s.txt
./sim_main face_tests/example12.img 20 -s > ./face_my_out12_s.txt
./sim_main face_tests/example13.img 20 -s > ./face_my_out13_s.txt
./sim_main face_tests/example14.img 20 -s > ./face_my_out14_s.txt
./sim_main face_tests/example15.img 20 -s > ./face_my_out15_s.txt


echo FACE_TEST_DIFF_2 - S flag
diff ./face_my_out1_s.txt ./face_results/example1-s.txt
diff ./face_my_out2_s.txt ./face_results/example2-s.txt
diff ./face_my_out3_s.txt ./face_results/example3-s.txt
diff ./face_my_out4_s.txt ./face_results/example4-s.txt
diff ./face_my_out5_s.txt ./face_results/example5-s.txt
diff ./face_my_out6_s.txt ./face_results/example6-s.txt
diff ./face_my_out7_s.txt ./face_results/example7-s.txt
diff ./face_my_out8_s.txt ./face_results/example8-s.txt
diff ./face_my_out9_s.txt ./face_results/example9-s.txt
diff ./face_my_out10_s.txt ./face_results/example10-s.txt
diff ./face_my_out11_s.txt ./face_results/example11-s.txt
diff ./face_my_out12_s.txt ./face_results/example12-s.txt
diff ./face_my_out13_s.txt ./face_results/example13-s.txt
diff ./face_my_out14_s.txt ./face_results/example14-s.txt
diff ./face_my_out15_s.txt ./face_results/example15-s.txt

echo TEST_RUN_3 - F flags
./sim_main face_tests/example1.img 43 -f > ./face_my_out1_f.txt
./sim_main face_tests/example2.img 43 -f > ./face_my_out2_f.txt
./sim_main face_tests/example3.img 43 -f > ./face_my_out3_f.txt
./sim_main face_tests/example4.img 43 -f > ./face_my_out4_f.txt
./sim_main face_tests/example5.img 80 -f > ./face_my_out5_f.txt
./sim_main face_tests/example6.img 80 -f > ./face_my_out6_f.txt
./sim_main face_tests/example7.img 80 -f > ./face_my_out7_f.txt
./sim_main face_tests/example8.img 80 -f > ./face_my_out8_f.txt
./sim_main face_tests/example9.img 80 -f > ./face_my_out9_f.txt
./sim_main face_tests/example10.img 80 -f > ./face_my_out10_f.txt
./sim_main face_tests/example11.img 80 -f > ./face_my_out11_f.txt
./sim_main face_tests/example12.img 20 -f > ./face_my_out12_f.txt
./sim_main face_tests/example13.img 20 -f > ./face_my_out13_f.txt
./sim_main face_tests/example14.img 20 -f > ./face_my_out14_f.txt
./sim_main face_tests/example15.img 20 -f > ./face_my_out15_f.txt


echo FACE_TEST_DIFF_3 - F flags
diff ./face_my_out1_f.txt ./face_results/example1-f.txt
diff ./face_my_out2_f.txt ./face_results/example2-f.txt
diff ./face_my_out3_f.txt ./face_results/example3-f.txt
diff ./face_my_out4_f.txt ./face_results/example4-f.txt
diff ./face_my_out5_f.txt ./face_results/example5-f.txt
diff ./face_my_out6_f.txt ./face_results/example6-f.txt
diff ./face_my_out7_f.txt ./face_results/example7-f.txt
diff ./face_my_out8_f.txt ./face_results/example8-f.txt
diff ./face_my_out9_f.txt ./face_results/example9-f.txt
diff ./face_my_out10_f.txt ./face_results/example10-f.txt
diff ./face_my_out11_f.txt ./face_results/example11-f.txt
diff ./face_my_out12_f.txt ./face_results/example12-f.txt
diff ./face_my_out13_f.txt ./face_results/example13-f.txt
diff ./face_my_out14_f.txt ./face_results/example14-f.txt
diff ./face_my_out15_f.txt ./face_results/example15-f.txt

rm face_my_out1.txt  
rm face_my_out2.txt  
rm face_my_out3.txt  
rm face_my_out4.txt  
#rm face_my_out5.txt  
rm face_my_out6.txt  
rm face_my_out7.txt  
rm face_my_out8.txt  
rm face_my_out9.txt  
rm face_my_out10.txt
rm face_my_out11.txt
rm face_my_out12.txt
rm face_my_out13.txt
rm face_my_out14.txt
rm face_my_out15.txt

rm face_my_out1_s.txt  
rm face_my_out2_s.txt  
rm face_my_out3_s.txt  
rm face_my_out4_s.txt  
#rm face_my_out5_s.txt  
rm face_my_out6_s.txt  
rm face_my_out7_s.txt  
rm face_my_out8_s.txt  
rm face_my_out9_s.txt  
rm face_my_out10_s.txt
rm face_my_out11_s.txt
rm face_my_out12_s.txt
rm face_my_out13_s.txt
rm face_my_out14_s.txt
rm face_my_out15_s.txt

rm face_my_out1_f.txt  
rm face_my_out2_f.txt  
rm face_my_out3_f.txt  
rm face_my_out4_f.txt  
#rm face_my_out5_f.txt  
rm face_my_out6_f.txt  
rm face_my_out7_f.txt  
rm face_my_out8_f.txt  
rm face_my_out9_f.txt  
rm face_my_out10_f.txt
rm face_my_out11_f.txt
rm face_my_out12_f.txt
rm face_my_out13_f.txt
rm face_my_out14_f.txt
rm face_my_out15_f.txt