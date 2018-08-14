#include <iostream>
#include <fstream>
#include <string.h>
#include <chrono>
#include "perm.h"

using namespace std;
using namespace chrono;

//variables
int buffer[64];//holds original data/key
int keySub[16][48];//16 subkeys
//////int dataSub[16][32];//I could of only used a [2][2][32] array, however it would lead to less legible code, and the additional bits used are not an issue with today's hard drive space
bool fail=false;
//end variables

//prototypes
void keyIn(int n);//saves key.txt to int key
void keyGen();//generates 16 subkeys
void keyFlip();//reverses order of keys for decryption
void dataIn();//opens data.txt 64-bit
void dataEnc();//encodes data
void dataOut();//outputs 64-bit data
//end prototypes

int main()
{

    typedef high_resolution_clock clo;
    clo::time_point t = clo::now();//timer
    int choice=0,choice2=0;
    do{
        cout<<"1)  DES Encryption\n2)  DES Decryption\n3)  3x DES Encryption\n4)  3x DES Decryption"<<endl;
        cin>>choice;
    }while(choice<=0||choice>=5);
    if(choice==3||choice==4){
        do{
            cout<<endl<<endl<<"(1) Key?\n(2) Keys?\n(3) Keys?"<<endl;//value is set to n+10
            cin>>choice2;
        }while(choice2>3||choice2<1);
    }
    switch(choice){
        case 1:
        case 2:
            keyIn(0);
            keyGen();
            if(choice==2)
                keyFlip();
            dataIn();
            dataEnc();
            dataOut();
            break;
        case 3:
        case 4:
            for(int i=0;i<3;i++){
                if(choice2==1){
                   keyIn(0);}
                if(choice2==2){
                    (i%2==1) ? keyIn(1):keyIn(0);}
                if(choice2==3){
                    keyIn(i);}
                keyGen();
                if((i==1&&choice!=4)||(choice==4&&i!=1)){//if decrypt flip key order
                    keyFlip();
                }
                dataIn();
                dataEnc();
                dataOut();
            }
            break;
        default:
            cout<<"error 404, option not found"<<endl;
            fail=true;
    }
    clo::time_point t1= clo::now();
    duration<double> time = duration_cast<duration<double> >(t1-t);

    if(!fail){
        cout<<"File saved to \"data.txt\""<<endl<<"runtime: "<<time.count()<<" miliseconds"<<endl;
    }else
        cout<<"Operation failed";
    return 0;
}

void keyIn(int n){
    char keyC;
    string key[3]={"key0.txt","key1.txt","key2.txt"};
    ifstream keyF(key[n], ios::in|ios::binary);//loads key file
    if (!keyF.is_open()){
        cout<<"Could not open "<<key[n]<<endl;
        fail=true;
    }
    else {
        int i=0;
        while(!keyF.eof()||i==64){
                keyF.get(keyC);
                if(keyC==48||keyC==49){//only accepts a 1 or 0 for input, binary
                    buffer[i++]=(int)keyC-48;//converts char(ASCII) into int
                }
        }
    }
    keyF.close();
}

void keyGen(){
    int i,j,k,temp1,temp2;
    bool run=true;
    int key[2][17][28];//2x17x28 set of C and D keys

    for(k=0;k<28;k++){//64-bit key ->28-bit keys C,D
        key[0][0][k]=buffer[PC1[k]-1];//C0
        key[1][0][k]=buffer[PC1[k+28]-1];//D0
    }//generates 16 C and D keys
    for (i=0; i<2; i++){
        for(j=1; j<17; j++){
            run = true;
            for(k=0; k<28; k++){
                if(run){
                    temp1 = key[i][j-1][0];//saves first 2 bits for left shift
                    temp2 = key[i][j-1][1];
                    run = false;
                }
                key[i][j][k]=key[i][j-1][k+shift[j-1]];//bitshift from previous key
                if(k==27&&shift[j-1]==1){
                    key[i][j][27]=temp1;
                }if(k==27&&shift[j-1]==2){
                    key[i][j][26]=temp1;
                    key[i][j][27]=temp2;
                }
            }
        }
    }

    //generate 16 subkeys
    for(j=0; j<16; j++){
        for(k=0; k<48; k++){
            if(PC2[k]<=28)
                keySub[j][k]=key[0][j+1][PC2[k]-1];
            else
                keySub[j][k]=key[1][j+1][PC2[k]-29];
        }
    }
/*
     //key print
    cout<<"Key = ";
    for(int i=0; i<64; i++){
        cout <<buffer[i];
    }
    cout<<endl;

/*
    for(int j=0; j<16; j++){
        for (int i=0; i<2; i++){
            if(i==0) cout<<"C";
            else cout<<"D";
            cout<<j<<" = ";
            for(int k=0; k<28; k++){
                cout<<key[i][j][k];
            }
            cout<<endl;
        }
    }
    //end key print
    //*/
    /*
    //Print 16 round keys
    for (i=0;i<16;i++){
        cout<<"K"<<i<<" = ";
        for(k=0;k<48;k++){
            cout<<keySub[i][k];
        }cout<<endl;
    }//*/

}

void keyFlip(){
    int keySub2[48];
    for (int i=0;i<8;i++){//copy keys
        for(int j=0;j<48;j++){
            keySub2[j]=keySub[i][j];
            keySub[i][j]=keySub[15-i][j];
            keySub[15-i][j]=keySub2[j];
        }
    }

}

void dataIn(){
    char dataC;
    ifstream dataF("data.txt", ios::in|ios::binary);//loads key file
    if (!dataF.is_open())
        cout<<"Could not import data\n";
    else {
        int i=0;
        while(!dataF.eof()&&i<64){
                dataF.get(dataC);
                if(dataC==48||dataC==49){//only accepts a 1 or 0 for input, binary
                    buffer[i++]=(int)dataC-48;//converts char(ASCII) into int
                }
        }
    }
    dataF.close();
}

void dataEnc(){
    int i,j,k,buf;
    int buffer2[32];
    int data[2][17][32];//L-R,Round,bit

    for (k=0;k<32;k++){//initial permutation
        data[0][0][k]=buffer[permInit[k]-1];
        data[1][0][k]=buffer[permInit[k+32]-1];
    }

    for (j=1;j<17;j++){//16 rounds
        for (k=0;k<32;k++){//Ln+1<Rn
            data[0][j][k] = data[1][j-1][k];
        }
        //setting Rn+1
        for(k=0;k<48;k++){//ebit expansion
            buffer[k] = data[1][j-1][EBit[k]-1]^keySub[j-1][k];
        }
        memset(buffer2, 0, sizeof(buffer2));//reset buffer2 array
        for(k=0;k<48;k+=6){//SBox
            buf=s[k/6][buffer[k]*2+buffer[k+5]][buffer[k+1]*8+buffer[k+2]*4+buffer[k+3]*2+buffer[k+4]];
            for(i=3;i>=0;i--){
                buffer2[k/6*4+i]=buf%2;
                buf/=2;
            }
        }
        for(k=0;k<32;k++){//final perm and Ln+f(Rn-1,kn)
            data[1][j][k]=data[0][j-1][k]^buffer2[p[k]-1];
        }
    }

    //reverse order of final blocks and apply final perm
    for(k=0;k<64;k++){
        if(permFin[k]<=32)
            buffer[k]=data[1][16][permFin[k]-1];
        else
            buffer[k]=data[0][16][permFin[k]-33];
    }

}

void dataOut(){
    ofstream out;
    int i;
    out.open("data.txt");
    for(i=0;i<64;i++){#endif // PERM_H_INCLUDED
        out<<buffer[i];
        if((i+1)%4==0&&i!=0)
            out<<" ";
    }
    out.close();
}
