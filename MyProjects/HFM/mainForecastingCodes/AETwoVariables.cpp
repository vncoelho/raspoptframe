// ===================================
// Main.cpp file generated by OptFrame
// Project EFP
// ===================================

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "../../../OptFrame/RandGen.hpp"
#include "../../../OptFrame/Util/RandGenMersenneTwister.hpp"

using namespace std;
using namespace optframe;
using namespace EFP;

int AETwoVariables(int argc, char **argv)
{
	cout << "Welcome to AE batch with two variables!" << endl;
	RandGenMersenneTwister rg;
	//long  1412730737
	long seed = time(NULL); //CalibrationMode
	//seed = 1;
	cout << "Seed = " << seed << endl;
	srand(seed);
	rg.setSeed(seed);

	if (argc != 4)
	{
		cout << "Parametros incorretos!" << endl;
		cout << "Os parametros esperados sao: nome nomeValidationSet saida parameters options precision" << endl;
		exit(1);
	}

	int nTrainningRounds = atoi(argv[1]);
	double alphaLimitTemp = atof(argv[2]);
	int nTempVariables = atoi(argv[3]); //maxNumberOfInputVariables;

	cout << "nTrainningRounds=" << nTrainningRounds << endl;
	cout << "nTempVariables=" << nTempVariables << endl;
	cout << "alphaLimitTemp=" << alphaLimitTemp << endl;
	//===================================

	vector<string> explanatoryVariables;

	string col1 = "./MyProjects/EFP/Instance/AEManyVariables/task5Col1";
	string col2 = "./MyProjects/EFP/Instance/AEManyVariables/task5Col2";
	string col3 = "./MyProjects/EFP/Instance/AEManyVariables/task5Col3";
	string col4 = "./MyProjects/EFP/Instance/AEManyVariables/task5Col4";

	explanatoryVariables.push_back(col1);
	if (nTempVariables >= 1)
		explanatoryVariables.push_back(col2);
	if (nTempVariables >= 2)
		explanatoryVariables.push_back(col3);
	if (nTempVariables >= 3)
		explanatoryVariables.push_back(col4);

	treatForecasts rF(explanatoryVariables);

	int maxMu = 100;
	int maxInitialDesv = 10;
	int maxMutationDesv = 30;
	int maxPrecision = 300;

	int nBatches = 1;
	int timeES = 180;
	int timeVND = 0;
	int timeILS = 0;
	int timeGRASP = 0;

	vector<vector<double> > vfoIndicatorCalibration; //vector with the FO of each batch

	for (int n = 0; n < nBatches; n++)
	{
		int randomPrecision = rg.rand(maxPrecision) + 10;
		int evalFOMinimizer = rg.rand(NMETRICS); //tree is the number of possible objetive function index minimizers
		int evalAprox = rg.rand(2); //Enayatifar aproximation using previous values
		int construtive = rg.rand(3);
		double initialDesv = rg.rand(maxInitialDesv) + 1;
		double mutationDesv = rg.rand(maxMutationDesv) + 1;
		int mu = rg.rand(maxMu) + 1;
		int lambda = mu * 6;
		double alphaACF = rg.rand01(); //limit ACF for construtive ACF
		int alphaSign = rg.rand(2);
		if (alphaSign == 0)
			alphaACF = alphaACF * -1;

		// ============ FORCES ======================
		int randomParametersFiles = 0; // best file
		mu = 10;
		lambda = mu * 6;
		initialDesv = 10;
		mutationDesv = 20;
		randomPrecision = 50;		//10
		//randomParametersFiles = 0;
		evalFOMinimizer = MAPE_INDEX;
		evalAprox = 0;
		alphaACF = 1;
		construtive = 2;
		// ============ END FORCES ======================

		vector<double> vAlphaACFlimits;
		vAlphaACFlimits.push_back(0);

		for (int expVar = 1; expVar <= nTempVariables; expVar++)
		{
			vAlphaACFlimits.push_back(alphaLimitTemp);
		}

		// ============= METHOD PARAMETERS=================
		methodParameters methodParam;
		//seting up ES params
		methodParam.setESMU(mu);
		methodParam.setESLambda(lambda);
		methodParam.setESInitialDesv(initialDesv);
		methodParam.setESMutationDesv(mutationDesv);
		methodParam.setESMaxG(100000);
		//seting up Construtive params
		methodParam.setConstrutiveMethod(construtive);
		methodParam.setConstrutivePrecision(randomPrecision);
		methodParam.setConstrutiveLimitAlphaACF(vAlphaACFlimits);
		//seting up Eval params
		methodParam.setEvalAprox(evalAprox);
		methodParam.setEvalFOMinimizer(evalFOMinimizer);
		// ==========================================

		// ================== READ FILE ============== CONSTRUTIVE 0 AND 1
		ProblemParameters problemParam;

		problemParam.setStepsAhead(24);
		int stepsAhead = problemParam.getStepsAhead();

		// =========================================== CONSTRUTIVE 0 AND 1
		//========ADAPTATION FOR CONSTRUTIVE 2 ===============
		if (construtive == 2) //ACF construtive
			problemParam.setMaxLag(500);
		//=================================================

		int maxLag = problemParam.getMaxLag();

		int nTotalForecastingsTrainningSet = maxLag + nTrainningRounds * stepsAhead;
		cout << "nTotalForecastingsTrainningSet: " << nTotalForecastingsTrainningSet << endl;
		cout << "maxNotUsed: " << problemParam.getMaxLag() << endl;

		vector<vector<double> > trainningSet; // trainningSetVector
		trainningSet.push_back(rF.getPartsForecastsEndToBegin(0, stepsAhead, nTotalForecastingsTrainningSet));
		for (int expVar = 1; expVar <= nTempVariables; expVar++)
		{
			trainningSet.push_back(rF.getPartsForecastsEndToBegin(expVar, stepsAhead, nTotalForecastingsTrainningSet));
		}

		ForecastClass pFC(trainningSet, problemParam, rg, methodParam);

		pair<Solution<RepEFP>&, Evaluation&>* sol;
		sol = pFC.run(timeES, timeVND, timeILS);

		vector<double> foIndicatorCalibration;

		int nValidationSamples = maxLag + stepsAhead;

		vector<vector<double> > validationSet; //validation set for calibration
		validationSet.push_back(rF.getPartsForecastsEndToBegin(0, 0, nValidationSamples));
		for (int expVar = 1; expVar <= nTempVariables; expVar++)
		{
			validationSet.push_back(rF.getPartsForecastsEndToBegin(expVar, 0, nValidationSamples));
		}

		vector<double> foIndicatorsWeeks;
		foIndicatorsWeeks = pFC.returnErrors(sol, validationSet);
		foIndicatorCalibration.push_back(foIndicatorsWeeks[MAPE_INDEX]);
		foIndicatorCalibration.push_back(foIndicatorsWeeks[RMSE_INDEX]);
		foIndicatorCalibration.push_back(nTrainningRounds);
		foIndicatorCalibration.push_back(stepsAhead);
		foIndicatorCalibration.push_back(timeES);
		foIndicatorCalibration.push_back(nTempVariables);
		foIndicatorCalibration.push_back(alphaLimitTemp);
		foIndicatorCalibration.push_back(seed);

		vfoIndicatorCalibration.push_back(foIndicatorCalibration);
		//getchar();
	}

	// =================== PRINTING RESULTS ========================
	for (int n = 0; n < nBatches; n++)
	{

		for (int i = 0; i < vfoIndicatorCalibration[n].size(); i++)
			cout << vfoIndicatorCalibration[n][i] << "\t";

		cout << endl;
	}
	// =======================================================

	// =================== PRINTING RESULTS ON FILE ========================
	string calibrationFile = "./AEManyVariableResults";

	FILE* fResults = fopen(calibrationFile.c_str(), "a");
	for (int n = 0; n < nBatches; n++)
	{
		for (int i = 0; i < vfoIndicatorCalibration[n].size(); i++)
			fprintf(fResults, "%.7f\t", vfoIndicatorCalibration[n][i]);
		fprintf(fResults, "\n");
	}

	fclose(fResults);
	return 0;
}

