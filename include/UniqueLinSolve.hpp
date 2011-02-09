
//
// this include is necessary
//
#include "dlvhex/PluginInterface.h"
#include "dlvhex/Error.h"

#include "dlvhex/AtomSet.h"

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "LinearSolve.hpp"

namespace dlvhex {
        
    class UniqueLinSolveAtom : public LinearSolveAtom
    {
    public:
      
      virtual void retrieve(const Query& query, 
			    Answer& answer) throw (PluginError);
    };
}
   
