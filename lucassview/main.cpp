#include <TRint.h>
#include <TROOT.h>
//#include "../MyClient/MyClient.h"
//#include "../MySettings/MySettings.h"


//void initializeSettings(MySettings&);
int main(int argc, char *argv[])
{
	TRint theApp("App", &argc, argv);
	
  //MySettings set(false);
  //initializeSettings(set);
  //MyClient c(set);

	//gROOT->ProcessLine(".L OnlineMacros.c");

	theApp.Run();

	return 0;
}
