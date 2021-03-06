/*
 * ForecastClass.hpp
 *
 *  Created on: 10/09/2014
 *      Author: vitor
 */

#ifndef FORECASTCLASS_HPP_
#define FORECASTCLASS_HPP_

#include "Evaluator.hpp"
#include "MultiEvaluatorHFM.hpp"
#include "Evaluation.h"
#include "ProblemInstance.hpp"
#include "HFMESContinous.hpp"
#include "treatForecasts.hpp"
#include "../../OptFrame/Heuristics/EvolutionaryAlgorithms/ES.hpp"
#include "../../OptFrame/Heuristics/Empty.hpp"
#include "../../OptFrame/NSSeq.hpp"
#include "../../OptFrame/Heuristics/LocalSearches/VND.h"
#include "../../OptFrame/Heuristics/LocalSearches/VariableNeighborhoodDescent.hpp"
#include "../../OptFrame/Heuristics/LocalSearches/FirstImprovement.hpp"
#include "../../OptFrame/Heuristics/LocalSearches/RandomDescentMethod.hpp"
#include "../../OptFrame/Heuristics/ILS/IteratedLocalSearchLevels.hpp"
#include "../../OptFrame/Heuristics/GRASP/BasicGRASP.hpp"
//#include "../../OptFrame/Heuristics/VNS/MOVNSLevels.hpp"
#include "../../OptFrame/Heuristics/2PPLS.hpp"
#include "../../OptFrame/MultiEvaluator.hpp"
#include "../../OptFrame/MultiObjSearch.hpp"
#include "../../OptFrame/Util/UnionNDSets.hpp"

#include "../../OptFrame/Heuristics/MOLocalSearches/MOBestImprovement.hpp"
#include "../../OptFrame/Heuristics/MOLocalSearches/MORandomImprovement.hpp"
#include "../../OptFrame/Heuristics/MOLocalSearches/GPLS.hpp"

namespace EFP
{

class ForecastClass
{
private:
	ProblemInstance* p;
	EFPEvaluator* eval;
	GRConstructive<RepEFP>* c;
	vector<NSSeq<RepEFP>*> vNS;
	RandGen& rg;
	EmptyLocalSearch<RepEFP> emptyLS;
	vector<NSSeq<RepEFP>*> vNSeq;

	EFPESContinous* EsCOpt;
	ES<RepEFP>* es;

	vector<vector<double> >& tForecastings;
	vector<LocalSearch<RepEFP>*> vLS;

	VariableNeighborhoodDescent<RepEFP>* vnd;
	IteratedLocalSearchLevels<RepEFP>* ils;

	vector<vector<double> > vBlindResults;
	vector<pair<Solution<RepEFP>&, Evaluation&>*> vFinalSol;

	ILSLPerturbationLPlus2<RepEFP>* ilsPert;
	vector<NS<RepEFP>*> vizPert;

	ProblemParameters& problemParam;
	methodParameters& methodParam;

	//OptimalLinearRegression* olr;
	MultiEvaluator<RepEFP>* mev;

public:

	ForecastClass(vector<vector<double> >& _tForecastings, ProblemParameters& _problemParam, RandGen& _rg, methodParameters& _methodParam) :
			tForecastings(_tForecastings), problemParam(_problemParam), rg(_rg), methodParam(_methodParam)
	{

		p = new ProblemInstance(tForecastings, problemParam);
		eval = new EFPEvaluator(*p, problemParam, methodParam.getEvalFOMinimizer(), methodParam.getEvalAprox());

		int nIntervals = 5;

		NSSeqNEIGHModifyRules* nsModifyFuzzyRules = new NSSeqNEIGHModifyRules(*p, rg);
		NSSeqNEIGHVAlpha* nsVAlpha = new NSSeqNEIGHVAlpha(*p, rg, nIntervals);
		NSSeqNEIGHChangeSingleInput* nsChangeSingleInput = new NSSeqNEIGHChangeSingleInput(*p, rg, problemParam.getMaxLag(), problemParam.getMaxUpperLag());
		NSSeqNEIGHRemoveSingleInput* nsRemoveSingleInput = new NSSeqNEIGHRemoveSingleInput(rg);
		NSSeqNEIGHAddSingleInput* nsAddSingleInput = new NSSeqNEIGHAddSingleInput(*p, rg, problemParam.getMaxLag(), problemParam.getMaxUpperLag());
//		NSSeqNEIGHAddX* nsAddMean01 = new NSSeqNEIGHAddX(*p, rg, 0.1);
//		NSSeqNEIGHAddX* nsAddMean1 = new NSSeqNEIGHAddX(*p, rg, 1);
//		NSSeqNEIGHAddX* nsAddMeanD15 = new NSSeqNEIGHAddX(*p, rg, p->getMean(0) / 15);
//		NSSeqNEIGHAddX* nsAddMeanD6 = new NSSeqNEIGHAddX(*p, rg, p->getMean(0) / 6);
//		NSSeqNEIGHAddX* nsAddMeanD2 = new NSSeqNEIGHAddX(*p, rg, p->getMean(0) / 2);
//		NSSeqNEIGHAddX* nsAddMean = new NSSeqNEIGHAddX(*p, rg, p->getMean(0));
//		NSSeqNEIGHAddX* nsAddMeanM2 = new NSSeqNEIGHAddX(*p, rg, 2 * p->getMean(0));
//		NSSeqNEIGHAddX* nsAddMeanM5 = new NSSeqNEIGHAddX(*p, rg, 5 * p->getMean(0));
//		NSSeqNEIGHAddX* nsAddMeanBigM = new NSSeqNEIGHAddX(*p, rg, 100 * p->getMean(0));

		int cMethod = methodParam.getConstrutiveMethod();
		int cPre = methodParam.getConstrutivePrecision();

		if (cMethod == 0)
			c = new ConstructiveRandom(*p, problemParam, rg, cPre);
		if (cMethod == 2)
			c = new ConstructiveACF(*p, problemParam, rg, cPre, methodParam.getConstrutiveLimitAlphaACF());

		FirstImprovement<RepEFP>* fiModifyFuzzyRules = new FirstImprovement<RepEFP>(*eval, *nsModifyFuzzyRules);
		FirstImprovement<RepEFP>* fiChangeSingleInput = new FirstImprovement<RepEFP>(*eval, *nsChangeSingleInput);
		FirstImprovement<RepEFP>* fiVAlpha = new FirstImprovement<RepEFP>(*eval, *nsVAlpha);
//		int maxRDM = 100;
//		RandomDescentMethod<RepEFP>* rdm = new RandomDescentMethod<RepEFP>(*eval, *ns, maxRDM);
//		//rdm->setMessageLevel(3);
		vLS.push_back(fiVAlpha);
		vLS.push_back(fiModifyFuzzyRules);
		vLS.push_back(fiChangeSingleInput);
		vnd = new VariableNeighborhoodDescent<RepEFP>(*eval, vLS);

		vizPert.push_back(nsModifyFuzzyRules);
		vizPert.push_back(nsVAlpha);
		vizPert.push_back(nsChangeSingleInput);
		ilsPert = new ILSLPerturbationLPlus2<RepEFP>(*eval, 50, *vizPert[0], rg);
		ilsPert->add_ns(*vizPert[1]);
		ilsPert->add_ns(*vizPert[2]);
		ils = new IteratedLocalSearchLevels<RepEFP>(*eval, *c, *vnd, *ilsPert, 100, 10);

		int mu = methodParam.getESMU();
		int lambda = methodParam.getESLambda();
		int esMaxG = methodParam.getESMaxG();
		int initialDesv = methodParam.getESInitialDesv();
		int mutationDesv = methodParam.getESMutationDesv();
		//cout<<mu<<"\t"<<lambda<<"\t"<<esMaxG<<"\t"<<initialDesv<<"\t"<<mutationDesv<<endl;
		//getchar();
		EsCOpt = new EFPESContinous(*eval, *c, vNSeq, emptyLS, mu, lambda, esMaxG, rg, initialDesv, mutationDesv);

		//olr = new OptimalLinearRegression(*eval, *p);

		vNSeq.push_back(nsModifyFuzzyRules);
		vNSeq.push_back(nsChangeSingleInput);
		vNSeq.push_back(nsRemoveSingleInput);
		vNSeq.push_back(nsAddSingleInput);
//		vNSeq.push_back(nsAddMean01);
//		vNSeq.push_back(nsAddMean1);
//		vNSeq.push_back(nsAddMeanD15);
//		vNSeq.push_back(nsAddMeanD6);
//		vNSeq.push_back(nsAddMeanD2);
//		vNSeq.push_back(nsAddMean);
//		vNSeq.push_back(nsAddMeanM2);
//		vNSeq.push_back(nsAddMeanM5);
//		vNSeq.push_back(nsAddMeanBigM);
//		vNSeq.push_back(nsVAlpha);

		//TODO check why ES goes more generations some time when we do not have improvements.

		vector<int> vNSeqMax(vNSeq.size(), 1000);
		double mutationRate = 0.1;
		int selectionType = 1;
		string outputFile = "LogPopFOPlus";
		es = new ES<RepEFP>(*eval, *c, vNSeq, vNSeqMax, emptyLS, selectionType, mutationRate, rg, mu, lambda, esMaxG, outputFile, 0);
		es->setMessageLevel(3);

		//MO
		vector<Evaluator<RepEFP>*> v_e;
		v_e.push_back(new EFPEvaluator(*p, problemParam, MAPE_INDEX, 0));
		v_e.push_back(new EFPEvaluator(*p, problemParam, MAPE_INV_INDEX, 0));
		v_e.push_back(new EFPEvaluator(*p, problemParam, RMSE_INDEX, 0));
		v_e.push_back(new EFPEvaluator(*p, problemParam, WMAPE_INDEX, 0));
		v_e.push_back(new EFPEvaluator(*p, problemParam, SMAPE_INDEX, 0));
		v_e.push_back(new EFPEvaluator(*p, problemParam, MMAPE_INDEX, 0));
		mev = new MultiEvaluator<RepEFP>(v_e);
	}

	virtual ~ForecastClass()
	{
//		tForecastings.clear();

		delete p;
		delete eval;
		delete c;

		for (int i = 0; i < vLS.size(); i++)
			delete vLS[i];
		vLS.clear();

		for (int i = 0; i < vNSeq.size(); i++)
			delete vNSeq[i];
		vNSeq.clear();

		//Do not delete because vNSeq may contain the same NS
//		for (int i = 0; i < vizPert.size(); i++)
//			delete vizPert[i];
//		vizPert.clear();

		delete ilsPert; //todo verify
		delete ils; //todo verify
		delete vnd;
		delete EsCOpt;

	}


	//add solution to pareto front evaluating with forecasting class evaluators
	void addSolToParetoWithFCMEV(Solution<RepEFP>& s, Pareto<RepEFP>& pf)
	{
		pf.push_back(s, *mev);
	}

	Pareto<RepEFP>* runMultiObjSearch(int timeGPLS, Pareto<RepEFP>* pf = NULL)
	{

//		HFMMultiEvaluator mev(*new EFPEvaluator(*p, problemParam, MAPE_INDEX, 0));

//		if (vS != NULL)
//		{
//			if (pf == NULL)
//			{
//				pf = new Pareto<RepEFP>();
//
//				for (int ns = 0; ns < vS.size(); ns++)
//					pf->push_back(*vS[ns], mev);
//			}
//			else
//			{
//				for (int ns = 0; ns < vS.size(); ns++)
//					pf->push_back(*vS[ns], mev);
//			}
//		}

		int initial_population_size = 10;
		GRInitialPopulation<RepEFP> bip(*c, rg, 1);
//		MOVNSLevels<RepEFP> multiobjectvns(v_e, bip, initial_population_size, vNSeq, rg, 10, 10);
		GRInitialPareto<RepEFP> grIP(*c, rg, 1, *mev);
		MORandomImprovement<RepEFP> moriCSI(*mev, *vNSeq[0], 1000);
		MORandomImprovement<RepEFP> moriRSI(*mev, *vNSeq[1], 1000);
		MORandomImprovement<RepEFP> moriASI(*mev, *vNSeq[2], 1000);
		MORandomImprovement<RepEFP> moriMFR(*mev, *vNSeq[3], 1000);
		vector<MOLocalSearch<RepEFP>*> vMOLS;
		vMOLS.push_back(&moriCSI);
		vMOLS.push_back(&moriRSI);
		vMOLS.push_back(&moriASI); //Todo -- CHECK THIS NS -- IT IS PRODUCING ERRORS INSIDE G2PPLS
		vMOLS.push_back(&moriMFR);

		GeneralParetoLocalSearch<RepEFP> generalPLS(*mev, grIP, initial_population_size, vMOLS);
		if (pf == NULL)
		{
			pf = generalPLS.search(timeGPLS, 0);
		}
		else
		{
			pf = generalPLS.search(timeGPLS, 0, pf);
		}

//		vector<MultiEvaluation*> vEval = pf->getParetoFront();
//		vector<Solution<RepEFP>*> vSolPf = pf->getParetoSet();
//
//		int nObtainedParetoSol = vEval.size();
//		for (int i = 0; i < nObtainedParetoSol; i++)
//		{
//			//vector<double> solEvaluations;
//			for (int e = 0; e < vEval[i]->size(); e++)
//				cout << vEval[i]->at(e).getObjFunction() << "\t";
//			cout << endl;
//
//		}

//		mev->clear();
		return pf;
	}

	pair<Solution<RepEFP>&, Evaluation&>* runOLR()
	{

		pair<Solution<RepEFP>&, Evaluation&>* finalSol = NULL;
		finalSol = EsCOpt->search(1);

		//olr->exec(finalSol->first, 100, 100);

		return finalSol;
	}

	pair<Solution<RepEFP>&, Evaluation&>* runGRASP(int timeGRASP, int nSol)
	{
		BasicGRASP<RepEFP> g(*eval, *c, emptyLS, 0.1, nSol);
		g.setMessageLevel(3);
		pair<Solution<RepEFP>&, Evaluation&>* finalSol;

		finalSol = g.search(timeGRASP);
		return finalSol;
	}

	pair<Solution<RepEFP>&, Evaluation&>* run(int timeES, int timeVND, int timeILS)
	{
		if (timeES == 0)
			timeES = 1;

		pair<Solution<RepEFP>&, Evaluation&>* finalSol;

		double targetValue = 3.879748973;
		targetValue = 0;
		//finalSol = EsCOpt->search(timeES);
		finalSol = es->search(timeES, targetValue);

//		vnd->setMessageLevel(3);
//		if (timeVND > 0)
//			vnd->exec(finalSol->first, finalSol->second, timeVND, 0);
//
//		const Solution<RepEFP> solVND = finalSol->first;
//		const Evaluation evaluationVND = finalSol->second;
//		evaluationVND.print();
//		if (timeILS > 0)
//			finalSol = ils->search(timeILS, 0, &solVND, &evaluationVND);

		return finalSol;
	}

	pair<Solution<RepEFP>&, Evaluation&>* runGILS(int timeGRASP, int timeILS)
	{
		BasicGRASP<RepEFP> g(*eval, *c, emptyLS, 0.1, 100000);
		g.setMessageLevel(3);
		pair<Solution<RepEFP>&, Evaluation&>* finalSol;

		finalSol = g.search(timeGRASP);
		const Solution<RepEFP> solGRASP = finalSol->first;
		const Evaluation evaluationGrasp = finalSol->second;

		ils->setMessageLevel(3);
		if (timeILS > 0)
			finalSol = ils->search(timeILS, 0, &solGRASP, &evaluationGrasp);

		return finalSol;
	}

	//return blind forecasts for the required steps ahead of problemParam class
	vector<double> returnBlind(pair<SolutionEFP&, Evaluation&>* sol, vector<vector<double> >& vTimeSeries)
	{
		return eval->returnForecasts(sol->first.getR(), vTimeSeries, vTimeSeries[eval->getTargetFile()].size(), problemParam.getStepsAhead());
	}

	vector<double> returnErrors(pair<SolutionEFP&, Evaluation&>* sol, vector<vector<double> >& vForecastingsValidation)
	{
		return eval->evaluateAll(sol->first.getR(), -1, &vForecastingsValidation);

//		(NMETRICS, 0);
//		vector<double> estimatedValues = returnForecasts(sol, vForecastingsValidation);
//
//		int maxLag = problemParam.getMaxLag();
//		vector<double> targetValues;
//
//		for (int i = maxLag; i < vForecastingsValidation[0].size(); i++)
//			targetValues.push_back(vForecastingsValidation[0][i]);
//
////		cout << validationSetValues << endl;
////		getchar();
//		foIndicatorNew = returnErrorsCallGetAccuracy(targetValues, estimatedValues);
////		cout << "insideForecastClassNew" << endl;
////		cout << foIndicatorNew << endl;

//		return foIndicatorNew;
	}

	//return all possible forecasting measures
	vector<double> callEvalGetAccuracy(vector<double> targetValues, vector<double> estimatedValues)
	{
		return eval->getAccuracy(targetValues, estimatedValues, -1);
	}
//
	vector<double> returnForecasts(pair<SolutionEFP&, Evaluation&>* sol, vector<vector<double> > vForecastingsValidation)
	{
		pair<vector<double>, vector<double> > targetAndForecasts = eval->generateSWMultiRoundForecasts(sol->first.getR(), vForecastingsValidation, problemParam.getStepsAhead());
		return targetAndForecasts.second;
	}

	vector<double> returnErrorsPersistance(vector<double> targetValues, int fH)
	{

		vector<double> estimatedValues;
		int nSamples = targetValues.size();
		for (int s = 1; s < nSamples; s += fH)
			for (int k = 0; k < fH; k++)
			{
				if (s + k < nSamples)
					estimatedValues.push_back(targetValues[s - 1]);
			}

//		cout<<targetValues<<endl;
		targetValues.erase(targetValues.begin());
//		cout << estimatedValues << endl;
//		cout << targetValues << endl;
//		getchar();

		return eval->getAccuracy(targetValues, estimatedValues, -1);
	}

};

} /* namespace EFP */
#endif /* FORECASTCLASS_HPP_ */
