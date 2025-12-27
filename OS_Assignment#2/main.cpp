//*****************With race condition version*****************//

//[####################]100% done
#include<iostream>
#include<pthread.h>
#include<vector>
#include <fstream>

using namespace std;
class threadData{
public:
    int threadId;
    int left;
    int right;
    int subArrSize;
    vector<int> sub_array;
};
int threadIdCounter=0;
//the input array
vector<int> gNumbers;
//define the three gloable counter
int gAboveTHCounter=0;
int gEqualTHCounter=0;
int gBelowTHCounter=0;
//threashold
int gTH=0;
void merge(vector<int> &arr, vector<int> &l_arr, int ls, vector<int> &r_arr, int rs) {

    int i=0,j=0,k=0;
    while(i < ls && j < rs) {
        if(l_arr[i] <= r_arr[j]) {
            arr[k] = l_arr[i];
            i++;
        } else {
            arr[k] = r_arr[j];
            j++;
        }
        k++;
    }
    while(i < ls) {
        arr[k] = l_arr[i];
        i++;
        k++;
    }
    while(j < rs) {
        arr[k] = r_arr[j];
        j++;
        k++;
    }
}

// arr: array to be divided into left and right | n: size of the array 
void mergeSort(vector<int> &arr, int n) {
    if(n < 2) return;

    int ls = n/2;
    int rs = n-ls;

    vector<int> l_arr(ls), r_arr(rs);
    int i;
    for(i=0; i<ls; i++) l_arr[i] = arr[i];
    for(i=ls; i<n; i++) r_arr[i-ls] = arr[i];

    mergeSort(l_arr, ls);
    mergeSort(r_arr, rs);
    merge(arr,l_arr,ls,r_arr,rs);
}

void *threadFun(void *arg){
    threadData* data=(threadData*)arg;
    cout<<"Starting thread "<<data->threadId<<": low = "<<data->left<<", high = "<<data->right<<"\n";
    vector<int> sub_array(data->subArrSize);
    for(int i=0;i<sub_array.size();i++){
        if(gTH<gNumbers[data->left+i])gAboveTHCounter++;
        if(gTH>gNumbers[data->left+i])gBelowTHCounter++;
        if(gTH==gNumbers[data->left+i])gEqualTHCounter++;
        sub_array[i]=gNumbers[data->left+i];
    }
    mergeSort(sub_array,sub_array.size());
    data->sub_array=sub_array;
    return nullptr;
}

bool loadInput(string filePath,vector<int> &inputArray)
{

    //input file format:
    //size threashold
    //[array_elements]
	ifstream inputFile(filePath);

	if (!inputFile.is_open())
	{
		cout << "Error opening the file\n";
		return false;
	}
    int arraySize=0;

    inputFile>>arraySize;
    inputFile>>gTH;
    for(int i=0;i<arraySize;i++){
        int element=0;
        inputFile>>element;
        inputArray.push_back(element);
    }

	inputFile.close();
    return true;

}
bool saveResult(string filePath){

    ofstream outputFile(filePath);
	if (!outputFile.is_open())
	{
		cout << "Could not open file " << filePath << "\n";
		return false;
	}
	// first we want to store the order of execution for the processes
    outputFile<<"Sorted array:\n";
	for(int i=0;i<gNumbers.size();i++){
        outputFile<<gNumbers[i]<<' ';
    }
    outputFile.close();
    return true;
}
void *notWorkingThread(void *arg){
    threadData * data=(threadData*)arg;
    cout<<"Starting thread "<<data->threadId<<" : ";
    cout<<"There is no work to do...\n";
    return nullptr;
}
//function that assigns the work for each thread (give part of gNumbers for each worker thread)
void assignWorkToThreads(pthread_t threads[],threadData data[],int size){

    int sizeOfSubArr=gNumbers.size()/size;
    int j=0;
    if(size>gNumbers.size()){
        sizeOfSubArr=1;
    }

    for(int i=0;i<size;i++){
        if(i>=gNumbers.size()){
            data[i].left=-1;
            data[i].right=-1;
            data[i].subArrSize=0;
            data[i].threadId=threadIdCounter;
        }
        else {
            data[i].threadId=threadIdCounter;
            data[i].left=j;
            data[i].right=j+sizeOfSubArr-1;
            data[i].subArrSize=sizeOfSubArr;
        }
        j+=sizeOfSubArr;
        threadIdCounter++;
    }
    for(int i=0;i<size;i++){
        if(i>=gNumbers.size())
        {
            pthread_create(&threads[i],NULL,notWorkingThread,(void*)&data[i]);
        }
        else pthread_create(&threads[i],NULL,threadFun,(void*)&data[i]);
    }

}

void mainThreadWork(threadData *data,int size){
    //let main thread do the rest
    
    int tempSize=size;
    while(tempSize!=1){
        int j=0;
        for(int i=0;i<tempSize;i+=2){
            int ls=data[i].sub_array.size();
            int rs=data[i+1].sub_array.size();
            vector<int> temp(ls+rs);
            merge(temp,data[i].sub_array,ls,data[i+1].sub_array,rs);
            data[j].sub_array=temp;
            j++;
        }
        gNumbers=data[0].sub_array;
        tempSize/=2;
    }
}
int main(int argc,char*argv[]){
    int numberOfThreads=stoi(argv[1]);//T

    if(!loadInput("in.txt",gNumbers)){
        cout<<"Loading files went wrong!!\n";
        return 0;
    }

    cout<<"Main: Starting sorting with N="<<gNumbers.size()<<", TH="<<numberOfThreads<<"\n";

    pthread_t *threads=new pthread_t[numberOfThreads];
    threadData *data=new threadData[numberOfThreads];
    
    assignWorkToThreads(threads,data,numberOfThreads);
    for(int i=0;i<numberOfThreads;i++){
        pthread_join(threads[i],nullptr);
    }

    if(numberOfThreads>gNumbers.size())numberOfThreads=gNumbers.size();

    cout<<"Main: Above Threshold = "<<gAboveTHCounter<<"\n";
    cout<<"Main: Equals Threshold = "<<gEqualTHCounter<<"\n";
    cout<<"Main: Below Threshold = "<<gBelowTHCounter<<"\n";

    mainThreadWork(data,numberOfThreads);

    saveResult("out.txt");

    return 0;
}