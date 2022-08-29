#include<iostream>
#include<fstream>
#include <cstdio>

#define OPENFILE    "in.txt"   //имя шифруемого файла
#define TMPFILE     "tmp.txt"  //временный файл с добавочными байтами
#define CRYPTEDFILE "out.txt"  //зашифрованный файл
#define KEYFILE     "key.txt"  //файл с ключом
#define INITFILE    "init.txt" //файл с синхропосылкой
#define SIZEFILE    "size.txt" //временный файл с размером

using namespace std;

//===============глобальные переменные====================

unsigned char LR[16];   //входной блок i-го раунда
unsigned char L[8];     //левая часть
unsigned char R[8];     //правая часть
unsigned int Rbuf[16];  //16 4-битовых значений
char KEY[4][8];         //ключ (4 подключа по 64 бита)
uint64_t LNUM1;         //левая часть i-го раунда
uint64_t RNUM1;         //правая часть i-го раунда
uint64_t LNUM2;         //левая часть i+1 -го раунда
uint64_t RNUM2;         //правая часть i+1 -го раунда

unsigned char OUT[16];         //массив для смены мест и вывода
unsigned char OPENBLOCK[16];   //массив с символами открытого текста
unsigned char OUTRES[16];      //массив, принимающий результат XORа
unsigned int SBOX[16][16] =    //таблица замен
    {
        {4,10,9,2,13,8,0,14,6,11,1,12,7,15,5,3},
        {14,11,4,12,6,13,15,10,2,3,8,1,0,7,5,9},
        {5,8,1,13,10,3,4,2,14,15,12,7,6,0,9,11},
        {7,13,10,1,0,8,9,15,14,4,6,12,11,2,5,3},
        {6,12,7,1,5,15,13,8,4,10,9,14,0,3,11,2},
        {4,11,10,0,7,2,1,13,3,6,8,5,9,12,15,14},
        {13,11,4,1,3,15,5,9,0,10,14,7,6,8,2,12},
        {1,15,13,0,5,7,10,4,9,2,3,14,6,11,8,12},
        {4,10,9,2,13,8,0,14,6,11,1,12,7,15,5,3},
        {14,11,4,12,6,13,15,10,2,3,8,1,0,7,5,9},
        {0,8,1,13,10,3,2,4,14,15,12,6,7,5,9,11},
        {7,13,10,1,0,8,9,15,14,4,6,12,11,2,5,3},
        {6,12,7,1,5,15,13,8,4,10,9,14,0,3,11,2},
        {4,11,10,0,7,2,1,13,3,6,8,5,9,12,15,14},
        {13,11,4,1,3,15,5,9,0,10,14,7,6,8,2,12},
        {15,1,13,0,5,7,10,4,9,2,3,14,6,11,8,12}
        
    };
//========================================================

//===============процедура шифрования=====================
void encrypt ()
{	
	/*for(int i = 0; i < 4; i++)    //считывание ключа
    {
    	for(int j = 0; j < 8; j++)
    	{
    		keyfile.read(&KEY[i][j], sizeof(char));
    	}
    }*/
    
    for(int i = 0; i < 8; i++)   //формирование части L
    {
    	L[i] = LR[i];
    }
    for(int i = 8; i < 16; i++)   //формирование части R
    {
    	R[i-8] = LR[i];
    }
    
    LNUM1 =              ((uint64_t)L[7] |       //формирование 64-битного числа из левой части
                          (uint64_t)L[6] << 8  |
                          (uint64_t)L[5] << 16 |
                          (uint64_t)L[4] << 24 |
                          (uint64_t)L[3] << 32 |
                          (uint64_t)L[2] << 40 |
                          (uint64_t)L[1] << 48 |
                          (uint64_t)L[0] << 56
    );
    RNUM1 =              ((uint64_t)R[7] |       //формирование 64-битного числа из правой части
                          (uint64_t)R[6] << 8  |
                          (uint64_t)R[5] << 16 |
                          (uint64_t)R[4] << 24 |
                          (uint64_t)R[3] << 32 |
                          (uint64_t)R[2] << 40 |
                          (uint64_t)R[1] << 48 |
                          (uint64_t)R[0] << 56 
    );
    
    for (int k = 0; k < 60; k++)    //60 раундов основного цикла, ключи K0 - K3
    {
    	
    	uint64_t KEYNUM =((uint64_t)KEY[k%4][7] |       //формирование 64-битного числа из подключа раунда
                          (uint64_t)KEY[k%4][6] << 8  |
                          (uint64_t)KEY[k%4][5] << 16 |
                          (uint64_t)KEY[k%4][4] << 24 |
                          (uint64_t)KEY[k%4][3] << 32 |
                          (uint64_t)KEY[k%4][2] << 40 |
                          (uint64_t)KEY[k%4][1] << 48 |
                          (uint64_t)KEY[k%4][0] << 56
        );
    	
    	LNUM2 = RNUM1;   //формирование части Li+1
    	RNUM1 = RNUM1 + KEYNUM;  //сложение по модулю 2^64
    	
    	Rbuf[15] = (RNUM1&0x000000000000000F);    //формирование 16 4-х битовых последовательностей
    	Rbuf[14] = (RNUM1&0x00000000000000F0)>>4;
    	Rbuf[13] = (RNUM1&0x0000000000000F00)>>8;
    	Rbuf[12] = (RNUM1&0x000000000000F000)>>12;
    	Rbuf[11] = (RNUM1&0x00000000000F0000)>>16;
    	Rbuf[10] = (RNUM1&0x0000000000F00000)>>20;
    	Rbuf[9] =  (RNUM1&0x000000000F000000)>>24;
    	Rbuf[8] =  (RNUM1&0x00000000F0000000)>>28;
    	Rbuf[7] =  (RNUM1&0x0000000F00000000)>>32;
    	Rbuf[6] =  (RNUM1&0x000000F000000000)>>36;
    	Rbuf[5] =  (RNUM1&0x00000F0000000000)>>40;
    	Rbuf[4] =  (RNUM1&0x0000F00000000000)>>44;
    	Rbuf[3] =  (RNUM1&0x000F000000000000)>>48;
    	Rbuf[2] =  (RNUM1&0x00F0000000000000)>>52;
    	Rbuf[1] =  (RNUM1&0x0F00000000000000)>>56;
    	Rbuf[0] =  (RNUM1&0xF000000000000000)>>60;
	
        for(int i = 0; i < 16; i++)    //замена по таблице замен
        {
        	Rbuf[i] = SBOX[i][Rbuf[i]];
        }
    
        RNUM1 =          ((uint64_t)Rbuf[15] |       //формирование 64-битного числа из правой части
                          (uint64_t)Rbuf[14] << 4  |
                          (uint64_t)Rbuf[13] << 8  |
                          (uint64_t)Rbuf[12] << 12 |
                          (uint64_t)Rbuf[11] << 16 |
                          (uint64_t)Rbuf[10] << 20 |
                          (uint64_t)Rbuf[9]  << 24 |
                          (uint64_t)Rbuf[8]  << 28 |
                          (uint64_t)Rbuf[7]  << 32 |
                          (uint64_t)Rbuf[6]  << 36 |
                          (uint64_t)Rbuf[5]  << 40 |
                          (uint64_t)Rbuf[4]  << 44 |
                          (uint64_t)Rbuf[3]  << 48 |
                          (uint64_t)Rbuf[2]  << 52 |
                          (uint64_t)Rbuf[1]  << 56 |
                          (uint64_t)Rbuf[0]  << 60 
                         );
    
        RNUM1 = ((RNUM1 << 11) | (RNUM1 >> 53));   //циклический сдвиг влево на 11 бит
        RNUM1 = RNUM1 ^ LNUM1;                     //XOR с левой частью, RNUM1 идет в следующую итерацию
        LNUM1 = LNUM2;                             //левая часть следующей итерации - бывшая правая часть
    }
    
    for (int k = 60; k < 64; k++)    //4 раундоа основного цикла, ключи K3 - K0
    {
    	
    	uint64_t KEYNUM =((uint64_t)KEY[3-(k%4)][7] |       //формирование 64-битного числа из подключа раунда
                          (uint64_t)KEY[3-(k%4)][6] << 8  |
                          (uint64_t)KEY[3-(k%4)][5] << 16 |
                          (uint64_t)KEY[3-(k%4)][4] << 24 |
                          (uint64_t)KEY[3-(k%4)][3] << 32 |
                          (uint64_t)KEY[3-(k%4)][2] << 40 |
                          (uint64_t)KEY[3-(k%4)][1] << 48 |
                          (uint64_t)KEY[3-(k%4)][0] << 56
        );
    	
    	LNUM2 = RNUM1;   //формирование части Li+1
    	RNUM1 = RNUM1 + KEYNUM;  //сложение по модулю 2^64
    	
    	Rbuf[15] = (RNUM1&0x000000000000000F);    //формирование 16 4-х битовых последовательностей
    	Rbuf[14] = (RNUM1&0x00000000000000F0)>>4;
    	Rbuf[13] = (RNUM1&0x0000000000000F00)>>8;
    	Rbuf[12] = (RNUM1&0x000000000000F000)>>12;
    	Rbuf[11] = (RNUM1&0x00000000000F0000)>>16;
    	Rbuf[10] = (RNUM1&0x0000000000F00000)>>20;
    	Rbuf[9] =  (RNUM1&0x000000000F000000)>>24;
    	Rbuf[8] =  (RNUM1&0x00000000F0000000)>>28;
    	Rbuf[7] =  (RNUM1&0x0000000F00000000)>>32;
    	Rbuf[6] =  (RNUM1&0x000000F000000000)>>36;
    	Rbuf[5] =  (RNUM1&0x00000F0000000000)>>40;
    	Rbuf[4] =  (RNUM1&0x0000F00000000000)>>44;
    	Rbuf[3] =  (RNUM1&0x000F000000000000)>>48;
    	Rbuf[2] =  (RNUM1&0x00F0000000000000)>>52;
    	Rbuf[1] =  (RNUM1&0x0F00000000000000)>>56;
    	Rbuf[0] =  (RNUM1&0xF000000000000000)>>60;
	
        for(int i = 0; i < 16; i++)    //замена по таблице замен
        {
        	Rbuf[i] = SBOX[i][Rbuf[i]];
        }
    
        RNUM1 =          ((uint64_t)Rbuf[15] |       //формирование 64-битного числа из правой части
                          (uint64_t)Rbuf[14] << 4  |
                          (uint64_t)Rbuf[13] << 8  |
                          (uint64_t)Rbuf[12] << 12 |
                          (uint64_t)Rbuf[11] << 16 |
                          (uint64_t)Rbuf[10] << 20 |
                          (uint64_t)Rbuf[9]  << 24 |
                          (uint64_t)Rbuf[8]  << 28 |
                          (uint64_t)Rbuf[7]  << 32 |
                          (uint64_t)Rbuf[6]  << 36 |
                          (uint64_t)Rbuf[5]  << 40 |
                          (uint64_t)Rbuf[4]  << 44 |
                          (uint64_t)Rbuf[3]  << 48 |
                          (uint64_t)Rbuf[2]  << 52 |
                          (uint64_t)Rbuf[1]  << 56 |
                          (uint64_t)Rbuf[0]  << 60 
                         );
    
        RNUM1 = ((RNUM1 << 11) | (RNUM1 >> 53));   //циклический сдвиг влево на 11 бит
        RNUM1 = RNUM1 ^ LNUM1;                      //XOR с левой частью, RNUM1 идет в следующую итерацию
        LNUM1 = LNUM2;                             //левая часть следующей итерации - бывшая правая часть
    }
    
    OUT[7] = (RNUM1&0x00000000000000FF);     //формирование бит на выход и обмен выходных частей местами
    OUT[6] = (RNUM1&0x000000000000FF00)>>8;
    OUT[5] = (RNUM1&0x0000000000FF0000)>>16;
    OUT[4] = (RNUM1&0x00000000FF000000)>>24;
    OUT[3] = (RNUM1&0x000000FF00000000)>>32;
    OUT[2] = (RNUM1&0x0000FF0000000000)>>40;
    OUT[1] = (RNUM1&0x00FF000000000000)>>48;
    OUT[0] = (RNUM1&0xFF00000000000000)>>56;
    
    OUT[15] = (LNUM1&0x00000000000000FF);
    OUT[14] = (LNUM1&0x000000000000FF00)>>8;
    OUT[13] = (LNUM1&0x0000000000FF0000)>>16;
    OUT[12] = (LNUM1&0x00000000FF000000)>>24;
    OUT[11] = (LNUM1&0x000000FF00000000)>>32;
    OUT[10] = (LNUM1&0x0000FF0000000000)>>40;
    OUT[9]  = (LNUM1&0x00FF000000000000)>>48;
    OUT[8]  = (LNUM1&0xFF00000000000000)>>56;
}

int main()
{
	ifstream in (OPENFILE, ios::in | ios::binary);
    in.seekg(0, ios::end);       
    long int sizef1 = in.tellg();    //размер исходного файла в байтах
    in.seekg(0, ios::beg);     
	
	ofstream size (SIZEFILE, ios::out);    //запись в файл исходного числа байтов
	size<<(sizef1);
	size.close();
	
	ofstream tmp (TMPFILE, ios_base::app | ios::binary);   //копирование файла для добавления байт
	for(int i = 0; i< sizef1; i++)
	{
		char buffer;
		in.read(&buffer, sizeof(char));
		tmp.put((char)buffer);
	}
	tmp.close();
	in.close();
	
	ofstream in2 (TMPFILE, ios_base::app | ios::binary);  //запись недостающего для блока числа байт
    if ((sizef1 % 16) != 0)
    {
    	for (int i = 0; i < (16 - (sizef1 % 16)); i++)
    	{
    		in2.put(0);
    	}
    }
    in2.close();
    
    ifstream initial (INITFILE, ios::in | ios::binary);   //заполнение массива LR синхропосылкой
    initial.read((char*)LR,sizeof(LR));
    initial.close();
    
    ifstream in3 (TMPFILE, ios::in | ios::binary);
    in3.seekg(0, ios::end);       
    long int size2 = in3.tellg();    //размер дополненного файла в байтах
    in3.seekg(0, ios::beg);
	in3.close();
	
	ifstream key (KEYFILE, ios::in | ios::binary);
	for(int i = 0; i < 4; i++)    //считывание ключа
    {
    	for(int j = 0; j < 8; j++)
    	{
    		key.read(&KEY[i][j], sizeof(char));
    	}
    }
    key.close();
    
    ifstream tmpfile (TMPFILE, ios::in | ios::binary);
    ofstream out (CRYPTEDFILE, ios_base::app | ios::binary);
    for(int k = 0; k < (size2/16); k++)     //основоной цикл штфрования
    {
    	encrypt();
    	tmpfile.read((char*)OPENBLOCK,sizeof(OPENBLOCK));
    	for(int i = 0; i < 16; i++)
    	{
    		OUTRES[i] = (OUT[i] ^ OPENBLOCK[i]);
    		out.put((char)OUTRES[i]);
    		LR[i] = OUTRES[i];
		}
	}
	
	tmpfile.close();
	out.close();
	remove(TMPFILE);
	return 0;
}
