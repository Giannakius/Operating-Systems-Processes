#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;
int main()
{


    int segmentation_degree = 10000; // Bathmos Katatmishs
    segmentation_degree = segmentation_degree - 1;

    ///////////////////////////
    ifstream file("BigTest.txt", ios::binary);
    string fileStr;

    istreambuf_iterator<char> inputIt(file), emptyInputIt;
    back_insert_iterator<string> stringInsert(fileStr);

    copy(inputIt, emptyInputIt, stringInsert);

    //cout << fileStr;
    //////////////////////////

    int string_size = fileStr.length();
    
    int lines_count = 0 ;

    // Count '\n' (change line) in BigText.txt
    int x;
    lines_count = 0 ;
    for (x=0;x<string_size;x++){
        if (fileStr[x]=='\n'){
            lines_count++;
        }
    }
    //cout << count;


    int chars_in_one_degree = lines_count / segmentation_degree ; 
    //cout << chars_in_one_degree;


    string array1[segmentation_degree];
    int temp = 0 , k = 0 , start = 0 , counter =0;

    while (counter<lines_count){
        while (k<=string_size and fileStr[k]!='\n' ){
            k++;
        }
        array1[temp] = fileStr.substr(start,k);
        temp = temp + 1;
        start = start + k +1;
    //cout << k;
    counter = counter + 1 ;
    }
    //cout << lines_count;
    for (x=0; x<= lines_count ; x++){
        cout << array1[x] << '\n';
    }


    return 0;
}