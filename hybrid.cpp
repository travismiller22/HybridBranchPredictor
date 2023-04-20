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
long lineNum = 0;

// Standard variables
long m1;
long m2;
long k;
long n;
string traceFile;

// File Loading stuff

struct fileContentStruct {
    string hexAdd;
    string op;
};

fileContentStruct* fileData(string hexAdd, string op)
{
    fileContentStruct* temp = new fileContentStruct();
    temp->hexAdd = hexAdd;
    temp->op = op;
    return temp;
}

vector<fileContentStruct*> traceFileContent;

void readTraceFile(string traceFileName)
{
    ifstream infile(traceFileName);
    string hexAdd;
    string op;
    while (infile >> hexAdd >> op)
    {
        traceFileContent.push_back(fileData(hexAdd, op));
    }
}

// Bimodal

//Bimodal data structures
map <long, int> bimodTable;
long bimodAccess = 0;
long bimodRight = 0;
long bimodWrong = 0;

void initBimodTable() {
    for (long i = 0; i < pow(2, m2); i++) {
        bimodTable.insert({ i, 4 });
    }
}

//Bimodal index address generator
long bimodIndexMaker(string hexAdd) {
    long decAdd = stol(hexAdd, &sz, 16);
    decAdd = decAdd / long(pow(2, 2));
    long index = decAdd % long(pow(2, m2));
    return(index);
}

auto bimodUpdateBranchPredictor = [](auto itr, string op) {
    long prevVal = itr->second;
    if (op == "t") {
        itr->second = min(itr->second + 1, 7);
        if (prevVal >= 4) {
            bimodRight += 1;
        }
        else {
            bimodWrong += 1;
        }
    }
    else {
        itr->second = max(itr->second - 1, 0);
        if (prevVal < 4) {
            bimodRight += 1;
        }
        else {
            bimodWrong += 1;
        }
    }
};

void bimodUpdateStat() {
    bimodAccess += 1;
    lineNum += 1;
}

void bimodal(string hexAdd, string op) {
    //cout<<endl;
    //cout<<"<Line #"<<lineNum<<">"<<"\t"<<hexAdd<<"\t"<<op<<endl;

    //Determine the index
    long index = bimodIndexMaker(hexAdd);
    //cout<<"\t"<<"PT index:"<<"\t"<<index<<endl;

    //Check current prediction
    auto itr = bimodTable.find(index);
    int prevVal = itr->second;
    //cout<<"\t"<<"PT value:"<<"\t"<<prevVal<<endl;

    if (prevVal >= 4) {
        //cout<<"\t"<<"Prediction:"<<"\t"<<"true"<<endl;
    }
    else {
        //cout<<"\t"<<"Prediction:"<<"\t"<<"false"<<endl;
    }

    //Update the branch predictor
    bimodUpdateBranchPredictor(itr, op);
    //cout<<"\t"<<"New PT value:"<<"\t"<<itr->second<<endl;

    //Update bimod stat
    bimodUpdateStat();
}

// GShare

//GShare data structures
string globalPC;
map <long, int> gShareTable;
long gShareAccess = 0;
long gShareRight = 0;
long gShareWrong = 0;

void initGlobalPC() {
    for (int i = 0; i < n; i++) {
        globalPC.push_back('0');
    }
}

void initGShareTable() {
    for (long i = 0; i < pow(2, m1); i++) {
        gShareTable.insert({ i, 4 });
    }
}

long gShareIndexMaker(string hexAdd) {
    //Convert the address into decimal
    long decAdd = stol(hexAdd, &sz, 16);
    //Discard the last two bits
    decAdd = decAdd / long(pow(2, 2));
    //Separate the last n bits
    long lastNBits = decAdd % long(pow(2, n));
    //Separate the first m-n bits
    long firstBits = decAdd / long(pow(2, n));
    //Xor the last n bits with globalPC
    long nDec = stol(globalPC, nullptr, 2);
    long xoredBits = lastNBits ^ nDec;
    //Add the first (m-n) bits in decimal * 2**(m-n) + xored last n bits
    long newAdd = firstBits * long(pow(2, n)) + xoredBits;
    //Determine the index using the m bits
    long index = newAdd % long(pow(2, m1));
    return(index);
}

void gShareUpdateStat() {
    gShareAccess += 1;
    lineNum += 1;
}

auto gShareUpdateBranchPredictor = [](auto itr, string op) {
    //Update the branch predictor
    long prevVal = itr->second;
    if (op == "t") {
        itr->second = min(itr->second + 1, 7);
        if (prevVal >= 4) {
            gShareRight += 1;
        }
        else {
            gShareWrong += 1;
        }
    }
    else {
        itr->second = max(itr->second - 1, 0);
        if (prevVal < 4) {
            gShareRight += 1;
        }
        else {
            gShareWrong += 1;
        }
    }
};

void globalPCUpdate(string op) {
    string newGlobalPC;
    if (op == "t") {
        newGlobalPC.push_back('1');
    }
    else {
        newGlobalPC.push_back('0');
    }
    for (auto itr = globalPC.begin(); itr != globalPC.end(); itr++) {
        newGlobalPC.push_back(*itr);
    }
    newGlobalPC.pop_back();
    globalPC = newGlobalPC;
    //cout<<"\t"<<"BHR now set to:"<<"\t";
    for (auto itr = globalPC.begin(); itr != globalPC.end(); itr++) {
        //cout<<"["<<*itr<<"]";
    }
    //cout<<endl;
}

void gShare(string hexAdd, string op) {

    //cout<<endl;
    //cout<<"<Line #"<<lineNum<<">"<<"\t"<<hexAdd<<"\t"<<op<<endl;

    long index = gShareIndexMaker(hexAdd);
    //cout<<"\t"<<"PT index:"<<"\t"<<index<<endl;

    //Check current prediction
    auto itr = gShareTable.find(index);
    int prevVal = itr->second;

    //cout<<"\t"<<"PT value:"<<"\t"<<prevVal<<endl;
    if (prevVal >= 4) {
        //cout<<"\t"<<"Prediction:"<<"\t"<<"true"<<endl;
    }
    else {
        //cout<<"\t"<<"Prediction:"<<"\t"<<"false"<<endl;
    }

    gShareUpdateBranchPredictor(itr, op);
    //cout<<"\t"<<"New PT value:"<<"\t"<<itr->second<<endl;

    globalPCUpdate(op);

    gShareUpdateStat();
}

// Hybrid

//Hybrid data structures
map <long, int> hybridTable;
long hybridAccess = 0;
long hybridRight = 0;
long hybridWrong = 0;

void initHybridTable() {
    for (long i = 0; i < pow(2, k); i++) {
        hybridTable.insert({ i, 1 });
    }
}

void hybrid(string hexAdd, string op) {
    //cout<<endl;
    //cout<<"<Line #"<<lineNum<<">"<<"\t"<<hexAdd<<"\t"<<op<<endl;

    //Obtain index from bimod and gshare
    long bimodIndex = bimodIndexMaker(hexAdd);
    long gShareIndex = gShareIndexMaker(hexAdd);
    long decAdd = stoi(hexAdd, &sz, 16);
    decAdd = decAdd / long(pow(2, 2));
    long hybridIndex = decAdd % long(pow(2, k));

    auto hybridItr = hybridTable.find(hybridIndex);
    auto bimodItr = bimodTable.find(bimodIndex);
    auto gShareItr = gShareTable.find(gShareIndex);

    int bimodPrevVal = bimodItr->second;
    int gSharePrevVal = gShareItr->second;
    //int hybridPrevVal = hybridItr->second;

    //cout<<"\t"<<"CT index:"<<"\t"<<hybridItr->first<<endl;
    //cout<<"\t"<<"CT value:"<<"\t"<<hybridItr->second<<endl;
    //cout<<"\t"<<"bimodal-PT index:"<<"\t"<<bimodItr->first<<endl;
    //cout<<"\t"<<"bimodal-PT value:"<<"\t"<<bimodItr->second<<endl;
    //cout<<"\t"<<"gshare-PT index:"<<"\t"<<gShareItr->first<<endl;
    //cout<<"\t"<<"gshare-PT value:"<<"\t"<<gShareItr->second<<endl;

    if (hybridItr->second >= 2) {
        //GShare
        if (gSharePrevVal >= 4) {
            //cout<<"\t"<<"Prediction:"<<"\t"<<"true"<<endl;
        }
        else {
            //cout<<"\t"<<"Prediction:"<<"\t"<<"false"<<endl;
        }
        gShareUpdateBranchPredictor(gShareItr, op);
        //cout<<"\t"<<"New gshare-PT value:"<<"\t"<<gShareItr->second<<endl;
        globalPCUpdate(op);
        if ((op == "t" && gSharePrevVal >= 4) || (op == "n" && gSharePrevVal < 4)) {
            hybridRight += 1;
        }
        else {
            hybridWrong += 1;
        }
    }
    else {
        //Bimod
        if (bimodPrevVal >= 4) {
            //cout<<"\t"<<"Prediction:"<<"\t"<<"true"<<endl;
        }
        else {
            //cout<<"\t"<<"Prediction:"<<"\t"<<"false"<<endl;
        }
        bimodUpdateBranchPredictor(bimodItr, op);
        //cout<<"\t"<<"New bimodal-PT value:"<<"\t"<<bimodItr->second<<endl;
        globalPCUpdate(op);
        if ((op == "t" && bimodPrevVal >= 4) || (op == "n" && bimodPrevVal < 4)) {
            hybridRight += 1;
        }
        else {
            hybridWrong += 1;
        }
    }

    if (op == "t") {
        if (gSharePrevVal >= 4 && bimodPrevVal < 4) {
            hybridItr->second = min(hybridItr->second + 1, 3);
        }
        if (gSharePrevVal < 4 && bimodPrevVal >= 4) {
            hybridItr->second = max(hybridItr->second - 1, 0);
        }
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

    hybridAccess += 1;
    lineNum += 1;
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
        initBimodTable();
    }

    if (opGShare.compare(opCode) == 0) {
        string m1Str = argv[2];
        string nStr = argv[3];
        m1 = stol(m1Str, &sz);
        n = stol(nStr, &sz);
        traceFile = argv[4];
        cout << "COMMAND" << endl;
        cout << "./sim gshare " << m1 << " " << n << " " << traceFile << endl; //m1=9, n=3
        initGlobalPC();
        initGShareTable();
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
        initBimodTable();
        initGlobalPC();
        initGShareTable();
        initHybridTable();
        cout << "COMMAND" << endl;
        cout << "./sim hybrid " << k << " " << m1 << " " << n << " " << m2 << " " << traceFile << endl;
    }

    readTraceFile(traceFile);

    for (unsigned long filePointer = 0; filePointer < traceFileContent.size(); filePointer++) {
        string hexAdd = traceFileContent[filePointer]->hexAdd;
        string op = traceFileContent[filePointer]->op;

        if (opBimodal.compare(opCode) == 0) {
            bimodal(hexAdd, op);
        }

        if (opGShare.compare(opCode) == 0) {
            gShare(hexAdd, op);
        }

        if (opHybrid.compare(opCode) == 0) {
            hybrid(hexAdd, op);
        }
    }

    if (opBimodal.compare(opCode) == 0) {
        cout << "OUTPUT" << endl;
        cout << "number of predictions:" << "\t" << "\t" << bimodAccess << endl;
        cout << "number of mispredictions:" << "\t" << bimodWrong << endl;
        cout << std::fixed;
        cout << "misprediction rate:" << "\t" << "\t" << std::setprecision(2) << float(bimodWrong) / float(bimodAccess) * 100 << "%" << endl;
        cout << "FINAL BIMODAL CONTENTS" << endl;
        for (auto itr = bimodTable.begin(); itr != bimodTable.end(); ++itr) {
            cout << itr->first << '\t' << itr->second << endl;
        }
    }

    if (opGShare.compare(opCode) == 0) {
        cout << "OUTPUT" << endl;
        cout << "number of predictions:" << "\t" << "\t" << gShareAccess << endl;
        cout << "number of mispredictions:" << "\t" << gShareWrong << endl;
        cout << std::fixed;
        cout << "misprediction rate:" << "\t" << "\t" << std::setprecision(2) << float(gShareWrong) / float(gShareAccess) * 100 << "%" << endl;
        cout << "FINAL GSHARE CONTENTS" << endl;
        for (auto itr = gShareTable.begin(); itr != gShareTable.end(); ++itr) {
            cout << itr->first << '\t' << itr->second << endl;
        }
    }

    if (opHybrid.compare(opCode) == 0) {
        cout << "OUTPUT" << endl;
        cout << "number of predictions:" << "\t" << "\t" << hybridAccess << endl;
        cout << "number of mispredictions:" << "\t" << hybridWrong << endl;
        cout << std::fixed;
        cout << "misprediction rate:" << "\t" << "\t" << std::setprecision(2) << float(hybridWrong) / float(hybridAccess) * 100 << "%" << endl;
        cout << "FINAL CHOOSER CONTENTS" << endl;
        for (auto itr = hybridTable.begin(); itr != hybridTable.end(); ++itr) {
            cout << itr->first << '\t' << itr->second << endl;
        }
        cout << "FINAL GSHARE CONTENTS" << endl;
        for (auto itr = gShareTable.begin(); itr != gShareTable.end(); ++itr) {
            cout << itr->first << '\t' << itr->second << endl;
        }
        cout << "FINAL BIMODAL CONTENTS" << endl;
        for (auto itr = bimodTable.begin(); itr != bimodTable.end(); ++itr) {
            cout << itr->first << '\t' << itr->second << endl;
        }
    }
}
