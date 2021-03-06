// ===================================
// Main.cpp file generated by OptFrame
// Project EFP
// ===================================

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <numeric>
#include "../../../OptFrame/RandGen.hpp"
#include "../../../OptFrame/Util/RandGenMersenneTwister.hpp"

using namespace std;
using namespace optframe;
using namespace EFP;
extern int nThreads;

int stockMarketForecasting(int argc, char **argv)
{
	nThreads = 4;
	cout << "Welcome to stock market forecasting!" << endl;
	RandGenMersenneTwister rg;
	//long  1412730737
	long seed = time(NULL); //CalibrationMode
	seed = 0;
	cout << "Seed = " << seed << endl;
	srand(seed);
	rg.setSeed(seed);

	if (argc != 2)
	{
		cout << "Parametros incorretos!" << endl;
		cout << "Os parametros esperados sao: nomeOutput targetTS construtiveNRulesACF timeES" << endl;
		exit(1);
	}

	const char* caminhoOutput = argv[1];

	string nomeOutput = caminhoOutput;
	//===================================
	cout << "Parametros:" << endl;
	cout << "nomeOutput=" << nomeOutput << endl;

	//Numero de passos a frente - Horizonte de previsao
	int fh = 30;
	//O valor mais antigo que pode ser utilizado como entrada do modelo de previsao [100]
	int argvMaxLagRate = 10;

	vector<string> explanatoryVariables;

	explanatoryVariables.push_back(nomeOutput);

	treatForecasts rF(explanatoryVariables);

	//Parametros do metodo
	int mu = 100;
	int lambda = mu * 6;
	int evalFOMinimizer = MAPE_INDEX;
	int contructiveNumberOfRules = 100;
	int evalAprox = 0;
	double alphaACF = -1;
	int construtive = 2;
	// ============ END FORCES ======================

	// ============= METHOD PARAMETERS=================
	methodParameters methodParam;
	//seting up ES params
	methodParam.setESMU(mu);
	methodParam.setESLambda(lambda);
	methodParam.setESMaxG(10000);

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

	problemParam.setStepsAhead(fh);

	int nTotalForecastingsTrainningSet = rF.getForecastsSize(0) - fh;

	//========SET PROBLEM MAXIMUM LAG ===============
	cout << "argvMaxLagRate = " << argvMaxLagRate << endl;

	int iterationMaxLag = ((nTotalForecastingsTrainningSet - fh) * argvMaxLagRate) / 100.0;
	iterationMaxLag = ceil(iterationMaxLag);
	if (iterationMaxLag > (nTotalForecastingsTrainningSet - fh))
		iterationMaxLag--;
	if (iterationMaxLag <= 0)
		iterationMaxLag = 1;

	problemParam.setMaxLag(iterationMaxLag);
	int maxLag = problemParam.getMaxLag();

	//If maxUpperLag is greater than 0 model uses predicted data
	problemParam.setMaxUpperLag(0);
	//int maxUpperLag = problemParam.getMaxUpperLag();
	//=================================================

	vector<double> foIndicators;
	int beginTrainingSet = 0;
	cout << std::setprecision(9);
	cout << std::fixed;
	double NTRaprox = (nTotalForecastingsTrainningSet - maxLag) / double(fh);
	cout << "BeginTrainninningSet: " << beginTrainingSet << endl;
	cout << "#nTotalForecastingsTrainningSet: " << nTotalForecastingsTrainningSet << endl;
	cout << "#~NTR: " << NTRaprox << endl;
	cout << "#timeSeriesSize: " << rF.getForecastsSize(0) << endl;
	cout << "#maxNotUsed: " << maxLag << endl;
	cout << "#StepsAhead: " << fh << endl << endl;

	vector<vector<double> > trainningSet; // trainningSetVector
	trainningSet.push_back(rF.getPartsForecastsEndToBegin(0, fh, nTotalForecastingsTrainningSet));

//		forecastObject.runMultiObjSearch();
//		getchar();
	pair<Solution<RepEFP>&, Evaluation&>* sol;
	Pareto<RepEFP>* pf = new Pareto<RepEFP>();

	ForecastClass* forecastObject;
	int timeES = 10;
	int timeGPLS = 30;
	for (int b = 0; b < 2; b++)
	{
		if(b==1)
			methodParam.setEvalFOMinimizer(MAPE_INV_INDEX);
		forecastObject = new ForecastClass(trainningSet, problemParam, rg, methodParam);
		sol = forecastObject->run(timeES, 0, 0);
		forecastObject->addSolToParetoWithFCMEV(sol->first, *pf);
		pf = forecastObject->runMultiObjSearch(timeGPLS, pf);
	}



	vector<MultiEvaluation*> vEvalPF = pf->getParetoFront();
	vector<Solution<RepEFP>*> vSolPF = pf->getParetoSet();
	int nObtainedParetoSol = vEvalPF.size();
	for (int i = 0; i < nObtainedParetoSol; i++)
	{
		//vector<double> solEvaluations;
		for (int e = 0; e < vEvalPF[i]->size(); e++)
			cout << vEvalPF[i]->at(e).getObjFunction() << "\t";
		cout << endl;
	}
	pf->exportParetoFront("paretoFrontGPLS.txt");

	//Validacao
//	vector<vector<double> > validationSet;
//	validationSet.push_back(rF.getPartsForecastsEndToBegin(0, 0, maxLag + fh));
//	vector<double> errors = forecastObject.returnErrors(sol, trainningSet);
//	vector<double> previsao = forecastObject.returnForecasts(sol, validationSet);
//
//	foIndicators.push_back(errors[MAPE_INDEX]);
//	foIndicators.push_back(sol->second.evaluation());
//	foIndicators.push_back(argvMaxLagRate);
//	foIndicators.push_back(maxLag);
//	foIndicators.push_back(NTRaprox);
//	foIndicators.push_back(timeES);
//	foIndicators.push_back(seed);
//
//	cout << setprecision(3);
//	cout << foIndicators << endl;
//	cout << errors << endl;
//	cout << previsao << endl;
//
//	cout << "printing real values!" << endl;
//	for (int b = 93; b < validationSet[0].size(); b++)
//		cout << validationSet[0][b] << "\t";
//	cout << endl;
//
//	cout << sol->first.getR() << endl;

	cout << "MO Stock Market forecasting finished!" << endl;
	return 0;
}

