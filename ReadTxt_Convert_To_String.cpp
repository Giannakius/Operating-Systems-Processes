#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;
int main()
{
    int segmentation_degree = 100; // Bathmos Katatmishs
    segmentation_degree = segmentation_degree ;

    ////// Make The txt to string ////////////
    ifstream file("BigTest.txt", ios::binary);
    string fileStr;

    istreambuf_iterator<char> inputIt(file), emptyInputIt;
    back_insert_iterator<string> stringInsert(fileStr);

    copy(inputIt, emptyInputIt, stringInsert);

    //cout << fileStr;

    //////////////////////////////////////////

    int string_size = fileStr.length();     // Length of .txt file
    
    int lines_count = 0 ;                   // Count how many rows in .txt
    int x;
    lines_count = 0 ;
    for (x=0;x<string_size;x++){
        if (fileStr[x]=='\n'){
            lines_count++;
        }
    }


    string array1[segmentation_degree];         // Array with the segmended string in its rows
    int temp = 0 , k = 0 , start = 0 , counter =0;

    while (counter<=lines_count){
        while (k<=string_size and fileStr[k]!='\n' ){
            k++;    // find all the chars until the new line
        }
        array1[temp] = fileStr.substr(start,k);     // copy them to the array [temp]
        temp = temp + 1;
        start = start + k +1;   // fix the start for the new start
    //cout << k;
    counter = counter + 1 ;     // lines counter to stop the while
    }

    // Print the Array
    for (x=0; x<= lines_count ; x++){
        cout << array1[x] << '\n';
    }

    return 0;
}