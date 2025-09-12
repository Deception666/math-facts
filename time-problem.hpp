#ifndef _TIME_PROBLEM_HPP_
#define _TIME_PROBLEM_HPP_

#include "problem.hpp"

class QWidget;

class TimeProblem :
   public Problem
{
public:
   TimeProblem( ) noexcept;
   virtual ~TimeProblem( ) noexcept;

   virtual QVector< QString > GetResponses( ) const noexcept override;
   virtual QString GetQuestionWithAnswer( ) const noexcept override;
   virtual size_t GetNumberOfResponses( ) const noexcept override;

   virtual void OnPaintEvent(
      QPaintEvent * paint_event,
      QWidget & widget ) noexcept override;
   virtual void OnKeyReleaseEvent(
      QKeyEvent * key_event,
      const QWidget & widget ) noexcept override;

private:
   void PaintClock(
      QWidget & widget ) noexcept;
   void PaintResponse(
      QWidget & widget ) noexcept;

   void GradeAnswer( ) noexcept;

};

#endif // _TIME_PROBLEM_HPP_
