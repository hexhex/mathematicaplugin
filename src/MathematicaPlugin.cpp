
//
// this include is necessary
//
#include "dlvhex/PluginInterface.h"
#include "dlvhex/Error.h"

#include "dlvhex/AtomSet.h"
#include "UniqueLinSolve.hpp"
      
namespace dlvhex {
    
    //
    // A plugin must derive from PluginInterface
    //
    class MathematicaPlugin : public PluginInterface
    {
    public:
      
      //
      // register all atoms of this plugin:
      //
      virtual void
      getAtoms(AtomFunctionMap& a)
      {
	boost::shared_ptr<PluginAtom> linearSolve(new LinearSolveAtom);
	boost::shared_ptr<PluginAtom> uniqueLinSolve(new UniqueLinSolveAtom);
	
	a["linearSolve"] = linearSolve;
        a["uniqueLinSolve"] = uniqueLinSolve;


      }
      
      virtual void
      setOptions(bool doHelp, std::vector<std::string>& argv, std::ostream& out)
      {
      }
      
    };
    
    
    //
    // now instantiate the plugin
    //
    MathematicaPlugin theMathematicaPlugin;
    
} // namespace dlvhex

//
// and let it be loaded by dlvhex!
//
extern "C"
dlvhex::MathematicaPlugin*
PLUGINIMPORTFUNCTION()
{
  dlvhex::theMathematicaPlugin.setPluginName(PACKAGE_TARNAME);
  dlvhex::theMathematicaPlugin.setVersion(MATHEMATICAPLUGIN_MAJOR,
					     MATHEMATICAPLUGIN_MINOR,
					     MATHEMATICAPLUGIN_MICRO);
  return new dlvhex::MathematicaPlugin();
}


