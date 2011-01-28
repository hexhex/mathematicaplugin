
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
        
    class UniqueLinSolveAtom : public PluginAtom
    {
    public:
      
      UniqueLinSolveAtom()
      {
        // this Atom calculates a solutionvector x which solves the matrix
	// equation Ax==b iff there exists an unique solution 
	//
        // first parameter: the predicate of the matrix A
        //
        addInputPredicate();
	
	//
	// second parameter: the predicate of vector b
	//
	addInputPredicate();

	//
	// output: predicates with 3 arguments: #row, #column, value for these indices 
	//
	setOutputArity(3);
        
      }

      virtual void

      retrieve(const Query& query, Answer& answer) throw (PluginError)
      {
	Tuple parms = query.getInputTuple();
	
	std::string matrixPred = "";
	std::string constantPred = "";
	int argc = 2;
	char* argv[] = {"-linkname", "math -mathlink"};

	//check number and type of arguments
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
		constantPred = parms[1].getString();
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
	AtomSet constantInt;
      
	if (totalInt.empty()) 
	  {
	    throw PluginError("Could not find any interpretion");
	  }
	else 
	  {
	    // separate interpretation into facts of first predicate (matrix)
	    totalInt.matchPredicate(matrixPred, matrixInt);
	    // and into facts of second predicate (vector)
	    totalInt.matchPredicate(constantPred, constantInt);
	  }
	
	int mRows = 0;
	int mColumns = 0;
	int cRows = 0;
	int cColumns = 0;
	evaluateMatrix(matrixInt, mRows, mColumns);
	evaluateVector(constantInt, cRows, cColumns);
	
	if(mRows != cRows) throw PluginError("Coefficient matrix and target vector(s) or matrix do not have the same dimensions.");

	std::vector <std::vector <std::string> > matrix(mRows);
	for(int i = 0; i < mRows; i++)
	  matrix[i].resize(mColumns);
	
	std::vector <std::vector <std::string> > constants(cRows);
	for (int i = 0; i < cRows; i++)
	  constants[i].resize(cColumns);

	//write the values of the Atoms in the Interpretation into std::vectors for further processing
	convertMatrixToVector(matrixInt, mRows, mColumns, matrix);
	convertMatrixToVector(constantInt, cRows, cColumns, constants); 

	//check if matrix and target vector or matrix are fully defined
	checkVector(matrix, mRows, mColumns, matrixPred);
	checkVector(constants, cRows, cColumns, constantPred);
	 
	//convert matrix to MatrixRank-expression and calculate rank of coefficient matrix A 
	std::string coeffMRankExpr = toMatrixRankExpr(matrix, mRows, mColumns);
	//std::cout << "MatrixRank expression: " << coeffMRankExpr << std::endl;
	int coeffMRank = calculateRank(argc, argv, coeffMRankExpr);

	//convert matrix A and target b to MatrixRank-expression and calculate rank
	//of extended coefficient matrix [A,b]
	std::string extendedMRankExpr = toMatrixRankExpr(matrix, mRows, mColumns, constants, cRows, cColumns);
	//std::cout << "Extended MatrixRank expression: " << extendedMRankExpr << std::endl;
	int extCoeffMRank = calculateRank(argc, argv, extendedMRankExpr);

	//compare calculated ranks and number of matrix colums, iff they are equal, 
	//a unique solution for the matrix equation exists
	if ((coeffMRank == extCoeffMRank) && (coeffMRank == mColumns))
	  {
	    std::string linSolExpr = toLinearSolveExpr(matrix, mRows, mColumns, constants, cRows, cColumns);
	    std::vector <std::string> result;
	    result = calculateSolution(argc, argv, linSolExpr);
	    if(result.size() != mColumns*cColumns)
	      throw PluginError("Wrong number of arguments in result vector");
	    Tuple out;
	    int index = 0;

	    //fill the result values with correct indices into Tuple out
	    //and add all Tuples to Answer
	    for (int r = 1; r <= mColumns; r++)
	      {
		for(int c = 1; c<= cColumns; c++)
		  {
		    out.push_back(Term(r));
		    out.push_back(Term(c));
		    out.push_back(Term(result[index],true));
		    answer.addTuple(out);
		    out.clear();
		    index++;
		  }
	      }
	  }
      }      



      //check number and datatypes of every atom's arguments, count lines and columns of matrix
      void evaluateMatrix(const AtomSet& set, int& rows, int& columns) throw (PluginError)
      {
	rows = 0;
	columns = 0;
	AtomSet::const_iterator it = set.begin();
	
	for (int i = 0; i < set.size(); i++)
	  {
	    if (it->getArity()!= 3) //check all Atoms in AtomSet for the right Arity
	      {
		throw PluginError("Wrong number of arguments in matrix atom");
	      }
	    Term row = it->getArgument(1);
	    Term column = it->getArgument(2);
	    Term value = it->getArgument(3);
	    //check for all Atoms if the lines and columns are coded as integers and the value as string
	    if (!row.isInt() || !column.isInt() || !value.isString())
	      {
		throw PluginError("Row and column numbers have to be integers, value has to be a quoted string");
	      }

	    //calculate the necessary size of the 2-dimensional 
	    //string-array matrix[lines][columns]
	    if (row.getInt() > rows)
	      {
		rows = row.getInt();
	      }
	    if (column.getInt() > columns)
	      {
		columns = column.getInt();
	      }
	    it++;
	  }
      }

      //check number and datatypes of every atom's arguments, 
      //count rows of vector
      void evaluateVector(const AtomSet& set, int& rows, int& columns) throw (PluginError)
      {
	rows = 0;
	columns = 0;
	AtomSet::const_iterator it = set.begin();
	if(it->getArity() == 2)
	  {
	    for (int i = 0; i < set.size(); i++)
	      {
		if (it->getArity()!= 2) //check all Atoms in AtomSet for the right Arity
		  {
		    throw PluginError("Wrong number of arguments in vector atom");
		  }
		Term row = it->getArgument(1);
		Term value = it->getArgument(2);
		//check for all Atoms if the lines and columns are coded as integers and the value as string
		if (!row.isInt() || !value.isString())
		  {
		    throw PluginError("Line numbers have to be integers, values have to be quoted strings");
		  }

		//calculate the necessary size of the 1-dimensional string-array vector[lines]
		if (row.getInt() > rows)
		  {
		    rows = row.getInt();
		  }
		it++;
	      }
	    columns=1;
	  }
	else if (it->getArity() == 3)
	  {
	    evaluateMatrix(set, rows, columns);
	  }
	else 
	  throw PluginError("Wrong number of Arguments in target vector or matrix");
    
      }

      //iterate the given AtomSet and sort the values of all Atoms into the 2-dim vector matrix, size of matrix is given by lines and columns
      void convertMatrixToVector(const AtomSet& set, const int& rows, const int& columns, std::vector <std::vector<std::string> >& matrix)
      {
	//initialize the string array 
	for (int i = 0; i < rows; i++)
	  {
	    for (int j = 0; j < columns; j++) 
	      {
		matrix[i][j]= "";
	      }
	  }
	
        AtomSet::const_iterator it = set.begin();
	if (it->getArity() == 3) 
	  {
	    for (int i = 0; i < set.size(); i++) {
	      matrix[it->getArgument(1).getInt()-1][it->getArgument(2).getInt()-1] 
		= it->getArgument(3).getUnquotedString();
	      it++;
	    }
	  }
	else 
	  {
	     for (int i = 0; i < set.size(); i++) {
	      matrix[it->getArgument(1).getInt()-1][0] 
		= it->getArgument(2).getUnquotedString();
	      it++;
	    }
	  }
      }


      //evaluate the vector (filled with values of interpretation), 
      //each possible position in matrix or vector must be coded in interpretation
      void checkVector(std::vector <std::vector<std::string> >& vektor, 
		       const int& rows, const int&columns, 
		       const std::string& predicateName) throw (PluginError)
      {
	for (int i = 0; i < rows; i++) 
	  {
	    for (int j = 0; j < columns; j++)
	      {
		if (vektor[i][j]=="") 
		  {
		    std::string errorMessage = "Missing value of matrix or vector " + 
		      predicateName + " at row " + intToString(i+1) +
		      ", column " + intToString(j+1);
		    throw PluginError(errorMessage);
		  }
	      }
	  }
      }



      //create a MatrixRank mathematica expression from coefficient matrix A
      //return value: expression as string, can be sent to mathematica for evaluation
      std::string toMatrixRankExpr(const std::vector<std::vector<std::string> >&matrix,
				   const int& mRows, const int& mColumns)
      {
	std::string expression = "MatrixRank[{";
	 for (int i = 0; i<mRows; i++)
	    {
	      expression += "{";
	      for(int j = 0; j<mColumns-1; j++)
		{
		  expression += matrix[i][j];
		  expression += ",";
		}
	      expression += matrix[i][mColumns-1];
	      if (i < mRows-1)
		expression += "},";
	      else expression += "}";
	    }
	 expression += "}]";
	 return expression;
	
      }

       //create a MatrixRank mathematica expression from extended coefficient matrix [A,b]
      //return value: expression as string, can be sent to mathematica for evaluation
      std::string toMatrixRankExpr(const std::vector<std::vector<std::string> >&matrix,
				   const int& mRows, const int& mColumns,
				   const std::vector<std::vector<std::string> >&constants,
				   const int& cRows, const int& cColumns)
      {
	std::string expression = "MatrixRank[{";
	 for (int i = 0; i<mRows; i++)
	    {
	      expression += "{";
	      for(int j = 0; j<mColumns; j++)
		{
		  expression += matrix[i][j];
		  expression += ",";
		}
	      for(int r = 0; r<cColumns-1;r++) 
		{
		  expression += constants[i][r];
		  expression += ",";
		}
	      expression += constants[i][cColumns-1];
	      if (i < mRows-1)
		expression += "},";
	      else 
		expression += "}";
	    }
	 expression += "}]";
	 return expression;
	
      }


      //create a LinearSolve mathematica expression from matrix and vector
      //return value: expression as string, can be sent to mathematica for evaluation
      std::string toLinearSolveExpr(const std::vector<std::vector<std::string> >& matrix,
			   const int& mRows, const int& mColumns,
				    const std::vector<std::vector<std::string> >& vektor,
				    const int& cRows, const int& cColumns)
	{
	  std::string expression = "LinearSolve[{";
	  for (int i = 0; i<mRows; i++)
	    {
	      expression += "{";
	      for(int j = 0; j<mColumns-1; j++)
		{
		  expression += matrix[i][j];
		  expression += ",";
		}
	      expression += matrix[i][mColumns-1];
	      if (i < mRows-1)
		expression += "},";
	      else expression += "}";
	    }
	  expression += "},";
	  if (cColumns == 1)
	    {
	      expression += "{";
	      for (int i=0; i<cRows-1; i++)
		{
		  expression += vektor[i][0];
		  expression += ",";
	    }
	      expression += vektor[cRows-1][0];
	      expression += "}]";
	    }
	  else 
	    {
	      expression += "{";
	       for (int i = 0; i<cRows; i++)
		 {
		   expression += "{";
		   for(int j = 0; j<cColumns-1; j++)
		     {
		       expression += vektor[i][j];
		       expression += ",";
		     }
		   expression += vektor[i][cColumns-1];
		   if (i < cRows-1)
		     expression += "},";
		   else expression += "}";
		 }
	       expression += "}]";
	    }
	  
	  //std::cout << "Expression: " << expression << std::endl;
	  return expression;
	}


      void sendExpression(std::string expression) throw (PluginError)
      {
	int packet;
	//send expression to mathematica and wait for result
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
     	
      }

      int calculateRank(int argc, char* argv[], std::string expression) throw (PluginError)
      {
	int result;
	int errCount = 0;

	//create link to mathematica kernel
	MathLinkSingleton::instance()->init_and_openlink(argc, argv);
	
	sendExpression(expression);

	while(MLReady(MathLinkSingleton::instance()->lp))
	  {
	     // check type of the next part of the result-expression
	    switch(MLGetNext(MathLinkSingleton::instance()->lp)) 
	    {
	    //next part of expression should be an integer
	    case MLTKINT:
	      if(!MLGetInteger32(MathLinkSingleton::instance()->lp, &result))
		 throw PluginError("Error while reading integer from mathlink");
	      // std::cout << "Integer (Matrix Rank) von MathLink gelesen: " << result << std::endl;
	      break;
	    //everythink except an integer is an error
	    default:
	      errCount = 1;
	      break;
	    }
	     if(errCount) break;
	  }

	 //exit Mathematica and close link
	 MLPutFunction (MathLinkSingleton::instance()->lp, "Exit", 0);
	 MathLinkSingleton::instance()->closeLink();
	 MathLinkSingleton::instance()->deinit(); 

	 if (errCount)
	   throw PluginError("Error: could not calculate matrix rank");
	 else
	   return result;
      }

      std::vector <std::string> calculateSolution(int argc, char* argv[], std::string expression) throw (PluginError)
      {
	std::vector <std::string> result;

	 //create link to mathematica kernel
	 MathLinkSingleton::instance()->init_and_openlink(argc, argv);
	 
	 //send expression to mathematica and wait for result
	 sendExpression(expression);
      
	 int errcount=0;
	 int index = 0;

	 //boolean pNotation: set true, if mathematica sends answer in polish notation
	 bool pNotation = false;
	 //boolean first: set false, if the first functionname in polish notation was recognized
	 bool first = true;
	 //int pnStack: count all functions and their number of arguments, subtract founded arguments
	 int pnStack = 0;
	 std::string list = "List";
	 

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
	      if (pNotation)
		{
		  result[index] += intToString(data);
		  pnStack += -1;
		  if (pnStack == 0)
		    {
		      index++;
		      pNotation = false;
		      first = true;
		    }
		  else
		    result[index] += " ";
		}
	      else
		{
		  result.push_back(intToString(data));
		  index++;
		}
	      break;

	      //next part of expression: double
	    case MLTKREAL:
	      double real;
		 if(!MLGetReal64(MathLinkSingleton::instance()->lp, &real))
		   throw PluginError("Error while reading double from mathlink");
		 //std::cout << "Double von MathLink gelesen: " << real << std::endl;
	       if (pNotation)
		{
		  result[index] += doubleToString(real);
		  pnStack += -1;
		  if (pnStack == 0)
		    {
		      index++;
		      pNotation = false;
		      first = true;
		    }
		  else
		    result[index] += " ";
		}
	       else
		 {
		   result.push_back(doubleToString(real));
		   index++;
		 }
	      break;
	      
	      //next part of expression: string
	    case MLTKSTR:
	      const char *str;
		 if(!MLGetString(MathLinkSingleton::instance()->lp, &str))
		   throw PluginError("Error while reading string from mathlink");
		 std::cout << "String von MathLink gelesen: " << str << std::endl;
	      MLReleaseString(MathLinkSingleton::instance()->lp, str);
	      //a string should not occur in a valid result, Error
	      errcount = 1;
	      break;

	      //next part of expression: symbol
	    case MLTKSYM:
	      const char *symbol;
		 if(!MLGetSymbol(MathLinkSingleton::instance()->lp, &symbol))
		   throw PluginError("Error while reading symbol from mathlink");
		 //std::cout << "Symbol von MathLink gelesen: " << symbol << std::endl;
	       if (pNotation)
		{
		  result[index] += std::string(symbol);
		  pnStack += -1;
		  if (pnStack == 0)
		    {
		      index++;
		      pNotation = false;
		      first = true;
		    }
		  else
		    result[index] += " ";
		}
	      else
		result.push_back(std::string(symbol));
	      MLReleaseSymbol(MathLinkSingleton::instance()->lp, symbol);   
	      break;

	      //next part of expression: function
	    case MLTKFUNC:
	      const char *func;
	      int n;
	      if(!MLGetFunction(MathLinkSingleton::instance()->lp, &func, &n))
		   throw PluginError("Error while reading function from mathlink");
	      //std::cout << "Funktion " << func << " mit " << n << " Argumenten von MathLink gelesen: " << std::endl;
	      //this is necessary to handle results in polish notation for symbolic matrices
	      if(list.compare(func))
		{
		  if(first)
		    {
		      result.push_back(std::string(func) + " ");
		      pNotation = true;
		      pnStack = n;
		      first = false;
		    }
		  else
		    {
		      result[index] += std::string(func);
		      result[index] += " ";
		      pnStack += n-1;
		    }
		}
	      
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
	 

	 if (errcount)
	   throw PluginError("Error occured while reading from mathlink");
	 else
	   return result;

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
	boost::shared_ptr<PluginAtom> uniqueLinSolve(new UniqueLinSolveAtom);
	
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


