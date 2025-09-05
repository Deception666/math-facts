#include "arithmetic-problem.hpp"

#include <QtCore/QPointF>
#include <QtCore/QRect>
#include <QtCore/QRectF>
#include <QtCore/Qt>
#include <QtCore/QtTypes>
#include <QtCore/QVector>
#include <QtGui/QBrush>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QPixmap>
#include <QtWidgets/QWidget>

#include <cassert>

static int32_t GenerateAnswer(
   const int32_t top,
   const int32_t bottom,
   const ArithmeticProblem::Operation operation ) noexcept
{
   int32_t answer { };

   switch (operation)
   {
   case ArithmeticProblem::Operation::ADD:
      answer = top + bottom;
      break;

   case ArithmeticProblem::Operation::SUB:
      answer = top - bottom;
      break;

   case ArithmeticProblem::Operation::MUL:
      answer = top * bottom;
      break;

   case ArithmeticProblem::Operation::DIV:
      assert(bottom != 0);
      assert(top % bottom == 0);
      answer = top / bottom;
      break;

   default:
      assert(false);
      break;
   }

   return
      answer;
}

static QString ArithmeticSymbol(
   const ArithmeticProblem::Operation operation ) noexcept
{
   QString symbol;

   switch (operation)
   {
   case ArithmeticProblem::Operation::ADD:
      symbol = "+";
      break;

   case ArithmeticProblem::Operation::SUB:
      symbol = "-";
      break;

   case ArithmeticProblem::Operation::MUL:
      symbol = "*";
      break;

   case ArithmeticProblem::Operation::DIV:
      symbol = "/";
      break;

   default:
      assert(false);
      break;
   }

   return
      symbol;
}

ArithmeticProblem::ArithmeticProblem(
   const int32_t top,
   const int32_t bottom,
   const Operation operation ) noexcept :
top_ { QString::number(top) },
bottom_ { QString::number(bottom) },
answer_ { GenerateAnswer(top, bottom, operation) },
operation_ { operation }
{
}

ArithmeticProblem::~ArithmeticProblem( ) noexcept
{
}

QVector< QString > ArithmeticProblem::GetResponses( ) const noexcept
{
   QVector< QString > responses;

   for (const auto response : responses_)
   {
      responses.append(
         QString::number(response));
   }

   return
      responses;
}

QString ArithmeticProblem::GetQuestionWithAnswer( ) const noexcept
{
   return
      top_ +
      " " +
      ArithmeticSymbol(operation_) +
      " " +
      bottom_ +
      " = " +
      QString::number(answer_);
}

size_t ArithmeticProblem::GetNumberOfResponses( ) const noexcept
{
   return
      responses_.size();
}

void ArithmeticProblem::OnPaintEvent(
   QPaintEvent * paint_event,
   QWidget & widget ) noexcept
{
   // pixmap is at pixel ratio of 1.0
   QPixmap text_pixmap {
      2048, 2048
   };

   text_pixmap.fill(
      QColor { 0, 0, 0, 0 });

   QPainter text_painter {
      &text_pixmap
   };

   text_painter.setRenderHint(
      QPainter::RenderHint::Antialiasing,
      true);
   text_painter.setRenderHint(
      QPainter::RenderHint::TextAntialiasing,
      true);

   const QFont font {
   #if _WIN32
      "Comic Sans MS",
   #else
      "Noto Sans Mono",
   #endif
      200
   };

   text_painter.setFont(
      font);

   // set the line thickness
   // set the color of line and font
   text_painter.setPen(
      QPen {
         QBrush { GetTextColor() },
         10.0
      });

   const QFontMetrics font_metrics {
      text_painter.font()
   };

   QString bottom;

   switch (operation_)
   {
   case Operation::ADD: bottom += "+  "; break;
   case Operation::SUB: bottom += "-  "; break;
   case Operation::MUL: bottom += "x  "; break;
   case Operation::DIV: bottom += "\u00F7  "; break;
   }

   bottom += bottom_;

   // problem two is always the longest line
   const QRect text_bounding_rect =
      font_metrics.tightBoundingRect(
         bottom);

   // add a bit of space between the lines
   const int32_t line_space_gap { 10 };
   const int32_t line_height =
      text_bounding_rect.height() +
      line_space_gap;

   text_painter.drawText(
      QRect { 0, 0, text_pixmap.width(), text_pixmap.height() },
      top_,
      QTextOption { Qt::AlignmentFlag::AlignRight });

   QRect line2_bounding_box;
   
   text_painter.drawText(
      QRect { 0, line_height, text_pixmap.width(), text_pixmap.height() },
      Qt::AlignmentFlag::AlignRight,
      bottom,
      &line2_bounding_box);

   text_painter.drawText(
      QRect { 0, line_height * 2 + 40, text_pixmap.width(), text_pixmap.height() },
      response_,
      QTextOption { Qt::AlignmentFlag::AlignRight });

   const int32_t answer_line_width =
      text_bounding_rect.width();
   const qreal answer_line_y =
      line_height * 2 + 40 +
      (font_metrics.ascent() - text_bounding_rect.height()) / 1.25;

   // draw the answer line
   text_painter.drawLine(
      QPointF {
         static_cast< qreal >(text_pixmap.width()),
         answer_line_y },
      QPointF {
         static_cast< qreal >(text_pixmap.width() - answer_line_width),
         answer_line_y });

   // aspect of the inner background box
   const qreal background_box_width { widget.width() - 60.0 };
   const qreal background_box_height { widget.height() - 60.0 };
   
   const qreal background_box_aspect {
      background_box_width / background_box_height
   };

   const qreal problem_width =
      line2_bounding_box.width();
   const qreal problem_height =
      (line_height * 2 + 40 + font_metrics.height());
   const qreal problem_crop_x_start =
      text_pixmap.width() - problem_width;

   const qreal problem_box_aspect =
      problem_width / problem_height;

   qreal scaled_width { };
   qreal scaled_height { };

   if (background_box_aspect >= 1.0)
   {
      // the background box is wider than it is tall

      // make the height of problem box match background box height
      scaled_height = background_box_height;
      scaled_width = problem_box_aspect * scaled_height;

      if (scaled_width > background_box_width)
      {
         scaled_width = background_box_width;
         scaled_height = scaled_width / problem_box_aspect;
      }
   }
   else
   {
      // the background box is taller than it is wide

      // make the width of problem box match background box width
      scaled_width = background_box_width;
      scaled_height = scaled_width / problem_box_aspect;

      if (scaled_height > background_box_height)
      {
         scaled_height = background_box_height;
         scaled_width = problem_box_aspect * scaled_height;
      }
   }

   QPainter painter { &widget };

   painter.setRenderHint(
      QPainter::RenderHint::SmoothPixmapTransform,
      true);

   painter.drawPixmap(
      QRectF { 
         widget.width() / 2.0 - scaled_width / 2.0,
         widget.height() / 2.0 - scaled_height / 2.0,
         scaled_width,
         scaled_height },
      text_pixmap,
      QRectF {
         problem_crop_x_start,
         0.0,
         text_pixmap.width() - problem_crop_x_start,
         problem_height });
}

void ArithmeticProblem::OnKeyReleaseEvent(
   QKeyEvent * key_event,
   const QWidget & widget ) noexcept
{
   assert(key_event);

   const auto AppendChar =
      [ this ] (
         const char c )
      {
         if (response_.size() < 3)
         {
            response_ += c;
         }
      };

   switch (key_event->key())
   {
   case Qt::Key::Key_0: AppendChar('0'); break;
   case Qt::Key::Key_1: AppendChar('1'); break;
   case Qt::Key::Key_2: AppendChar('2'); break;
   case Qt::Key::Key_3: AppendChar('3'); break;
   case Qt::Key::Key_4: AppendChar('4'); break;
   case Qt::Key::Key_5: AppendChar('5'); break;
   case Qt::Key::Key_6: AppendChar('6'); break;
   case Qt::Key::Key_7: AppendChar('7'); break;
   case Qt::Key::Key_8: AppendChar('8'); break;
   case Qt::Key::Key_9: AppendChar('9'); break;

   case Qt::Key::Key_Backspace:
      if (response_.size())
      {
         response_.resize(
            response_.size() - 1);
      }
      
      break;

   case Qt::Key::Key_Enter: [[fall_through]];
   case Qt::Key::Key_Return:
      GradeAnswer();

      break;
   }
}

void ArithmeticProblem::GradeAnswer( ) noexcept
{
   if (response_.isEmpty())
      return;

   const int32_t response =
      response_.toInt();

   responses_.push_back(
      response);

   const auto result =
      response == answer_ ?
      AnswerResult::CORRECT :
      AnswerResult::INCORRECT;

   if (result == AnswerResult::INCORRECT)
   {
      response_.clear();
   }

   emit
      Answered(result);
}
