#ifndef INITIALIZATION
#define INITIALIZATION

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <cstdlib>

using namespace std;

struct ProcessedInput{
  int numSNPs;
  int numWindows;
  int numHaps;
  int numAncPops;
  int numAdm;
  int numAnc;
  double windowSize;
  int numTrees;
  int genSinceAdmixture;
  vector<double> * windowBeginLocs;
  vector<double> * windowEndLocs;
  vector<int> * windowBeginIndexes;
  vector<int> * windowEndIndexes;
  int ** ancHapClassesPerWindow;
  int ** ancWindows;
  int ** admWindows;
  string outputName;
  bool allowParallel;
  int bootstrapSampleSize;
  int bootstrappingMethod;
  int numEmIterations;
  vector<int> * hapClasses;
  int callAnc;    // Include anc in adm after first iteration
  int useAnc;    // Use anc as anc after first iteration
  unsigned int mtryFactor;
  bool doForwardBackward;
};

ProcessedInput* processInput(int argc, char ** argv, ofstream * logfile);

#endif
