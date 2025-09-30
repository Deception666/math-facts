#ifndef _TIME_PROBLEM_HPP_
#define _TIME_PROBLEM_HPP_

#include "problem.hpp"

#include <QtCore/QString>
#include <QtCore/QVector>

#include <cstdint>
#include <variant>

class QString;
class QWidget;

class TimeProblem :
   public Problem
{
public:
   class Time
   {
   public:
      Time(
         const uint8_t hour,
         const uint8_t minute ) noexcept;

      bool GradeResponse(
         const QString & response ) const noexcept;

      QString Answer( ) const noexcept;

      uint8_t Hour( ) const noexcept { return hour_; }
      uint8_t Minute( ) const noexcept { return minute_; }

   private:
      uint8_t hour_;
      uint8_t minute_;

   };

   TimeProblem(
      Time time ) noexcept;
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
   void PaintQuestionAndResponse(
      QWidget & widget ) noexcept;

   void GradeAnswer( ) noexcept;

   QString response_;

   QVector< QString > responses_;

   std::variant< Time > problem_;

};

#endif // _TIME_PROBLEM_HPP_
