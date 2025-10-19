#ifndef _TIME_PROBLEM_HPP_
#define _TIME_PROBLEM_HPP_

#include "problem.hpp"

#include <QtCore/QString>
#include <QtCore/QVector>

#include <cstdint>
#include <variant>
#include <utility>

class QPixmap;
class QSize;
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
      std::pair< QString, QSize > Question( ) const noexcept;

      uint8_t Hour( ) const noexcept { return hour_; }
      uint8_t Minute( ) const noexcept { return minute_; }

   private:
      uint8_t hour_;
      uint8_t minute_;

   };

   class MilitaryTime
   {
   public:
      MilitaryTime(
         const uint8_t hour,
         const uint8_t minute,
         const bool is_afternoon ) noexcept;

      bool GradeResponse(
         const QString & response ) const noexcept;

      QString Answer( ) const noexcept;
      std::pair< QString, QSize > Question( ) const noexcept;

      uint8_t Hour( ) const noexcept { return hour_; }
      uint8_t Minute( ) const noexcept { return minute_; }
      bool IsAfternoon( ) const noexcept { return is_afternoon_; }

   private:
      uint8_t hour_;
      uint8_t minute_;
      bool is_afternoon_;

   };

   TimeProblem(
      Time time ) noexcept;
   TimeProblem(
      MilitaryTime time ) noexcept;
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
   void PaintTimeProblem(
      QWidget & widget ) noexcept;
   void PaintMilitaryTimeProblem(
      QWidget & widget ) noexcept;
   void PaintQuestionAndResponse(
      QWidget & widget ) noexcept;

   QPixmap RenderClock( ) const noexcept;
   QPixmap RenderSunScene( ) const noexcept;

   void GradeAnswer( ) noexcept;

   QString response_;

   QVector< QString > responses_;

   std::variant< Time, MilitaryTime > problem_;

};

#endif // _TIME_PROBLEM_HPP_
