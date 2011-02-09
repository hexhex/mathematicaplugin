
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
#include "UniqueLinSolve.hpp"

namespace dlvhex {
        
  
      void
      UniqueLinSolveAtom::retrieve(const Query& query, 
			       Answer& answer) throw (PluginError)
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

    
} // namespace dlvhex
