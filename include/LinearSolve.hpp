
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

namespace dlvhex {
        
    class LinearSolveAtom : public PluginAtom
    {
    public:
      
      LinearSolveAtom();
      virtual void retrieve(const Query& query, 
			    Answer& answer) throw (PluginError);

    protected:
      //check number and datatypes of every atom's arguments, count lines and columns of matrix
      void evaluateMatrix(const AtomSet& set, 
			  int& rows, int& columns) throw (PluginError);
      
      //check number and datatypes of every atom's arguments, 
      //count rows of vector
      void evaluateVector(const AtomSet& set, 
			  int& rows, int& columns) throw (PluginError);
      
      //iterate the given AtomSet and sort the values of all Atoms into the 2-dim vector matrix, size of matrix is given by lines and columns
      void convertMatrixToVector(const AtomSet& set, 
				 const int& rows, const int& columns, 
				 std::vector <std::vector<std::string> >& matrix);
      
      //evaluate the vector (filled with values of interpretation), 
      //each possible position in matrix or vector must be coded in interpretation
      void checkVector(std::vector <std::vector<std::string> >& vektor, 
		       const int& rows, const int&columns, 
		       const std::string& predicateName) throw (PluginError);
      
      //create a MatrixRank mathematica expression from coefficient matrix A
      //return value: expression as string, can be sent to mathematica for evaluation
      std::string toMatrixRankExpr(const std::vector<std::vector<std::string> >&matrix,
				   const int& mRows, const int& mColumns);
      
       //create a MatrixRank mathematica expression from extended coefficient matrix [A,b]
      //return value: expression as string, can be sent to mathematica for evaluation
      std::string toMatrixRankExpr(const std::vector<std::vector<std::string> >&matrix,
				   const int& mRows, const int& mColumns,
				   const std::vector<std::vector<std::string> >&constants,
				   const int& cRows, const int& cColumns);

      //create a LinearSolve mathematica expression from matrix and vector
      //return value: expression as string, can be sent to mathematica for evaluation
      std::string toLinearSolveExpr(const std::vector<std::vector<std::string> >& matrix,
			   const int& mRows, const int& mColumns,
				    const std::vector<std::vector<std::string> >& vektor,
				    const int& cRows, const int& cColumns);
	
      //send expression to mathematica and wait for result
      void sendExpression(std::string expression) throw (PluginError);
      
      //send expression to mathematica for evaluation, return rank of matrix as integer
      int calculateRank(int argc, char* argv[], 
			std::string expression) throw (PluginError);
      
      //send expression to mathematica and return values of calculated vector or matrix as strings in std::vector
      std::vector <std::string> calculateSolution(int argc, char* argv[], 
						  std::string expression) throw (PluginError);
      
      std::string doubleToString(double real);
      
      std::string intToString(int number);
      
    };
      
} //namespace dlvhex
