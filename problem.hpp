#ifndef _PROBLEM_HPP_
#define _PROBLEM_HPP_

#include <QtCore/QObject>

#include <QtGui/QColor>

#include <chrono>
#include <cstdint>

class QKeyEvent;
class QPaintEvent;
class QWidget;

enum class AnswerResult : uint8_t {
   INCORRECT,
   CORRECT
};

class Problem :
   public QObject
{
   Q_OBJECT;

public:
   virtual ~Problem( ) noexcept;

   void SetTextColor(
      const QColor & color ) noexcept;
   const QColor & GetTextColor( ) const noexcept;

   bool SetStartTime(
      const std::chrono::steady_clock::time_point start_time ) noexcept;
   bool SetEndTime(
      const std::chrono::steady_clock::time_point end_time ) noexcept;
   std::chrono::steady_clock::duration GetResponseTime( ) const noexcept;

   virtual void OnPaintEvent(
      QPaintEvent * paint_event,
      QWidget & widget ) noexcept = 0;
   virtual void OnKeyReleaseEvent(
      QKeyEvent * key_event,
      const QWidget & widget ) noexcept = 0;

signals:
   void Answered(
      const AnswerResult response ) const;

private:
   QColor text_color_;

   std::chrono::steady_clock::time_point start_time_;
   std::chrono::steady_clock::time_point end_time_;

};

#endif // _PROBLEM_HPP_
