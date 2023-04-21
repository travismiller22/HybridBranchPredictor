#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
//#include <bits/stdc++.h>
#include <algorithm>
#include <vector>
#include <bitset>
#include <map>

using namespace std;
string::size_type sz;
long lineNumber = 0;

// Standard variables
long m1;
long m2;
long k;
long n;
string traceFile;

// File Loading stuff

struct fileContentStruct {
    string hAdd;
    string OpCode;
};

fileContentStruct* fileData(string hAdd, string OpCode)
{
    fileContentStruct* t = new fileContentStruct();
    t->hAdd = hAdd;
    t->OpCode = OpCode;
    return t;
}

vector<fileContentStruct*> traceFileContent;

void readTraceFile(string traceFileName)
{
    ifstream infile(traceFileName);
    string hAdd;
    string OpCode;
    while (infile >> hAdd >> OpCode)
    {
        traceFileContent.push_back(fileData(hAdd, OpCode));
    }
}

// Bimodal

//Bimodal data structures
map <long, int> bTable;
long bmAcc = 0;
long bmR = 0;
long bmW = 0;

void initbTable() {
    for (long i = 0; i < pow(2, m2); i++) {
        bTable.insert({ i, 4 });
    }
}

//Bimodal index address generator
long B_Index_Gen(string hAdd) {
    long dAdd = stol(hAdd, &sz, 16);
    dAdd = dAdd / long(pow(2, 2));
    long index = dAdd % long(pow(2, m2));
    return(index);
}

auto BM_UpdatePredict = [](auto itr, string OpCode) {
    long previousValue = itr->second;
    if (OpCode == "t") {
        itr->second = min(itr->second + 1, 7);
        if (previousValue >= 4)
            bmR += 1;
        else
            bmW += 1;
    }
    else{
        itr->second = max(itr->second - 1, 0);
        if (previousValue < 4)
            bmR += 1;
        else
            bmW += 1;
    }
};

void bm_UpdateStat() {
    bmAcc += 1;
    lineNumber += 1;
}

void bimodal(string hAdd, string OpCode) {
   
    //Determine the index
    long index = B_Index_Gen(hAdd);
   
    //Check current prediction
    auto itr = bTable.find(index);
    int previousValue = itr->second;
    
    //if (previousValue >= 4) {
    //    //cout<<"\t"<<"Prediction:"<<"\t"<<"true"<<endl;
    //}
    //else {
    //    //cout<<"\t"<<"Prediction:"<<"\t"<<"false"<<endl;
    //}

    //Update the branch predictor
    BM_UpdatePredict(itr, OpCode);
    
    //Update bimod stat
    bm_UpdateStat();
}

// GShare

//GShare data structures
string g_pc;
map <long, int> gs_Table;
long gs_Acc = 0;
long gs_Right = 0;
long gs_Wrong = 0;

void initialize_GPC() {
    int z = 0;
    while (z < n) {
        g_pc.push_back('0');
        z++;
    }
}

void initialize_GSTable() {
    for (long i = 0; i < pow(2, m1); i++) {
        gs_Table.insert({ i, 4 });
    }
}

long GS_IndexGenerator(string hAdd) {
    //Convert the address into decimal
    long dAdd = stol(hAdd, &sz, 16);
    //Discard the last two bits
    dAdd = dAdd / long(pow(2, 2));
    //Separate the last n bits
    long lastNBits = dAdd % long(pow(2, n));
    //Separate the first m-n bits
    long firstBits = dAdd / long(pow(2, n));
    //Xor the last n bits with g_pc
    long nDec = stol(g_pc, nullptr, 2);
    long xoredBits = lastNBits ^ nDec;
    //Add the first (m-n) bits in decimal * 2**(m-n) + xored last n bits
    long newAdd = firstBits * long(pow(2, n)) + xoredBits;
    //Determine the index using the m bits
    long index = newAdd % long(pow(2, m1));
    return(index);
}

void gShareUpdInfo() { //Updates the statistcs from the gshare prediction
    gs_Acc += 1;
    lineNumber += 1;
}

auto gShareUpdateBP = [](auto itr, string OpCode) {
    //Update the branch predictor
    long previousValue = itr->second;
    if (OpCode == "t") {
        itr->second = min(itr->second + 1, 7);
        if (previousValue >= 4) {
            gs_Right += 1;
        }
        else {
            gs_Wrong += 1;
        }
    }
    else {
        itr->second = max(itr->second - 1, 0);
        if (previousValue < 4) {
            gs_Right += 1;
        }
        else {
            gs_Wrong += 1;
        }
    }
};

void g_PCUpdate(string OpCode) { //Global Update - updates the global history register
    string new_gPC;
    if (OpCode == "t") {
        new_gPC.push_back('1');
    }
    else {
        new_gPC.push_back('0');
    }
    for (auto itr = g_pc.begin(); itr != g_pc.end(); itr++) {
        new_gPC.push_back(*itr);
    }
    new_gPC.pop_back();
    g_pc = new_gPC;
    //cout<<"\t"<<"BHR now set to:"<<"\t";
    for (auto itr = g_pc.begin(); itr != g_pc.end(); itr++) {
        //cout<<"["<<*itr<<"]";
    }
    //cout<<endl;
}

void gShare(string hAdd, string OpCode) {

    //cout<<endl;
    //cout<<"<Line #"<<lineNum<<">"<<"\t"<<hAdd<<"\t"<<OpCode<<endl;

    long index = GS_IndexGenerator(hAdd);
    //cout<<"\t"<<"PT index:"<<"\t"<<index<<endl;

    //Check current prediction
    auto itr = gs_Table.find(index);
    int previousValue = itr->second;

    //cout<<"\t"<<"PT value:"<<"\t"<<previousValue<<endl;
    if (previousValue >= 4) {
        //cout<<"\t"<<"Prediction:"<<"\t"<<"true"<<endl;
    }
    else {
        //cout<<"\t"<<"Prediction:"<<"\t"<<"false"<<endl;
    }

    gShareUpdateBP(itr, OpCode);
    //cout<<"\t"<<"New PT value:"<<"\t"<<itr->second<<endl;

    g_PCUpdate(OpCode);

    gShareUpdInfo();
}

// Hybrid

//Hybrid data structures
map <long, int> h_Table;
long h_Acc = 0;
long h_Wrong = 0;
long h_Right = 0;

void intialize_HTable() {
    long t = 0;
    while (t < pow(2, k)) {
        h_Table.insert({ t, 1 });
        t++;
    }
}

void hybrid(string hAdd, string OpCode) {

    //Obtain index from bimod and gshare
    long bimodIndex = B_Index_Gen(hAdd);
    long gShareIndex = GS_IndexGenerator(hAdd);
    long dAdd = stoi(hAdd, &sz, 16);
    dAdd = dAdd / long(pow(2, 2));
    long hybridIndex = dAdd % long(pow(2, k));

    auto hybridItr = h_Table.find(hybridIndex);
    auto bimodItr = bTable.find(bimodIndex);
    auto gShareItr = gs_Table.find(gShareIndex);

    int bimodPrevVal = bimodItr->second;
    int gSharePrevVal = gShareItr->second;

    if (hybridItr->second >= 2) {
        //GShare
        //if (gSharePrevVal >= 4) {
        //    //cout<<"\t"<<"Prediction:"<<"\t"<<"true"<<endl;
        //}
        //else {
        //    //cout<<"\t"<<"Prediction:"<<"\t"<<"false"<<endl;
        //}
        gShareUpdateBP(gShareItr, OpCode);
        //cout<<"\t"<<"New gshare-PT value:"<<"\t"<<gShareItr->second<<endl;
        g_PCUpdate(OpCode);
        if ((OpCode == "t" && gSharePrevVal >= 4) || (OpCode == "n" && gSharePrevVal < 4))
            h_Right += 1;
        else 
            h_Wrong += 1;
    }
    else {
        //Bimod
        //if (bimodPrevVal >= 4) {
        //    //cout<<"\t"<<"Prediction:"<<"\t"<<"true"<<endl;
        //}
        //else {
        //    //cout<<"\t"<<"Prediction:"<<"\t"<<"false"<<endl;
        //}
        BM_UpdatePredict(bimodItr, OpCode);
        //cout<<"\t"<<"New bimodal-PT value:"<<"\t"<<bimodItr->second<<endl;
        g_PCUpdate(OpCode);
        if ((OpCode == "t" && bimodPrevVal >= 4) || (OpCode == "n" && bimodPrevVal < 4)) 
            h_Right += 1;
        else
            h_Wrong += 1;
        
    }

    if (OpCode == "t") {
        if (gSharePrevVal >= 4 && bimodPrevVal < 4)
            hybridItr->second = min(hybridItr->second + 1, 3);
        
        if (gSharePrevVal < 4 && bimodPrevVal >= 4)
            hybridItr->second = max(hybridItr->second - 1, 0);
    }
    else {
        if (gSharePrevVal < 4 && bimodPrevVal >= 4) {
            hybridItr->second = min(hybridItr->second + 1, 3);
        }
        if (gSharePrevVal >= 4 && bimodPrevVal < 4) {
            hybridItr->second = max(hybridItr->second - 1, 0);
        }
    }

    //cout<<"\t"<<"New CT value:"<<"\t"<<hybridItr->second<<endl;

    h_Acc += 1;
    lineNumber += 1;
}

int main(int argc, char* argv[]) {

    string opCode = argv[1];

    string opBimodal = "bimodal";
    string opGShare = "gshare";
    string opHybrid = "hybrid";

    if (opBimodal.compare(opCode) == 0) {
        string m2Str = argv[2];
        m2 = stol(m2Str, &sz);
        traceFile = argv[3];
        cout << "COMMAND" << endl;
        cout << "./sim bimodal " << m2 << " " << traceFile << endl; //m2=12
        initbTable();
    }

    if (opGShare.compare(opCode) == 0) {
        string m1Str = argv[2];
        string nStr = argv[3];
        m1 = stol(m1Str, &sz);
        n = stol(nStr, &sz);
        traceFile = argv[4];
        cout << "COMMAND" << endl;
        cout << "./sim gshare " << m1 << " " << n << " " << traceFile << endl; //m1=9, n=3
        initialize_GPC();
        initialize_GSTable();
    }

    if (opHybrid.compare(opCode) == 0) {
        string strK = argv[2];  //
        string strM1 = argv[3]; //m1 is bimodal = 12
        string strN = argv[4];  //m2 is gshare = 
        string strM2 = argv[5]; //

        k = stol(strK, &sz);
        m1 = stol(strM1, &sz);
        n = stol(strN, &sz);
        m2 = stol(strM2, &sz);
        traceFile = argv[6];
        initbTable();
        initialize_GPC();
        initialize_GSTable();
        intialize_HTable();
        cout << "COMMAND" << endl;
        cout << "./sim hybrid " << k << " " << m1 << " " << n << " " << m2 << " " << traceFile << endl;
    }

    readTraceFile(traceFile);

    for (unsigned long filePointer = 0; filePointer < traceFileContent.size(); filePointer++) {
        string hAdd = traceFileContent[filePointer]->hAdd;
        string OpCode = traceFileContent[filePointer]->OpCode;

        if (opBimodal.compare(opCode) == 0) {
            bimodal(hAdd, OpCode);
        }

        if (opGShare.compare(opCode) == 0) {
            gShare(hAdd, OpCode);
        }

        if (opHybrid.compare(opCode) == 0) {
            hybrid(hAdd, OpCode);
        }
    }

    if (opBimodal.compare(opCode) == 0) {
        cout << "OUTPUT" << endl;
        cout << "number of predictions:" << "\t" << "\t" << bmAcc << endl;
        cout << "number of mispredictions:" << "\t" << bmW << endl;
        cout << std::fixed;
        cout << "misprediction rate:" << "\t" << "\t" << std::setprecision(2) << float(bmW) / float(bmAcc) * 100 << "%" << endl;
        /*cout << "FINAL BIMODAL CONTENTS" << endl;
        for (auto itr = bTable.begin(); itr != bTable.end(); ++itr) {
            cout << itr->first << '\t' << itr->second << endl;
        }*/
    }

    if (opGShare.compare(opCode) == 0) {
        cout << "OUTPUT" << endl;
        cout << "number of predictions:" << "\t" << "\t" << gs_Acc << endl;
        cout << "number of mispredictions:" << "\t" << gs_Wrong << endl;
        cout << std::fixed;
        cout << "misprediction rate:" << "\t" << "\t" << std::setprecision(2) << float(gs_Wrong) / float(gs_Acc) * 100 << "%" << endl;
        cout << "FINAL GSHARE CONTENTS" << endl;
        /*for (auto itr = gs_Table.begin(); itr != gs_Table.end(); ++itr) {
            cout << itr->first << '\t' << itr->second << endl;
        }*/
    }

    if (opHybrid.compare(opCode) == 0) {
        cout << "OUTPUT" << endl;
        cout << "number of predictions:" << "\t" << "\t" << h_Acc << endl;
        cout << "number of mispredictions:" << "\t" << h_Wrong << endl;
        cout << std::fixed;
        cout << "misprediction rate:" << "\t" << "\t" << std::setprecision(2) << float(h_Wrong) / float(h_Acc) * 100 << "%" << endl;
        /*cout << "FINAL CHOOSER CONTENTS" << endl;
        for (auto itr = h_Table.begin(); itr != h_Table.end(); ++itr) {
            cout << itr->first << '\t' << itr->second << endl;
        }
        cout << "FINAL GSHARE CONTENTS" << endl;
        for (auto itr = gs_Table.begin(); itr != gs_Table.end(); ++itr) {
            cout << itr->first << '\t' << itr->second << endl;
        }
        cout << "FINAL BIMODAL CONTENTS" << endl;
        for (auto itr = bTable.begin(); itr != bTable.end(); ++itr) {
            cout << itr->first << '\t' << itr->second << endl;
        }*/
    }
}
