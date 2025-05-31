#ifndef _ARITHMETIC_PROBLEM_HPP_
#define _ARITHMETIC_PROBLEM_HPP_

#include "problem.hpp"

#include <QtCore/QString>

#include <cstdint>
#include <vector>

class QWidget;

class ArithmeticProblem :
   public Problem
{
public:
   enum class Operation : uint8_t
   {
      ADD, SUB, MUL, DIV
   };

   ArithmeticProblem(
      const int32_t top,
      const int32_t bottom,
      const Operation operation ) noexcept;
   virtual ~ArithmeticProblem( ) noexcept;

   virtual void OnPaintEvent(
      QPaintEvent * paint_event,
      QWidget & widget ) noexcept override;
   virtual void OnKeyReleaseEvent(
      QKeyEvent * key_event,
      const QWidget & widget ) noexcept override;

private:
   void GradeAnswer( ) noexcept;

   QString top_;
   QString bottom_;
   QString response_;
   Operation operation_;
   
   int32_t answer_;

   std::vector< int32_t > responses_;

};

#endif // _ARITHMETIC_PROBLEM_HPP_
