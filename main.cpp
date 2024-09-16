#include <QtCore/QFile>
#include <QtCore/QRect>
#include <QtCore/QRectF>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtCore/Qt>
#include <QtCore/QtGlobal>
#include <QtCore/QTimer>
#include <QtGui/QBrush>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QIcon>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QPixmap>
#include <QtGui/QTextOption>
#include <QtUiTools/QUiLoader>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
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

private slots:
   enum class TitleButtonID : uint8_t
   {
      ADD,
      SUB,
      MUL,
      DIV,
      ALL
   };

   void OnAnswerImageTimeout( ) noexcept;
   void OnTitleButtonPressed(
      const TitleButtonID title_button_id ) noexcept;

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

   struct Stopwatch
   {
      QPixmap base_image;
      QPixmap hand_image;

      QTimer periodic_update_timer;

      std::chrono::steady_clock::time_point start_time;
      std::chrono::steady_clock::time_point end_time;
   };

   enum class Stage : uint8_t
   {
      TITLE,
      MATH_PRACTICE
   };

   void SetupColors( ) noexcept;
   void SetupAnswerImages( ) noexcept;
   void SetupStopwatchImages( ) noexcept;
   void SetupTitleStage( ) noexcept;

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
   void PaintAnswerImage(
      QPaintEvent * paint_event ) noexcept;
   void PaintStopwatch(
      QPaintEvent * paint_event ) noexcept;
   void PaintTitleStage(
      QPaintEvent * paint_event ) noexcept;

   Stage current_stage_;

   TitleButtonID chosen_problems_;
   std::unique_ptr< QWidget > title_stage_buttons_;

   const Colors * current_colors_;
   std::array< Colors, 6 > colors_;

   Problem current_problem_;
   Randomizers randomizers_;

   const QPixmap * answer_image_;
   QPixmap wrong_answer_image_;
   QPixmap correct_answer_image_;

   Stopwatch practice_stopwatch_;

};

MathFactsWidget::MathFactsWidget(
   QWidget * const parent ) noexcept :
QWidget { parent },
current_stage_ { Stage::TITLE },
chosen_problems_ { },
title_stage_buttons_ { nullptr },
current_colors_ { nullptr },
answer_image_ { nullptr }
{
   SetupColors();
   SetupAnswerImages();
   SetupStopwatchImages();
   SetupTitleStage();
}

void MathFactsWidget::OnAnswerImageTimeout( ) noexcept
{
   answer_image_ = nullptr;

   update();
}

void MathFactsWidget::OnTitleButtonPressed(
   const TitleButtonID title_button_id ) noexcept
{
   if (title_button_id != TitleButtonID::DIV)
   {
      chosen_problems_ =
         title_button_id;

      title_stage_buttons_.reset();

      current_stage_ =
         Stage::MATH_PRACTICE;

      current_problem_ =
         GenerateProblem();

      update();

      QObject::connect(
         &practice_stopwatch_.periodic_update_timer,
         &QTimer::timeout,
         this,
         qOverload< >(&MathFactsWidget::update));

      practice_stopwatch_.periodic_update_timer.start(
         std::chrono::milliseconds { 500 });

      practice_stopwatch_.start_time =
         std::chrono::steady_clock::now();
      practice_stopwatch_.end_time =
         practice_stopwatch_.start_time +
         std::chrono::minutes { 5 };
   }
}

void MathFactsWidget::paintEvent(
   QPaintEvent * paint_event )
{
   QWidget::paintEvent(
      paint_event);

   if (Stage::TITLE == current_stage_)
   {
      PaintTitleStage(
         paint_event);
   }
   else
   {
      PaintProblem(
         paint_event);
   }
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

void MathFactsWidget::SetupAnswerImages( ) noexcept
{
   wrong_answer_image_.load(
      ":/wrong-answer-image");
   correct_answer_image_.load(
      ":/correct-answer-image");
}

void MathFactsWidget::SetupStopwatchImages( ) noexcept
{
   practice_stopwatch_.base_image.load(
      ":/stopwatch-base-image");
   practice_stopwatch_.hand_image.load(
      ":/stopwatch-hand-image");
}

void MathFactsWidget::SetupTitleStage( ) noexcept
{
   QFile title_stage_buttons_file {
      ":/title-stage-buttons-ui"
   };

   // assume it will always open
   title_stage_buttons_file.open(
      QFile::OpenModeFlag::ReadOnly);

   title_stage_buttons_.reset(
      QUiLoader { }.load(
         &title_stage_buttons_file,
         nullptr)); // ownership via smart pointer

   if (title_stage_buttons_)
   {
      title_stage_buttons_->setParent(
         this);

      struct ButtonSetup
      {
         const char * const object_name;
         QRect image_sheet_rect;
         TitleButtonID id;
      };

      const QPixmap math_buttons_image_sheet {
         ":/math-buttons-image-sheet"
      };

      const ButtonSetup button_setup[] {
         { "pushButtonAdd", { 0, 0, 141, 143 }, TitleButtonID::ADD },
         { "pushButtonSub", { 142, 0, 141, 143 }, TitleButtonID::SUB },
         { "pushButtonMul", { 284, 0, 141, 143 }, TitleButtonID::MUL },
         { "pushButtonDiv", { 428, 0, 141, 143 }, TitleButtonID::DIV },
         { "pushButtonAll", math_buttons_image_sheet.rect(), TitleButtonID::ALL }
      };

      for (const auto & button : button_setup)
      {
         QPushButton * push_button =
            title_stage_buttons_->findChild< QPushButton * >(
               button.object_name,
               Qt::FindChildOption::FindChildrenRecursively);

         QPixmap button_image =
            math_buttons_image_sheet.copy(
               button.image_sheet_rect);

         push_button->setIcon(
            QIcon { button_image });
         push_button->setIconSize(
            QSize { 
               math_buttons_image_sheet.width() / 2,
               math_buttons_image_sheet.height() / 2 });

         QObject::connect(
            push_button,
            &QPushButton::pressed,
            std::bind(
               &MathFactsWidget::OnTitleButtonPressed,
               this,
               button.id));
      }
   }
}

MathFactsWidget::Problem MathFactsWidget::GenerateProblem( ) noexcept
{
   if (randomizers_.addition_problems.empty() &&
       randomizers_.subtraction_problems.empty() &&
       randomizers_.multiplication_problems.empty())
   {
      if (TitleButtonID::ADD == chosen_problems_ ||
          TitleButtonID::ALL == chosen_problems_)
      {
         GenerateAdditionProblem();
      }

      if (TitleButtonID::SUB == chosen_problems_ ||
          TitleButtonID::ALL == chosen_problems_)
      {
         GenerateSubtractionProblem();
      }

      if (TitleButtonID::MUL == chosen_problems_ ||
          TitleButtonID::ALL == chosen_problems_)
      {
         GenerateMultiplicationProblem();
      }
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
   for (int32_t top { }; top <= 12; ++top)
   {
      for (int32_t bottom { }; bottom <= 12; ++bottom)
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

      answer_image_ = &correct_answer_image_;
   }
   else
   {
      current_problem_.line3.clear();

      answer_image_ = &wrong_answer_image_;
   }

   QTimer::singleShot(
      std::chrono::seconds { 1 },
      this,
      &MathFactsWidget::OnAnswerImageTimeout);
}

void MathFactsWidget::PaintProblem(
   QPaintEvent * paint_event ) noexcept
{
   PaintBackground(
      paint_event);

   PaintProblemText(
      paint_event);

   PaintAnswerImage(
      paint_event);

   PaintStopwatch(
      paint_event);
}

void MathFactsWidget::PaintBackground(
   QPaintEvent * paint_event ) noexcept
{
   QPainter painter {
      this
   };

   painter.setRenderHint(
      QPainter::RenderHint::Antialiasing,
      true);

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

   text_painter.setRenderHint(
      QPainter::RenderHint::Antialiasing,
      true);
   text_painter.setRenderHint(
      QPainter::RenderHint::TextAntialiasing,
      true);

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

   const QFontMetrics font_metrics {
      text_painter.font()
   };

   // problem two is always the longest line
   const QRect text_bounding_rect =
      font_metrics.tightBoundingRect(
         current_problem_.line2);

   // add a bit of space between the lines
   const int32_t line_space_gap { 10 };
   const int32_t line_height =
      text_bounding_rect.height() +
      line_space_gap;

   text_painter.drawText(
      QRect { 0, 0, text_pixmap.width(), text_pixmap.height() },
      current_problem_.line1,
      QTextOption { Qt::AlignmentFlag::AlignRight });

   QRect line2_bounding_box;
   
   text_painter.drawText(
      QRect { 0, line_height, text_pixmap.width(), text_pixmap.height() },
      Qt::AlignmentFlag::AlignRight,
      current_problem_.line2,
      &line2_bounding_box);

   text_painter.drawText(
      QRect { 0, line_height * 2 + 40, text_pixmap.width(), text_pixmap.height() },
      current_problem_.line3,
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
   const qreal background_box_width { width() - 60.0 };
   const qreal background_box_height { height() - 60.0 };
   
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

   QPainter painter { this };

   painter.setRenderHint(
      QPainter::RenderHint::SmoothPixmapTransform,
      true);

   painter.drawPixmap(
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

void MathFactsWidget::PaintAnswerImage(
   QPaintEvent * paint_event ) noexcept
{
   if (answer_image_)
   {
      const qreal window_length =
         std::min(
            width() * 0.3,
            height() * 0.3);

      QPainter painter { this };

      painter.setRenderHint(
         QPainter::RenderHint::SmoothPixmapTransform,
         true);

      painter.drawPixmap(
         QRectF { 30.0, 30.0, window_length, window_length },
         *answer_image_,
         answer_image_->rect());
   }
}

void MathFactsWidget::PaintStopwatch(
   QPaintEvent * paint_event) noexcept
{
   assert(
      practice_stopwatch_.base_image.size() ==
      (QSize { 512, 512 }));
   assert(
      practice_stopwatch_.hand_image.size() ==
      (QSize { 42, 152 }));

   QPixmap stopwatch_pixmap {
      practice_stopwatch_.base_image };

   QPainter stopwatch_painter {
      &stopwatch_pixmap };

   const auto total_time =
      practice_stopwatch_.end_time -
      practice_stopwatch_.start_time;
   const auto remaining_time =
      practice_stopwatch_.end_time -
      std::chrono::steady_clock::now();

   const qreal hand_rotation =
      remaining_time.count() * 360.0 / total_time.count();

   stopwatch_painter.translate(
      256.0,
      284.0);
   stopwatch_painter.rotate(
      hand_rotation);

   stopwatch_painter.drawPixmap(
      QRect {
         -21, -131,
         practice_stopwatch_.hand_image.width(),
         practice_stopwatch_.hand_image.height() },
      practice_stopwatch_.hand_image);

   const qreal window_length =
      std::min(
         width() * 0.3,
         height() * 0.3);

   QPainter painter { this };

   painter.setRenderHint(
      QPainter::RenderHint::SmoothPixmapTransform,
      true);

   painter.drawPixmap(
      QRectF {
         30.0, height() - window_length - 30.0,
         window_length, window_length },
      stopwatch_pixmap,
      stopwatch_pixmap.rect());
}

void MathFactsWidget::PaintTitleStage(
   QPaintEvent * paint_event ) noexcept
{
   PaintBackground(
      paint_event);

   const auto title_pixmap =
      QPixmap { ":/math-facts-title-image" }.scaledToWidth(
         width() - 60,
         Qt::TransformationMode::SmoothTransformation);

   QPainter title_painter { this };

   title_painter.setRenderHint(
      QPainter::RenderHint::Antialiasing,
      true);
   title_painter.setRenderHint(
      QPainter::RenderHint::SmoothPixmapTransform,
      true);

   title_painter.drawPixmap(
      QRect { 30, 30, width() - 60, title_pixmap.height() },
      title_pixmap);

   if (title_stage_buttons_)
   {
      title_stage_buttons_->setGeometry(
         QRect {
            0,
            title_pixmap.height() + 40,
            width(),
            height() - title_pixmap.height() - 70 });
   }
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
   application.setStyle(
      "Fusion");

   MathFactsWidget math_facts_widget {
      nullptr
   };

   math_facts_widget.setMinimumSize(
      QSize { 200, 200 });

   math_facts_widget.show();

   return
      application.exec();
}
