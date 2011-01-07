#include <iostream>
#include "mathlink.h"
#include "mathlinksingleton.hpp"


MathLinkSingleton* MathLinkSingleton::_instance = 0;

MathLinkSingleton* MathLinkSingleton::instance ()
  {
    static CGuard g;    //cleanup memory
    if (!_instance)
      _instance = new MathLinkSingleton();
    return _instance;  
  }

void MathLinkSingleton::deinit()
  {
    if(MathLinkSingleton::instance()->ep) MLDeinitialize(MathLinkSingleton::instance()->ep);
    //std::cout << "Deinit Link" << std::endl;
  }

void MathLinkSingleton::closeLink()
  {
    if (MathLinkSingleton::instance()->lp) MLClose(MathLinkSingleton::instance()->lp);
    //std::cout << "Close Link" << std::endl;
  }

void MathLinkSingleton::init_and_openlink(int argc, char* argv[])
  {
    ep=(MLENV)0;
    lp=(MLINK)0;

#if MLINTERFACE >= 3
    int err;
#else
    long err;
#endif /*MLINTERFACE >= 3*/
    ep = MLInitialize ((MLParametersPointer)0);
    if (ep == (MLENV)0) exit(1);
    // atexit(deinit);

#if MLINTERFACE < 3
    lp = MLOpenArgv(ep, argv, argv+argc, &err);
#else
    lp = MLOpenArgcArgv(ep, argc, argv, &err);
#endif
    if(lp == (MLINK)0)exit(2);
    //atexit(closeLink);

  }

MathLinkSingleton::CGuard::~CGuard()
    {
      if( NULL != MathLinkSingleton::_instance)
	{
	  //MathLinkSingleton::_instance->closeLink();
	  //MathLinkSingleton::_instance->deinit();
	  delete MathLinkSingleton::_instance;
	  MathLinkSingleton::_instance = NULL;
	}
    }
  

