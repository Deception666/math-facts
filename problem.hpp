#ifndef _PROBLEM_HPP_
#define _PROBLEM_HPP_

#include <QtCore/QObject>

#include <QtGui/QColor>

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

};

#endif // _PROBLEM_HPP_
