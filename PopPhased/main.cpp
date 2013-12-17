#include <iostream>
#include <vector>

#include "getdata.h"
#include "randomforest.h"
#include "crfviterbi.h"
#include "windowtosnp.h"

using namespace std;

// First iteration just uses those specified as anc to make calls just on those specified as adm

int main (int argc, char ** argv) {
    //  cout << argc << endl;
    
    //Step 0: Start a log file
    ofstream logfile;
    
    printf("Giddyup\n");
    
    printf("Reading files\n");
    ProcessedInput * processedInput = processInput(argc, argv, &logfile);
    
    int numWindows = processedInput->numWindows;
    int numAdm = processedInput->numAdm;  // # of admixed haplotypes
    int numAncPops = processedInput->numAncPops;
    vector<double> * windowBeginLocs = processedInput->windowBeginLocs;
    vector<double> * windowEndLocs = processedInput->windowEndLocs;
    vector<int> * windowBeginIndexes = processedInput->windowBeginIndexes;
    vector<int> * windowEndIndexes = processedInput->windowEndIndexes;
    int ** admWindows = processedInput->admWindows;
    int numEmIterations = processedInput->numEmIterations;
    int useAnc = processedInput->useAnc;
    int* numAdmPhasings = processedInput->numAdmPhasings;
        
    // For each window, columns are Ind1Phasing1Chrom1Anc1, Ind1Phasing1Chrom1Anc2, Ind1Phasing1Chrom2Anc1
    double** rfProbs = new double*[numWindows];
    for(int windex = 0; windex < numWindows; windex++){
        rfProbs[windex] = new double[2*numAdmPhasings[windex] * numAncPops];
    }
    
    int** windowCalls = new int*[numAdm];
    for(int i = 0; i < numAdm; i++){
        windowCalls[i] = new int[numWindows];
    }
    
    double** marginals = new double*[numAdm];
    for(int admIndex = 0; admIndex < numAdm; admIndex++){
        marginals[admIndex] = new double[numWindows * numAncPops];
    }
    
    printf("Growing forests\n");
    rfProbs = RandomForest(processedInput, rfProbs, 0);
    
    printf("Applying Conditional Random Fields\n");
    windowCalls = CrfViterbi(processedInput, rfProbs, windowCalls, marginals, 0);
    
    printf("Converting Window Calls to SNP Calls and Writing to File\n");
    WindowToSnpCalls(processedInput, windowCalls, 0, marginals);
    
    if(numEmIterations > 0){
        if(useAnc == 1){
            // Revert ancestral calls back to original because used themselves to call
            vector<int> * hapClasses = processedInput->hapClasses;
            for(int i = 0; i < numAdm; i++){
                if((*hapClasses)[i] != 0){
                    for(int windex = 0; windex < numWindows; windex++){
                        windowCalls[i][windex] = (*hapClasses)[i] - 1;
                    }
                }
            }
        }
        
        // Make ancestrals all of our called admixed+ancestral. Note: already made admWindows everyone in getdata
        for(int arrayToDeleteIndex = 0; arrayToDeleteIndex < numWindows; arrayToDeleteIndex++){
            delete[] processedInput->ancWindows[arrayToDeleteIndex];
        }
        delete[] processedInput->ancWindows;
        processedInput->ancWindows = processedInput->admWindows;
        if(processedInput->bootstrapSampleSize == processedInput->numAnc){
            // User didn't specify so automatically update this, otherwise leave the same
            processedInput->bootstrapSampleSize = processedInput->numAdm;
        }
        processedInput->numAnc = processedInput->numAdm;
        
        // EM: we're including called adm as anc now so have to change scale
        for(int windex = 0; windex < numWindows; windex++){
            delete[] processedInput->ancHapClassesPerWindow[windex];
            processedInput->ancHapClassesPerWindow[windex] = new int[processedInput->numAnc];
        }
        
        cout << "Running EM" << endl;
    }
    
    for(int emIteration = 1; emIteration <= numEmIterations; emIteration++){
        cout << "EM Iteration: " << emIteration << endl;
        // Modify processedInput for EM
        // Put in calls from last time. Need to transpose
        for(int i = 0; i < numAdm; i++){
            for(int windex = 0; windex < numWindows; windex++){
                processedInput->ancHapClassesPerWindow[windex][i] = windowCalls[i][windex];
            }
        }
        
        // Run RF
        rfProbs = RandomForest(processedInput, rfProbs, emIteration);
        
        // Run CRF
        windowCalls = CrfViterbi(processedInput, rfProbs, windowCalls, marginals, emIteration);
        
        // Print Iteration Results
        WindowToSnpCalls(processedInput, windowCalls, emIteration, marginals);
      
        // Note: processedInput->ancWindows already points at processedInput->admWindows
    }
    
    printf("Cleaning up\n");
    delete windowBeginLocs;
    delete windowEndLocs;
    delete windowBeginIndexes;
    delete windowEndIndexes;
    for(int arrayToDeleteIndex = 0; arrayToDeleteIndex < numWindows; arrayToDeleteIndex++){
        delete[] admWindows[arrayToDeleteIndex];
    }
    delete[] admWindows;
    
    for(int i = 0; i < numAdm; i++){
        delete[] rfProbs[i];
        delete[] marginals[i];
    }
    delete[] rfProbs;
    delete[] marginals;
    
    for(int i = 0; i < numAdm; i++){
        delete[] windowCalls[i];
    }
    delete[] windowCalls;
    for(int i = 0; i < numWindows; i++){
        delete[] processedInput->ancHapClassesPerWindow[i];
    }
    delete[] processedInput->ancHapClassesPerWindow;
    
    delete processedInput->hapClasses;
    
    delete processedInput;
    
    for(int windex = 0; windex < numWindows; windex++){
        delete[] processedInput->admWindowsPhasings[windex];
    }
    delete[] processedInput->admWindowsPhasings;
    
    for(int windex = 0; windex < numWindows; windex++){
        delete[] processedInput->numHetSitesPerInd[windex];
    }
    delete[] processedInput->numHetSitesPerInd;
    
    delete[] processedInput->numAdmPhasings;
    
    logfile.close();
    
    return 0;
}
