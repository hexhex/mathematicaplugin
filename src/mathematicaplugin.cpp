
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
#include "mathlinksingleton.hpp"

namespace dlvhex {
        
    class LinearSolveAtom : public PluginAtom
    {
    public:
      
      LinearSolveAtom()
      {
        // this Atom calculates a solutionvector x which solves the matrix
	// equation Ax==b
	//
        // first parameter: the predicate of the matrix A
        //
        addInputPredicate();
	
	//
	// second parameter: the predicate of vector b
	//
	addInputPredicate();
	setOutputArity(1);
        
      }

      virtual void

      retrieve(const Query& query, Answer& answer) throw (PluginError)
      {
	Tuple parms = query.getInputTuple();
	
	std::string matrixPred = "";
	std::string vectorPred = "";
	int argc = 2;
	char* argv[] = {"-linkname", "math -mathlink"};

	if (parms.size()!= 2)
	  {
	    throw PluginError("Wrong number of arguments");
	  }
	else
	  {
	    if(parms[0].isSymbol() && parms[1].isSymbol()) 
	      {
		matrixPred = parms[0].getString();
		//std::cout << "Matrixpraedikat: " << matrixPred << std::endl;
		vectorPred = parms[1].getString();
		//std::cout << "Vektorpraedikat: " << vectorPred << std::endl;
	      }
	    else
	      {
		throw PluginError("Wrong type of arguments");
	      }
	  }
	//get complete Interpretation of given predicates in query
	AtomSet totalInt = query.getInterpretation();
	AtomSet matrixInt;
	AtomSet vectorInt;
      
	if (totalInt.empty()) 
	  {
	    throw PluginError("Could not find any interpretion");
	  }
	else 
	  {
	    // separate interpretation into facts of first predicate (matrix)
	    totalInt.matchPredicate(matrixPred, matrixInt);
	    // and into facts of second predicate (vector)
	    totalInt.matchPredicate(vectorPred, vectorInt);
	  }
	
	int mLines = 0;
	int mColumns = 0;
	int vLines = 0;
	evaluateMatrix(matrixInt, mLines, mColumns);
	evaluateVector(vectorInt, vLines);
	//std::cout << "Lines nach evaluateMatrix: " << mLines << std::endl;
	//std::cout << "Columns nach evaluateMatrix: " << mColumns << std::endl;
	//std::cout << "Vectorlines nach evaluateVector: " << vLines << std::endl;
	
	std::vector <std::vector <std::string> > matrix(mLines);
	for(int i = 0; i < mLines; i++)
	  matrix[i].resize(mColumns);
	
	std::vector <std::string> vektor(vLines);

	convertMatrixToVector(matrixInt, mLines, mColumns, matrix);

	convertVectorToVector(vectorInt, vLines, vektor);
	 
	std::string expr = toString(matrix, mLines, mColumns, vektor, vLines);

	Tuple out = calculateSolution(argc, argv, expr);
	     
        answer.addTuple(out);
      }      



      //check number and datatypes of every atom's arguments, count lines and columns of matrix
      void evaluateMatrix(const AtomSet& set, int& lines, int& columns) throw (PluginError)
      {
	lines = 0;
	columns = 0;
	AtomSet::const_iterator it = set.begin();
	
	for (int i = 0; i < set.size(); i++)
	  {
	    if (it->getArity()!= 3) //check all Atoms in AtomSet for the right Arity
	      {
		throw PluginError("Wrong number of arguments in matrix atom");
	      }
	    Term line = it->getArgument(1);
	    Term column = it->getArgument(2);
	    Term value = it->getArgument(3);
	    //check for all Atoms if the lines and columns are coded as integers and the value as string
	    if (!line.isInt() || !column.isInt() || !value.isString())
	      {
		throw PluginError("Line and column numbers have to be integers, value has to be a quoted string");
	      }

	    //calculate the necessary size of the 2-dimensional 
	    //string-array matrix[lines][columns]
	    if (line.getInt() > lines)
	      {
		lines = line.getInt();
	      }
	    if (column.getInt() > columns)
	      {
		columns = column.getInt();
	      }
	    it++;
	  }
	//std::cout << "Anzahl der berechneten Zeilen der Matrix: " << lines << std::endl;
	//std::cout << "Anzahl der berechneten Spalten der Matrix: " << columns << std::endl;
      }

      //check number and datatypes of every atom's arguments, 
      //count lines of vector
       void evaluateVector(const AtomSet& set, int& lines) throw (PluginError)
      {
	lines = 0;
	AtomSet::const_iterator it = set.begin();

	for (int i = 0; i < set.size(); i++)
	  {
	    if (it->getArity()!= 2) //check all Atoms in AtomSet for the right Arity
	      {
		throw PluginError("Wrong number of arguments in vector atom");
	      }
	    Term line = it->getArgument(1);
	    Term value = it->getArgument(2);
	    //check for all Atoms if the lines and columsn are coded as integers and the value as string
	    if (!line.isInt() || !value.isString())
	      {
		throw PluginError("Line numbers have to be integers, values have to be quoted strings");
	      }

	    //calculate the necessary size of the 1-dimensional string-array vector[lines]
	    if (line.getInt() > lines)
	      {
		lines = line.getInt();
	      }
	    it++;
	  }
	//std::cout << "Anzahl der berechneten Zeilen des Vektors: " << lines << std::endl;
      }

      //iterate the given AtomSet and sort the values of all Atoms into the 2-dim vector matrix, size of matrix is given by lines and columns
      void convertMatrixToVector(const AtomSet& set, const int& lines, const int& columns, std::vector <std::vector<std::string> >& matrix)
      {
	//initialize the string array 
	for (int i = 0; i < lines; i++)
	  {
	    for (int j = 0; j < columns; j++) 
	      {
		matrix[i][j]= "";
	      }
	  }
	//write Atoms values into 2-dim vector matrix
        AtomSet::const_iterator it = set.begin();
	for (int i = 0; i < set.size(); i++) {
	  matrix[it->getArgument(1).getInt()-1][it->getArgument(2).getInt()-1] 
	    = it->getArgument(3).getUnquotedString();
	  it++;
	}
      }

      //iterate the Atomset and write all values into 1-dim vector vektor
       void convertVectorToVector(const AtomSet& set, const int& lines, std::vector<std::string>& vektor)
      {
	//initialize the string vector 
	for (int i = 0; i < lines; i++)
	  {
	    vektor[i] = "";
	  }

	//write Atom values into 1-dim vector vektor
        AtomSet::const_iterator it = set.begin();
	for (int i = 0; i < set.size(); i++) {
	  vektor[it->getArgument(1).getInt()-1] 
	    = it->getArgument(2).getUnquotedString();
	  it++;
	}
      }

      //create a LinearSolve mathematica expression from matrix and vector
      //return value: expression as string, can be sent to mathematica
      std::string toString(const std::vector<std::vector<std::string> >& matrix,
			   const int& mLines, const int& mColumns,
			   const std::vector<std::string>& vektor,
			   const int& vLines)
	{
	  std::string expression = "LinearSolve[{";
	  for (int i = 0; i<mLines; i++)
	    {
	      expression += "{";
	      for(int j = 0; j<mColumns-1; j++)
		{
		  expression += matrix[i][j];
		  expression += ",";
		}
	      expression += matrix[i][mColumns-1];
	      if (i < mLines-1)
		expression += "},";
	      else expression += "}";
	    }
	  expression += "},{";
	  for (int i=0; i<vLines-1; i++)
	    {
	      expression += vektor[i];
	      expression += ",";
	    }
	  expression += vektor[vLines-1];
	  expression += "}]";
	  std::cout << "Expression: " << expression << std::endl;
	  return expression;
	}


      Tuple calculateSolution(int argc, char* argv[], std::string expression) throw (PluginError)
      {
	int packet;
	Tuple out;

	 //create link to mathematica kernel
	 MathLinkSingleton::instance()->init_and_openlink(argc, argv);

	 //calculate the solution vector 
	 MLPutFunction (MathLinkSingleton::instance()->lp, "EvaluatePacket", 1);
	 MLPutFunction (MathLinkSingleton::instance()->lp, "ToExpression", 1);
	 MLPutString (MathLinkSingleton::instance()->lp, expression.c_str());
	 MLEndPacket (MathLinkSingleton::instance()->lp);

	 if (!MLFlush(MathLinkSingleton::instance()->lp))
	     throw PluginError("Error while flushing link to mathematica");

	 while ( (packet = MLNextPacket(MathLinkSingleton::instance()->lp), packet) 
	  && packet != RETURNPKT)
	   {
	   MLNewPacket(MathLinkSingleton::instance()->lp);
	   }
      
	 int errcount=0;
	 std::string solution = "{";
	 std::string temp = "";

	 //process result from mathematica, as long as data can be found on the link
	 while (MLReady(MathLinkSingleton::instance()->lp))
	{
	  // check type of the next part of the result-expression
	  switch(MLGetNext(MathLinkSingleton::instance()->lp)) 
	    {
	      //next part of expression: integer
	    case MLTKINT:
	      int data;
	      if(!MLGetInteger32(MathLinkSingleton::instance()->lp, &data))
		 throw PluginError("Error while reading integer from mathlink");
	      //std::cout << "Integer von MathLink gelesen: " << data << std::endl;
	      solution += intToString(data);
	      solution += ",";
	      break;

	      //next part of expression: double
	    case MLTKREAL:
	      double real;
		 if(!MLGetReal64(MathLinkSingleton::instance()->lp, &real))
		   throw PluginError("Error while reading double from mathlink");
		 //std::cout << "Double von MathLink gelesen: " << real << std::endl;
	      solution += doubleToString(real);
	      solution += ",";
	     
	      break;
	      
	      //next part of expression: string
	    case MLTKSTR:
	      const char *str;
		 if(!MLGetString(MathLinkSingleton::instance()->lp, &str))
		   throw PluginError("Error while reading string from mathlink");
		 //std::cout << "String von MathLink gelesen: " << str << std::endl;
	      temp = str;
	      solution += temp;
	      solution += ",";
	      MLReleaseString(MathLinkSingleton::instance()->lp, str);
	      break;

	      //next part of expression: symbol
	    case MLTKSYM:
	      const char *symbol;
		 if(!MLGetSymbol(MathLinkSingleton::instance()->lp, &symbol))
		   throw PluginError("Error while reading symbol from mathlink");
		 //std::cout << "Symbol von MathLink gelesen: " << symbol << std::endl;
	      temp = symbol;
	      solution += temp;
	      solution += ",";
	      MLReleaseSymbol(MathLinkSingleton::instance()->lp, symbol);   
	      break;

	      //next part of expression: function
	    case MLTKFUNC:
	      const char *func;
	      int n;
	      if(!MLGetFunction(MathLinkSingleton::instance()->lp, &func, &n))
		   throw PluginError("Error while reading function from mathlink");
	      //std::cout << "Funktion " << func << " mit " << n << " Argumenten von MathLink gelesen: " << std::endl;
		    temp = func;
		    solution += temp;
		    solution += ":";
		       MLReleaseSymbol(MathLinkSingleton::instance()->lp, func);   
	      break;

	    case MLTKERR:
	      errcount = 1;
	      break;

	    default:
	      errcount = 1;
	      break;
	    }
	  if(errcount) break;
	  }

	 //exit Mathematica and close link
	 MLPutFunction (MathLinkSingleton::instance()->lp, "Exit", 0);
	 MathLinkSingleton::instance()->closeLink();
	 MathLinkSingleton::instance()->deinit(); 
	 
	 solution = solution.erase(solution.size()-1, 1);
	 solution += "}";
	 
	 //put solutionstring on output-vector
	 out.push_back(Term(solution, true));

	 if (errcount)
	   throw PluginError("Error occured while reading from mathlink");
	 else
	   return out;

	}

      std::string doubleToString(double real)
      {
	std::stringstream ss;
	ss << real;
	return ss.str();
      }
      
      std::string intToString(int number)
      {
	std::stringstream ss;
	ss << number;
	return ss.str();
      }
      
    };
      
    
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
	
        a["linearSolve"] = linearSolve;

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


