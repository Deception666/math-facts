#include "time-problem.hpp"

TimeProblem::TimeProblem( ) noexcept
{
}

TimeProblem::~TimeProblem( ) noexcept
{
}

QVector< QString > TimeProblem::GetResponses( ) const noexcept
{
   return QVector<QString>();
}

QString TimeProblem::GetQuestionWithAnswer( ) const noexcept
{
   return QString();
}

size_t TimeProblem::GetNumberOfResponses( ) const noexcept
{
   return size_t();
}

void TimeProblem::OnPaintEvent(
   QPaintEvent * paint_event,
   QWidget & widget ) noexcept
{
   PaintClock(widget);

   
}

static int current_min = 0;
static int current_hour = 12;

void TimeProblem::OnKeyReleaseEvent(
   QKeyEvent * key_event,
   const QWidget & widget ) noexcept
{
   current_min += 5;

   if (current_min >= 60)
   {
      current_min = 0;
      current_hour += 1;
   }

   if (current_hour >= 13)
      current_hour = 1;
}


#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtWidgets/QWidget>
#include <QtCore/QSize>
#include <QtCore/QRect>
#include <QtCore/Qt>

#include <utility>

void TimeProblem::PaintClock(
   QWidget & widget ) noexcept
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

   const double min_hand_degree =
      current_min == 0 ?
      hour_to_degree[11].first :
      hour_to_degree[current_min / 5 - 1].first;

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
      hour_to_degree[current_hour - 1].first * (59.0 - current_min) / 59.0 +
      hour_to_degree[current_hour - 1].second * current_min / 59.0;

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
}

void TimeProblem::PaintResponse(
   QWidget & widget ) noexcept
{
}

void TimeProblem::GradeAnswer( ) noexcept
{
}
