#include <QtCore/QRect>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtCore/Qt>
#include <QtGui/QBrush>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QPixmap>
#include <QtGui/QRgb>
#include <QtGui/QTextOption>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

#include <algorithm>
#include <array>
#include <cstdint>
#include <random>

class MathFactsWidget :
   public QWidget
{
public:
   MathFactsWidget(
      QWidget * const parent ) noexcept;

protected:
   virtual void paintEvent(
      QPaintEvent * paint_event ) override;
   virtual void keyReleaseEvent(
      QKeyEvent * event ) override;

private:
   struct Colors
   {
      QColor background;
      QColor border;
      QColor text;
   };

   struct Problem
   {
      QString line1;
      QString line2;
      QString line3;
      int32_t answer;
   };

   struct Randomizers
   {
      std::default_random_engine random_engine {
         std::random_device { } ()
      };

      std::uniform_int_distribution< uint32_t > problem_distribution {
         0, 2
      };

      std::vector< Problem > addition_problems;
      std::vector< Problem > subtraction_problems;
      std::vector< Problem > multiplication_problems;
   };

   void SetupColors( ) noexcept;

   Problem GenerateProblem( ) noexcept;
   void GenerateAdditionProblem( ) noexcept;
   void GenerateSubtractionProblem( ) noexcept;
   void GenerateMultiplicationProblem( ) noexcept;

   void GradeAnswer( ) noexcept;

   void PaintProblem(
      QPaintEvent * paint_event ) noexcept;
   void PaintBackground(
      QPaintEvent * paint_event ) noexcept;
   void PaintProblemText(
      QPaintEvent * paint_event ) noexcept;

   const Colors * current_colors_;
   std::array< Colors, 6 > colors_;

   Problem current_problem_;
   Randomizers randomizers_;

};

MathFactsWidget::MathFactsWidget(
   QWidget * const parent ) noexcept :
QWidget { parent },
current_colors_ { nullptr }
{
   SetupColors();

   current_problem_ =
      GenerateProblem();
}

void MathFactsWidget::paintEvent(
   QPaintEvent * paint_event )
{
   QWidget::paintEvent(
      paint_event);

   PaintProblem(
      paint_event);
}

void MathFactsWidget::keyReleaseEvent(
   QKeyEvent * event )
{
   const auto AppendChar =
      [ this ] (
         const char c )
      {
         if (current_problem_.line3.size() < 3)
         {
            current_problem_.line3 += c;
         }
      };

   switch (event->key())
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
      if (current_problem_.line3.size())
      {
         current_problem_.line3.resize(
            current_problem_.line3.size() - 1);
      }
      
      break;

   case Qt::Key::Key_Enter:
      [[fall_through]];

   case Qt::Key::Key_Return:
      GradeAnswer();

      break;
   }

   update();
}

void MathFactsWidget::SetupColors( ) noexcept
{
   colors_[0] =
      Colors {
         QColor { 0xACFF7E },
         QColor { 0xEAFFDC },
         QColor { 0x155D00 }
      };

   colors_[1] =
      Colors {
         QColor { 0xEF99FF },
         QColor { 0xFFDFFE },
         QColor { 0x9D15D5 }
      };

   colors_[2] =
      Colors {
         QColor { 0xFFB23A },
         QColor { 0xFFEBD5 },
         QColor { 0xBD4E00 }
      };

   colors_[3] =
      Colors {
         QColor { 0x77D7FF },
         QColor { 0xD4FFFD },
         QColor { 0x02497C }
      };

   colors_[4] =
      Colors {
         QColor { 0xFFB1B1 },
         QColor { 0xFFDCDC },
         QColor { 0xC40300 }
      };

   colors_[5] =
      Colors {
         QColor { 0x3AF5BE },
         QColor { 0xCCFFF2 },
         QColor { 0x036247 }
      };

   current_colors_ =
      &colors_[0];
}

MathFactsWidget::Problem MathFactsWidget::GenerateProblem( ) noexcept
{
   if (randomizers_.addition_problems.empty() &&
       randomizers_.subtraction_problems.empty() &&
       randomizers_.multiplication_problems.empty())
   {
      GenerateAdditionProblem();
      GenerateSubtractionProblem();
      GenerateMultiplicationProblem();
   }

   std::vector< Problem > * const problems[] {
      &randomizers_.addition_problems,
      &randomizers_.subtraction_problems,
      &randomizers_.multiplication_problems
   };

   uint32_t problem_type =
      randomizers_.problem_distribution(
         randomizers_.random_engine);

   while (problems[problem_type]->empty())
   {
      problem_type =
         randomizers_.problem_distribution(
            randomizers_.random_engine);
   }

   Problem problem {
      std::move(problems[problem_type]->back())
   };

   problems[problem_type]->pop_back();

   return
      problem;
}

void MathFactsWidget::GenerateAdditionProblem( ) noexcept
{
   for (int32_t top { }; top <= 12; ++top)
   {
      for (int32_t bottom { }; bottom <= 12; ++bottom)
      {
         randomizers_.addition_problems.emplace_back(
            Problem {
               QString::number(top),
               "+  " + QString::number(bottom),
               QString { },
               top + bottom
            });
      }
   }

   std::shuffle(
      randomizers_.addition_problems.begin(),
      randomizers_.addition_problems.end(),
      std::default_random_engine {
         randomizers_.random_engine });
}

void MathFactsWidget::GenerateSubtractionProblem( ) noexcept
{
   for (int32_t top { }; top <= 12; ++top)
   {
      for (int32_t bottom { }; bottom <= 12; ++bottom)
      {
         if (top >= bottom)
         {
            randomizers_.subtraction_problems.emplace_back(
               Problem {
                  QString::number(top),
                  "-  " + QString::number(bottom),
                  QString { },
                  top - bottom
               });
         }
      }
   }

   std::shuffle(
      randomizers_.subtraction_problems.begin(),
      randomizers_.subtraction_problems.end(),
      std::default_random_engine {
         randomizers_.random_engine });
}

void MathFactsWidget::GenerateMultiplicationProblem( ) noexcept
{
   for (int32_t top { }; top <= 10; ++top)
   {
      for (int32_t bottom { }; bottom <= 10; ++bottom)
      {
         randomizers_.multiplication_problems.emplace_back(
            Problem {
               QString::number(top),
               "x  " + QString::number(bottom),
               QString { },
               top * bottom
            });
      }
   }

   std::shuffle(
      randomizers_.multiplication_problems.begin(),
      randomizers_.multiplication_problems.end(),
      std::default_random_engine {
         randomizers_.random_engine });
}

void MathFactsWidget::GradeAnswer( ) noexcept
{
   const int32_t response =
      current_problem_.line3.toInt();

   if (response == current_problem_.answer)
   {
      current_problem_ =
         GenerateProblem();

      const size_t new_colors_index =
         std::distance(
            &*colors_.cbegin(),
            current_colors_ + 1) % colors_.size();

      current_colors_ =
         &colors_[new_colors_index];
   }
   else
   {
   }
}

void MathFactsWidget::PaintProblem(
   QPaintEvent * paint_event ) noexcept
{
   PaintBackground(
      paint_event);

   PaintProblemText(
      paint_event);
}

void MathFactsWidget::PaintBackground(
   QPaintEvent * paint_event ) noexcept
{
   QPainter painter {
      this
   };

   painter.setBrush(
      QBrush {
         current_colors_->background
      });

   painter.drawRect(
      QRect { 0, 0, width(), height() });

   painter.setBrush(
      QBrush {
         current_colors_->border
      });
   painter.setPen(
      current_colors_->border);

   painter.drawRoundedRect(
      QRect { 30, 30, width() - 60, height() - 60 },
      15.0,
      15.0);
}

void MathFactsWidget::PaintProblemText(
   QPaintEvent * paint_event ) noexcept
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

   const QFont font {
      "Comic Sans MS",
      200
   };

   text_painter.setFont(
      font);

   // set the line thickness
   // set the color of line and font
   text_painter.setPen(
      QPen {
         QBrush { current_colors_->text },
         10.0
      });

   text_painter.setRenderHint(
      QPainter::RenderHint::TextAntialiasing);

   const QFontMetrics font_metrics {
      text_painter.font()
   };

   // problem two is always the longest line
   const QRect text_bounding_rect =
      font_metrics.tightBoundingRect(
         current_problem_.line2);

   // add a bit of space between the lines
   const int32_t line_height =
      text_bounding_rect.height() + 10;

   text_painter.drawText(
      QRect { 0, 0, text_pixmap.width(), text_pixmap.height() },
      current_problem_.line1,
      QTextOption { Qt::AlignmentFlag::AlignRight });
   text_painter.drawText(
      QRect { 0, line_height, text_pixmap.width(), text_pixmap.height() },
      current_problem_.line2,
      QTextOption { Qt::AlignmentFlag::AlignRight });

   const int32_t max_line_width =
      text_bounding_rect.width();

   // draw the answer line
   text_painter.drawLine(
      QPointF {
         static_cast< qreal >(text_pixmap.width()),
         line_height * 2.0 * devicePixelRatio() },
      QPointF {
         static_cast< qreal >(text_pixmap.width() - max_line_width),
         line_height * 2.0 * devicePixelRatio() });

   text_painter.drawText(
      QRect { 0, line_height * 2 + 40, text_pixmap.width(), text_pixmap.height() },
      current_problem_.line3,
      QTextOption { Qt::AlignmentFlag::AlignRight });

   // aspect of the inner background box
   const qreal background_box_width { width() - 60.0 };
   const qreal background_box_height { height() - 60.0 };
   
   const qreal background_box_aspect {
      background_box_width / background_box_height
   };

   const qreal problem_width =
      max_line_width * devicePixelRatio();
   const qreal problem_height =
      (line_height * 3 + 40) * devicePixelRatio();
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

   QPainter { this }.drawPixmap(
      QRectF { 
         width() / 2.0 - scaled_width / 2.0,
         height() / 2.0 - scaled_height / 2.0,
         scaled_width,
         scaled_height },
      text_pixmap,
      QRectF {
         problem_crop_x_start,
         0.0,
         text_pixmap.width() - problem_crop_x_start,
         problem_height });
}

int main(
   int argc,
   char ** argv )
{
   QApplication application {
      argc,
      argv
   };

   application.setWindowIcon(
      QIcon { ":/mainicon" });

   MathFactsWidget math_facts_widget {
      nullptr
   };

   math_facts_widget.setMinimumSize(
      QSize { 200, 200 });

   math_facts_widget.show();

   return
      application.exec();
}
