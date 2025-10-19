#include "time-problem.hpp"

#include <QtCore/QRect>
#include <QtCore/QRectF>
#include <QtCore/QSize>
#include <QtCore/QStringView>
#include <QtCore/Qt>
#include <QtCore/QTime>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QPixmap>
#include <QtGui/QTextOption>
#include <QtWidgets/QWidget>

#include <type_traits>

static QString GetAnswer(
   const std::variant< TimeProblem::Time, TimeProblem::MilitaryTime > & problem ) noexcept
{
   return
      std::visit(
         [ ] (
            auto && argument )
         {
            using T = std::decay_t< decltype(argument) >;

            if constexpr (std::is_same_v< T, TimeProblem::Time > ||
                          std::is_same_v< T, TimeProblem::MilitaryTime >)
               return
                  argument.Answer();
            else
               static_assert(false);
         },
         problem);
}

static auto GetQuestion(
   const std::variant< TimeProblem::Time, TimeProblem::MilitaryTime > & problem ) noexcept
{
   return
      std::visit(
         [ ] (
            auto && argument )
         {
            using T = std::decay_t< decltype(argument) >;

            if constexpr (std::is_same_v< T, TimeProblem::Time > ||
                          std::is_same_v< T, TimeProblem::MilitaryTime >)
               return
                  argument.Question();
            else
               static_assert(false);
         },
         problem);
}

static auto GetHourAndMinute(
   const std::variant< TimeProblem::Time, TimeProblem::MilitaryTime > & problem ) noexcept
{
   return
      std::visit(
         [ ] (
            auto && argument )
         {
            using T = std::decay_t< decltype(argument) >;

            if constexpr (std::is_same_v< T, TimeProblem::Time > ||
                          std::is_same_v< T, TimeProblem::MilitaryTime >)
               return
                  std::make_pair(
                     argument.Hour(),
                     argument.Minute());
            else
               static_assert(false);
         },
         problem);
}

static bool GradeResponse(
   const std::variant< TimeProblem::Time, TimeProblem::MilitaryTime > & problem,
   const QString & response ) noexcept
{
   return
      std::visit(
         [ & ] (
            auto && argument )
         {
            using T = std::decay_t< decltype(argument) >;

            if constexpr (std::is_same_v< T, TimeProblem::Time > ||
                          std::is_same_v< T, TimeProblem::MilitaryTime >)
               return
                  argument.GradeResponse(
                     response);
            else
               static_assert(false);
         },
         problem);
}

TimeProblem::Time::Time(
   const uint8_t hour,
   const uint8_t minute ) noexcept :
hour_ { static_cast< uint8_t >(hour % 12 == 0 ? 12 : hour % 12) },
minute_ { static_cast< uint8_t >((minute % 60) / 5 * 5) }
{
}

bool TimeProblem::Time::GradeResponse(
   const QString & response ) const noexcept
{
   return
      response == Answer();
}

QString TimeProblem::Time::Answer( ) const noexcept
{
   return
      QTime { hour_, minute_ }.toString(
         QStringView { L"h:mm", 4 });
}

std::pair< QString, QSize > TimeProblem::Time::Question( ) const noexcept
{
   return {
      "What time is it?",
      { 2048, 768 }
   };
}

TimeProblem::MilitaryTime::MilitaryTime(
   const uint8_t hour,
   const uint8_t minute,
   const bool is_afternoon ) noexcept :
hour_ { static_cast< uint8_t >(hour % 12 == 0 ? 12 : hour % 12) },
minute_ { static_cast< uint8_t >((minute % 60) / 5 * 5) },
is_afternoon_ { is_afternoon }
{
}

bool TimeProblem::MilitaryTime::GradeResponse(
   const QString & response ) const noexcept
{
   return
      response == Answer();
}

QString TimeProblem::MilitaryTime::Answer( ) const noexcept
{
   const uint8_t hour =
      is_afternoon_ ?
         hour_ == 12 ?
            hour_ :
            hour_ + 12
         :
         hour_ != 12 ?
            hour_ :
            0u;

   return
      QTime { hour, minute_ }.toString(
         QStringView { L"hh:mm", 5 });
}

std::pair< QString, QSize > TimeProblem::MilitaryTime::Question( ) const noexcept
{
   return {
      "What military time is it?",
      { 3328, 768 }
   };
}

TimeProblem::TimeProblem(
   Time time ) noexcept :
problem_ { std::move(time) }
{
}

TimeProblem::TimeProblem(
   MilitaryTime time ) noexcept :
problem_ { std::move(time) }
{
}

TimeProblem::~TimeProblem( ) noexcept
{
}

QVector< QString > TimeProblem::GetResponses( ) const noexcept
{
   return
      responses_;
}

QString TimeProblem::GetQuestionWithAnswer( ) const noexcept
{
   return
      GetQuestion(problem_).first +
      " It is " +
      GetAnswer(problem_);
}

size_t TimeProblem::GetNumberOfResponses( ) const noexcept
{
   return
      responses_.size();
}

void TimeProblem::OnPaintEvent(
   QPaintEvent * paint_event,
   QWidget & widget ) noexcept
{
   std::visit(
      [ &, this ] (
         const auto & argument )
      {
         using T = std::decay_t< decltype(argument) >;

         if constexpr (std::is_same_v< T, Time >)
            PaintTimeProblem(widget);
         else if constexpr (std::is_same_v< T, MilitaryTime >)
            PaintMilitaryTimeProblem(widget);
         else
            static_assert(false);
      },
      problem_);
}

void TimeProblem::OnKeyReleaseEvent(
   QKeyEvent * key_event,
   const QWidget & widget ) noexcept
{
   const auto key =
      key_event->key();

   switch (key)
   {
   case Qt::Key::Key_0: [[fall_through]];
   case Qt::Key::Key_1: [[fall_through]];
   case Qt::Key::Key_2: [[fall_through]];
   case Qt::Key::Key_3: [[fall_through]];
   case Qt::Key::Key_4: [[fall_through]];
   case Qt::Key::Key_5: [[fall_through]];
   case Qt::Key::Key_6: [[fall_through]];
   case Qt::Key::Key_7: [[fall_through]];
   case Qt::Key::Key_8: [[fall_through]];
   case Qt::Key::Key_9: [[fall_through]];
   case Qt::Key::Key_Colon:
      if (response_.size() < 5)
      {
         response_ += static_cast< QChar >(key);
      }

      break;

   case Qt::Key::Key_Backspace:
      response_.removeLast();
      break;

   case Qt::Key::Key_Enter: [[fall_through]];
   case Qt::Key::Key_Return:
      GradeAnswer();
      break;
   }
}

void TimeProblem::PaintTimeProblem(
   QWidget & widget ) noexcept
{
   const QPixmap clock_face =
      RenderClock();
   QSize clock_face_size =
      clock_face.size();

   QPainter widget_painter {
      &widget
   };

   widget_painter.setRenderHint(
      QPainter::RenderHint::SmoothPixmapTransform,
      true);

   const QSize widget_size =
      QSize {
         widget.width() - 60,
         (widget.height() - 60) / 2
      };

   if (widget_size.width() >= widget_size.height())
   {
      clock_face_size.scale(
         widget_size.height(),
         widget_size.height(),
         Qt::AspectRatioMode::KeepAspectRatioByExpanding);
   }
   else
   {
      clock_face_size.scale(
         widget_size.width(),
         widget_size.width(),
         Qt::AspectRatioMode::KeepAspectRatioByExpanding);
   }

   widget_painter.drawPixmap(
      QRect {
         widget.width() / 2 - clock_face_size.width() / 2,
         30,
         clock_face_size.width(),
         clock_face_size.height() },
      clock_face);

   PaintQuestionAndResponse(
      widget);
}

void TimeProblem::PaintMilitaryTimeProblem(
   QWidget & widget ) noexcept
{
   QPixmap morning_afternoon_scene =
      RenderSunScene();
   QPixmap clock_face =
      RenderClock();
   QSize morning_afternoon_scene_size =
      morning_afternoon_scene.size();
   QSize clock_face_size =
      clock_face.size();

   if (morning_afternoon_scene_size.height() > clock_face_size.height())
   {
      morning_afternoon_scene_size.scale(
         clock_face_size.height(),
         clock_face_size.height(),
         Qt::AspectRatioMode::KeepAspectRatio);

      morning_afternoon_scene =
         morning_afternoon_scene.scaled(
            morning_afternoon_scene_size,
            Qt::AspectRatioMode::IgnoreAspectRatio,
            Qt::TransformationMode::SmoothTransformation);
   }
   else if (clock_face_size.height() > morning_afternoon_scene_size.height())
   {
      clock_face_size.scale(
         morning_afternoon_scene_size.height(),
         morning_afternoon_scene_size.height(),
         Qt::AspectRatioMode::KeepAspectRatio);

      clock_face =
         clock_face.scaled(
            clock_face_size,
            Qt::AspectRatioMode::IgnoreAspectRatio,
            Qt::TransformationMode::SmoothTransformation);
   }

   QPixmap combined_clock_and_scene {
      clock_face_size.width() + morning_afternoon_scene_size.width() + 10,
      clock_face_size.height()
   };

   QSize combined_clock_and_scene_size =
      combined_clock_and_scene.size();

   combined_clock_and_scene.fill(
      Qt::transparent);

   QPainter combined_clock_and_scene_painter {
      &combined_clock_and_scene
   };

   combined_clock_and_scene_painter.drawPixmap(
      0, 0,
      clock_face);
   combined_clock_and_scene_painter.drawPixmap(
      clock_face_size.width() + 10, 0,
      morning_afternoon_scene);

   QPainter widget_painter {
      &widget
   };

   widget_painter.setRenderHint(
      QPainter::RenderHint::SmoothPixmapTransform,
      true);

   const QSize widget_size =
      QSize {
         widget.width() - 60,
         (widget.height() - 60) / 2
      };

   if (widget_size.width() >= widget_size.height())
   {
      combined_clock_and_scene_size.scale(
         widget_size.width(),
         widget_size.height(),
         Qt::AspectRatioMode::KeepAspectRatio);
   }
   else
   {
      combined_clock_and_scene_size.scale(
         widget_size.width(),
         widget_size.height(),
         Qt::AspectRatioMode::KeepAspectRatio);
   }

   widget_painter.drawPixmap(
      QRect {
         widget.width() / 2 - combined_clock_and_scene_size.width() / 2,
         30,
         combined_clock_and_scene_size.width(),
         combined_clock_and_scene_size.height() },
      combined_clock_and_scene);

   PaintQuestionAndResponse(
      widget);
}

void TimeProblem::PaintQuestionAndResponse(
   QWidget & widget ) noexcept
{
   const auto [
      question,
      text_pixmap_size ] =
      GetQuestion(problem_);

   // pixmap is at pixel ratio of 1.0
   QPixmap text_pixmap {
      text_pixmap_size
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
   // set the color of the font
   text_painter.setPen(
      QPen {
         QBrush { GetTextColor() },
         10.0
      });

   text_painter.drawText(
      QRect { 0, 0, text_pixmap.width(), text_pixmap.height() },
      question + "\n" + response_,
      QTextOption { Qt::AlignmentFlag::AlignHCenter });

   QPainter widget_painter {
      &widget
   };

   widget_painter.setRenderHint(
      QPainter::RenderHint::SmoothPixmapTransform,
      true);

   QSize text_size =
      text_pixmap.size();

   text_size.scale(
      widget.width() - 120.0,
      (widget.height() - 120.0) / 2.0 - 2.0,
      Qt::AspectRatioMode::KeepAspectRatio);

   widget_painter.drawPixmap(
      QRectF { 
         widget.width() / 2.0 - text_size.width() / 2.0,
         widget.height() / 2.0 + 2.0,
         static_cast< qreal >(text_size.width()),
         static_cast< qreal >(text_size.height()), },
      text_pixmap,
      QRectF {
         0.0,
         0.0,
         static_cast< qreal >(text_pixmap.width()),
         static_cast< qreal >(text_pixmap.height()) });
}

QPixmap TimeProblem::RenderClock( ) const noexcept
{
   QPixmap clock_face { ":/clock-face-image" };
   const QPixmap hour_hand { ":/clock-hour-hand-image" };
   const QPixmap minute_hand { ":/clock-minute-hand-image" };
   const QPixmap center_post { ":/clock-center-post-image" };

   QPainter clock_face_painter {
      &clock_face
   };

   clock_face_painter.setRenderHint(
      QPainter::RenderHint::SmoothPixmapTransform,
      true);

   QSize clock_face_size =
      clock_face.size();

   clock_face_painter.translate(
      clock_face_size.width() / 2.0,
      clock_face_size.height() / 2.0);

   const std::pair< double, double > hour_to_degree[] {
      { 30.0, 60.0 },   { 60.0, 90.0 },   { 90.0, 120.0 },
      { 120.0, 150.0 }, { 150.0, 180.0 }, { 180.0, 210.0 },
      { 210.0, 240.0 }, { 240.0, 270.0 }, { 270.0, 300.0 },
      { 300.0, 330.0 }, { 330.0, 360.0 }, { 0.0, 30.0 },
   };

   const auto [
      problem_hour,
      problem_minute ] =
      GetHourAndMinute(problem_);

   const double min_hand_degree =
      problem_minute < 5 ?
      hour_to_degree[11].first :
      hour_to_degree[problem_minute / 5 - 1].first;

   clock_face_painter.rotate(
      min_hand_degree);

   clock_face_painter.drawPixmap(
      QRect {
         -21, -219,
         minute_hand.width(),
         minute_hand.height() },
      minute_hand);

   clock_face_painter.resetTransform();

   clock_face_painter.translate(
      clock_face_size.width() / 2.0,
      clock_face_size.height() / 2.0);

   const double hour_hand_degree =
      hour_to_degree[problem_hour - 1].first * (59.0 - problem_minute) / 59.0 +
      hour_to_degree[problem_hour - 1].second * problem_minute / 59.0;

   clock_face_painter.rotate(
      hour_hand_degree);

   clock_face_painter.drawPixmap(
      QRect {
         -22, -161,
         hour_hand.width(),
         hour_hand.height() },
      hour_hand);

   clock_face_painter.resetTransform();

   clock_face_painter.drawPixmap(
      clock_face_size.width() / 2.0 - center_post.size().width() / 2.0,
      clock_face_size.height() / 2.0 - center_post.size().height() / 2.0,
      center_post);

   return
      clock_face;
}

QPixmap TimeProblem::RenderSunScene( ) const noexcept
{
   QPixmap morning_afternoon_scene { ":/morning-afternoon-scene-image" };
   const QPixmap morning_sun { ":/morning-sun-image" };
   const QPixmap afternoon_sun { ":/afternoon-sun-image" };

   QPainter scene_painter {
      &morning_afternoon_scene
   };

   scene_painter.setRenderHint(
      QPainter::RenderHint::SmoothPixmapTransform,
      true);

   QSize morning_afternoon_scene_size =
      morning_afternoon_scene.size();

   const bool is_afternoon =
      std::get< MilitaryTime >(problem_).IsAfternoon();

   if (is_afternoon)
   {
      scene_painter.drawPixmap(
         morning_afternoon_scene.width() - afternoon_sun.width() - 10,
         10,
         afternoon_sun);
   }
   else
   {
      scene_painter.drawPixmap(
         10, 10,
         morning_sun);
   }

   return
      morning_afternoon_scene;
}

void TimeProblem::GradeAnswer( ) noexcept
{
   if (response_.isEmpty())
      return;

   responses_.push_back(
      response_);

   const auto result =
      GradeResponse(
         problem_,
         response_) ?
      AnswerResult::CORRECT :
      AnswerResult::INCORRECT;

   if (result == AnswerResult::INCORRECT)
   {
      response_.clear();
   }

   emit
      Answered(result);
}
