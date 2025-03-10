
g++ -I /home/arshathahamed10/ti/ti-cgt-c7000_4.1.0.LTS/host_emulation/include/C7100/ -o output ../im2col_cnn.cpp `pkg-config --cflags --libs opencv4` -L /home/arshathahamed10/ti/ti-cgt-c7000_4.1.0.LTS/host_emulation/*.a 
./output