#include <stdio.h>
#include <stdlib.h>
#include "mathlink.h"

class MathLinkSingleton
{
public:
  static MathLinkSingleton* instance ();
  MLENV ep;
  MLINK lp;
  void init_and_openlink(int argc, char* argv[]);
  void deinit();
  void closeLink();

private:
  static MathLinkSingleton* _instance;
  MathLinkSingleton() {};   //private Constructor
  MathLinkSingleton(const MathLinkSingleton&);  //can't make a copy
  ~MathLinkSingleton(){};
  class CGuard
  {
  public:
    ~CGuard();
  };
  friend class CGuard;
 

};
