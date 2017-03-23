// ===================================
// Main.cpp file generated by OptFrame
// Project $name
// ===================================

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include "../OptFrame/Interactive.hpp"
#include "$project.h"

using namespace std;
using namespace optframe;
using namespace $project;

int main(int argc, char **argv)
{
   Interactive<Rep$project, MY_ADS $commadproject> optframe;
   //optframe.loadCallCommand();

   optframe.loadCommand(new $projectProblemCommand);

   optframe.execute();

   cout << "Program ended successfully" << endl;
   
   return 0;
}