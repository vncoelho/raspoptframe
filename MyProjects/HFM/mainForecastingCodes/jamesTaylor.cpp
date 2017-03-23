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

int jamesTaylorEuropeanDataset(int argc, char **argv)
{

	cout << "Welcome to James Taylor European Dataset Analysis" << endl;
	RandGenMersenneTwister rg;
	//long  1412730737
	long seed = time(NULL); //CalibrationMode
	//seed = 9;
	cout << "Seed = " << seed << endl;
	srand(seed);
	rg.setSeed(seed);

	if (argc != 5)
	{
		cout << "Parametros incorretos!" << endl;
		cout << "Os parametros esperados sao: nomeOutput targetTS construtiveNRulesACF timeES" << endl;
		exit(1);
	}

	const char* caminhoOutput = argv[1];
	int argvTargetTimeSeries = atoi(argv[2]);
	int argvNumberOfRules = atoi(argv[3]);
	int argvTimeES = atoi(argv[4]);

	//double argvAlphaACF = atof(argv[4]);

	//
	//	const char* caminho = argv[1];
	//	const char* caminhoValidation = argv[2];
	//	const char* caminhoOutput = argv[3];
	//	const char* caminhoParameters = argv[4];
	//	int instN = atoi(argv[5]);
	//	int stepsAhead = atoi(argv[6]);
	//	int mu = atoi(argv[7]);

	string nomeOutput = caminhoOutput;

	//===================================
	cout << "Parametros:" << endl;
	cout << "nomeOutput=" << nomeOutput << endl;
	cout << "argvTargetTimeSeries=" << argvTargetTimeSeries << endl;
	cout << "argvNumberOfRules=" << argvNumberOfRules << endl;
	//cout << "argvAlphaACF=" << argvAlphaACF << endl;
	//	getchar();
	//	cout << "instN=" << instN << endl;
	//	cout << "stepsAhead=" << stepsAhead << endl;
	//	cout << "mu=" << mu << endl;
	//===================================

	//CONFIG FILES FOR CONSTRUTIVE 0 AND 1

	vector<string> explanatoryVariables;

	string fileJamesTaylorTrainning = "./MyProjects/HFM/Instance/EDS/Hourly/SpainTrainningDemands";
	string fileJamesTaylorValidation = "./MyProjects/HFM/Instance/EDS/Validation/Hourly/SpainValidationDemands";
	//explanatoryVariables.push_back(fileJamesTaylorTrainning);
	//explanatoryVariables.push_back(fileJamesTaylorValidation);

	string fileJamesTaylorTrainningSweden = "./MyProjects/HFM/Instance/EDS/Hourly/SwedenTrainningDemands";
	string fileJamesTaylorValidationSweden = "./MyProjects/HFM/Instance/EDS/Validation/Hourly/SwedenValidationDemands";

	string fileJamesTaylorHHBelgium = "./MyProjects/HFM/Instance/EDS/HalfHourly/BelgiumTrainningDemands";
	string fileJamesTaylorHHBelgiumV = "./MyProjects/HFM/Instance/EDS/Validation/HalfHourly/BelgiumValidationDemands";

	string fileJamesTaylorHHPortugal = "./MyProjects/HFM/Instance/EDS/HalfHourly/PortugalTrainningDemands";
	string fileJamesTaylorHHPortugalV = "./MyProjects/HFM/Instance/EDS/Validation/HalfHourly/PortugalValidationDemands";


	string fileEirGrid = "./MyProjects/HFM/Instance/EirGrid/trainingSetEirGrid";
	string fileEirGridV = "./MyProjects/HFM/Instance/EirGrid/validationSetEirGrid";


	explanatoryVariables.push_back(fileJamesTaylorHHPortugal);
	explanatoryVariables.push_back(fileJamesTaylorHHPortugalV);

	treatForecasts rF(explanatoryVariables);

	//vector<vector<double> > batchOfBlindResults; //vector with the blind results of each batch

	/*int beginValidationSet = 0;
	 int nTrainningRoundsValidation = 50;
	 int nValidationSamples = problemParam.getNotUsedForTest() + nTrainningRoundsValidation * stepsAhead;
	 cout << "nValidationSamples = " << nValidationSamples << endl;
	 int nTotalForecastingsValidationSet = nValidationSamples;

	 vector<vector<double> > validationSet; //validation set for calibration
	 validationSet.push_back(rF.getPartsForecastsEndToBegin(0, beginValidationSet, nTotalForecastingsValidationSet));
	 validationSet.push_back(rF.getPartsForecastsEndToBegin(1, beginValidationSet, nTotalForecastingsValidationSet + stepsAhead));
	 validationSet.push_back(rF.getPartsForecastsEndToBegin(2, beginValidationSet, nTotalForecastingsValidationSet + stepsAhead));
	 */

	int mu = 100;
	int lambda = mu * 6;
	double initialDesv;
	double mutationDesv;

	int nBatches = 1;

	int timeES = 180;
	int timeVND = 0;
	int timeILS = 0;
	int timeGRASP = 0;

	vector<vector<double> > vfoIndicatorCalibration; //vector with the FO of each batch

	vector<SolutionEFP> vSolutionsBatches; //vector with the solution of each batch

	for (int n = 0; n < nBatches; n++)
	{
		int mu = 100;
		int lambda = mu * 6;
		int evalFOMinimizer = MAPE_INDEX;
		int contructiveNumberOfRules = 500;
		int evalAprox = 5;
		double alphaACF = 1;
		int construtive = 2;
		// ============ END FORCES ======================

		// ============= METHOD PARAMETERS=================
		methodParameters methodParam;
		//seting up Continous ES params
		methodParam.setESInitialDesv(10);
		methodParam.setESMutationDesv(20);
		methodParam.setESMaxG(100000);

		//seting up ES params
		methodParam.setESMU(mu);
		methodParam.setESLambda(lambda);

		//seting up ACF construtive params
		methodParam.setConstrutiveMethod(construtive);
		methodParam.setConstrutivePrecision(contructiveNumberOfRules);
		vector<double> vAlphaACFlimits;
		vAlphaACFlimits.push_back(alphaACF);
		methodParam.setConstrutiveLimitAlphaACF(vAlphaACFlimits);

		//seting up Eval params
		methodParam.setEvalAprox(evalAprox);
		methodParam.setEvalFOMinimizer(evalFOMinimizer);
		// ==========================================

		// ================== READ FILE ============== CONSTRUTIVE 0 AND 1
		ProblemParameters problemParam;
		//ProblemParameters problemParam(vParametersFiles[randomParametersFiles]);
		int nSA = 24;
		problemParam.setStepsAhead(nSA);
		int stepsAhead = problemParam.getStepsAhead();
		//========SET PROBLEM MAXIMUM LAG ===============
		problemParam.setMaxLag(672);
		int maxLag = problemParam.getMaxLag();

		int nTrainningRounds = 100;
		int nTotalForecastingsTrainningSet = maxLag + nTrainningRounds * stepsAhead;
		cout << "nTrainningRounds: " << nTrainningRounds << endl;
		cout << "nTotalForecastingsTrainningSet: " << nTotalForecastingsTrainningSet << endl;

		vector<vector<double> > trainningSet; // trainningSetVector
		int beginTrainingSet = 0;
		trainningSet.push_back(rF.getPartsForecastsEndToBegin(0, beginTrainingSet, nTotalForecastingsTrainningSet));

		ForecastClass forecastObject(trainningSet, problemParam, rg, methodParam);

		pair<Solution<RepEFP>&, Evaluation&>* sol;

//		cout<<"Teste Linear Regression"<<endl;
//		sol = forecastObject.runOLR();
//		cout<<"Linear Regression OK"<<endl;
//		getchar();

		int optMethod = rg.rand(2);
		optMethod = 0;
		if (optMethod == 0)
			sol = forecastObject.run(timeES, timeVND, timeILS);
		else
			sol = forecastObject.runGILS(timeGRASP, timeILS); // GRASP + ILS

		vector<double> validationNotUsed;
		validationNotUsed = rF.getLastForecasts(0, maxLag);
		vector<double> validationSetPure;
		validationSetPure = rF.getLastForecasts(1, rF.getForecastsSize(1));
		vector<double> validationSetPlusNotUsed = validationNotUsed;
		validationSetPlusNotUsed.insert(validationSetPlusNotUsed.end(), validationSetPure.begin(), validationSetPure.end());
		vector<vector<double> > validationSet; //validation set for calibration
		validationSet.push_back(validationSetPlusNotUsed);
		//validationSet.push_back(validationSetPure);

		double intervalOfBeginTrainningSet = double(beginTrainingSet) / double(rF.getForecastsDataSize());

		vector<double> vForecasts = forecastObject.returnForecasts(sol, validationSet);
		cout<<vForecasts<<endl;
		cout<<vForecasts.size()<<endl;


		vector<double> foIndicatorCalibration;
		foIndicatorCalibration = forecastObject.returnErrors(sol, validationSet);
		foIndicatorCalibration.push_back(nTrainningRounds);
		foIndicatorCalibration.push_back(beginTrainingSet);
		foIndicatorCalibration.push_back(intervalOfBeginTrainningSet);
		foIndicatorCalibration.push_back(nTotalForecastingsTrainningSet);
		foIndicatorCalibration.push_back(mu);
		foIndicatorCalibration.push_back(lambda);
		foIndicatorCalibration.push_back(initialDesv);
		foIndicatorCalibration.push_back(mutationDesv);
		foIndicatorCalibration.push_back(timeES);
		foIndicatorCalibration.push_back(timeVND);
		foIndicatorCalibration.push_back(timeILS);
		foIndicatorCalibration.push_back(timeGRASP);
		foIndicatorCalibration.push_back(evalFOMinimizer);
		foIndicatorCalibration.push_back(evalAprox);
		foIndicatorCalibration.push_back(construtive);
		foIndicatorCalibration.push_back(alphaACF);
		foIndicatorCalibration.push_back(optMethod);
		foIndicatorCalibration.push_back(seed);
		//getchar();
		//cout << foIndicatorCalibration << endl;
		vfoIndicatorCalibration.push_back(foIndicatorCalibration);
		vSolutionsBatches.push_back(sol->first);

	}

	cout << vfoIndicatorCalibration << endl;
	for (int n = 0; n < nBatches; n++)
	{

		for (int i = 0; i < vfoIndicatorCalibration[n].size(); i++)
			cout << vfoIndicatorCalibration[n][i] << "\t";

		cout << endl;
	}

	string calibrationFile = "./JamesTaylorSpainCalibration";

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