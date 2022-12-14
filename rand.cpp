#include <iostream>
#include <random>

using namespace std;

int Rand_Segment(int segment)
{
    int new_segment = segment;
    int PITHANOTHTA = 1 + (rand() % 10);    // pithanotita 1 - 10 
    
    // An tyxei 1-2-3-4-5-6-7 diladi 70% tote menoyme sto idio segment
    if (PITHANOTHTA<=7){
        //cout << "Idio segment " ; 
    }
    else { // An tyxei 8-9-10 , diladi 30% pithanotita tote pame se opoiodipote segment ektos apo ayto poy eimastan
        while (new_segment == segment){
            new_segment = 1 + (rand() % 100);
        }
        //cout << "allo segment " ; 
    }
    return new_segment ;
}

int main()
{
    srand((unsigned) time(NULL));
    int segment = 3 ;
    for(int i=0;i<60;i++)
    {
        segment = Rand_Segment(segment);
    }
    return 0;
}
