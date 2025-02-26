/*
To run
cd scripts
./mat_mul.sh
*/
#include <iostream>
#include <c7x.h>
#include <c7x_scalable.h>
using namespace std;
using namespace c7x;
int main(){
    int row1,col1,row2,col2,vec_len = element_count_of<int_vec>::value,iteration1, iteration2;
    cout<<"Enter the Row Size for Matrix 1 : ";
    cin>>row1;
    cout<<"Enter the Column Size for Matrix 1 : ";
    cin>>col1;
    cout<<"Enter the Row Size for Matrix 2 : ";
    cin>>row2;
    cout<<"Enter the Column Size for Matrix 2 : ";
    cin>>col2;
    if(row2 != col1){
        cout<<"This is not a valid matrix to do multiplication";
        return 0;
    }
    int32_t mat1[row1][col1],mat2[row2][col2],res[row1][col2],res2[row1][col2];
    for(int r = 0;r < row1;r++){
        for(int c =0;c < col1;c++){
            mat1[r][c] =  r+c;
        }
    }
    for(int r = 0;r < row2;r++){
        for(int c =0;c < col2;c++){
            mat2[r][c] =  r+c;
        }
    }
    for(int r = 0;r < row1;r++){
        for(int c = 0;c < col2;c++){
            res[r][c] = 0;
            res2[r][c] = 0;
        }
    }
    for(int r = 0;r < row1;r++){
        for(int c =0;c < col2;c++){
            for(int k = 0;k < col1;k++){
                res[r][c] += mat1[r][k] * mat2[k][c];
                iteration1++;   
            }
        }
    }
    int rem = col2 % vec_len;
    int start = col2 - rem;
    cout<<"Start : "<<start<<endl;
    // Add Streaming Engine
    __SE_TEMPLATE_v1 seTemplate = __gen_SE_TEMPLATE_v1();
    seTemplate.ELETYPE   = se_eletype<int_vec>::value;
    seTemplate.VECLEN    = se_veclen<int_vec>::value;
    seTemplate.DIMFMT = __SE_DIMFMT_1D;
    seTemplate.ICNT0 = col2;
    __SE_TEMPLATE_v1 seTemplate2 = __gen_SE_TEMPLATE_v1();
    seTemplate2.ELETYPE   = se_eletype<int_vec>::value;
    seTemplate2.VECLEN    = se_veclen<int_vec>::value;
    seTemplate2.DIMFMT = __SE_DIMFMT_2D;
    seTemplate2.ICNT0 = col1;
    seTemplate2.ICNT1 = row1;                 
    seTemplate2.DIM1 = col1;
    seTemplate2.ELEDUP    = __SE_ELEDUP_16X;
    // For Address Generator
    __SA_TEMPLATE_v1 saTemplate = __gen_SA_TEMPLATE_v1();
    saTemplate.VECLEN    = sa_veclen<int_vec>::value;
    saTemplate.DIMFMT = __SA_DIMFMT_2D;
    saTemplate.ICNT0 = col2 - (col2 % 16);
    saTemplate.ICNT1 = row1; 
    saTemplate.DIM1 = col2;   
    __SA0_OPEN(saTemplate);
    for(int r = 0;r < row1;r++){
        for(int c = 0;c+vec_len <= col2;c+=vec_len){
            int_vec vOutC = (int_vec)(0);
            __SE0_OPEN((void *)&mat2[0][c], seTemplate);
            __SE1_OPEN((void *)&mat1[r][0], seTemplate2);
            int times = 0;
            for(int cc = 0;cc < col1;cc+=1){
                int_vec resw = __vmpyww_vvv(strm_eng<0, int_vec>::get_adv(),strm_eng<1, int_vec>::get_adv());
                vOutC = __vaddw_vvv(vOutC,resw);
                iteration2++;
                __SE0_OPEN((void *)&mat2[cc+1][c], seTemplate);
            }
            __vpred pred = strm_agen<0, int_vec>::get_vpred();
            int_vec * addr = strm_agen<0, int_vec>::get_adv(&res2[0][0]);
            __vstore_pred(pred, addr, vOutC);
        }
        for(int x = start;x < col2;x++){
            for(int k = 0;k < col1;k++){
                res2[r][x] += mat1[r][k] * mat2[k][x];  
                iteration2++;
            }
        }
    }
    for(int r = 0;r < row1;r++){
        for(int c = 0;c < col2;c++){
            if(res[r][c] != res2[r][c]){
                cout<<"They are not Equal :("<<endl;
            }
        }
    }
    __SA0_CLOSE();
    __SE0_CLOSE();
    __SE1_CLOSE();
    cout<<"Iteration 1 : "<<iteration1<<endl;
    cout<<"Iteration 2 : "<<iteration2<<endl;
    cout<<"Intrinsics Code is "<<(iteration1/iteration2)<<" times better than Scalar Code"<<endl;
}