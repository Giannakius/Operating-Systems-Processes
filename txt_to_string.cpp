#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;
int main()
{
    int segmentation_degree = 1000; // Bathmos Katatmishs   // Posa kommatia tha xwristei to text

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
            lines_count++;  // athroisma grammwn sto text
        }
    }

    int lines_per_section = (lines_count+1) / segmentation_degree ;   // grammes ana tmima 
    
    string array1[segmentation_degree];         // Array with the segmended string in its rows
    int temp = 0 , k = 0 , start = 0 , counter =0 , X = 0 ;

    while (X<segmentation_degree){
        while (k<=string_size and counter<lines_per_section){
            k++;    // find all the chars until the new line
            if (fileStr[k]=='\n'){
                counter++;
            }
        }
        array1[temp] = fileStr.substr(start,k);     // copy them to the array [temp]
        temp = temp + 1;
        start = start + k +1;   // fix the start for the new start
        //cout << k;
    X = X + 1 ;     // lines counter to stop the while
    }

    //Print the Array
    
    for (x=0; x< segmentation_degree ; x++){
        cout << array1[x] << '\n' << '\n' << '\n';
    }

    return 0;
}