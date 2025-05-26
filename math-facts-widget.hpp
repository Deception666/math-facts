#ifndef _MATH_FACTS_WIDGET_HPP_
#define _MATH_FACTS_WIDGET_HPP_

#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtGui/QColor>
#include <QtGui/QKeyEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QPixmap>
#include <QtWidgets/QWidget>

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <vector>

class QSettings;

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
      std::vector< int32_t > responses;
      std::chrono::steady_clock::time_point start;
      std::chrono::steady_clock::time_point end;
   };

   struct Randomizers
   {
      std::default_random_engine random_engine {
         std::random_device { } ()
      };

      std::uniform_int_distribution< uint32_t > problem_distribution {
         0, 3
      };

      std::vector< Problem > addition_problems;
      std::vector< Problem > subtraction_problems;
      std::vector< Problem > multiplication_problems;
      std::vector< Problem > division_problems;
   };

   struct Stopwatch
   {
      QPixmap base_image;
      QPixmap hand_image;

      QTimer periodic_update_timer;

      std::chrono::steady_clock::time_point start_time;
      std::chrono::steady_clock::time_point end_time;

      bool PracticeTimeExceeded( ) const noexcept
      {
         return
            std::chrono::steady_clock::now() >=
            end_time;
      }
   };

   enum class Stage : uint8_t
   {
      TITLE,
      MATH_PRACTICE
   };

   enum EnabledMathFactBits : uint32_t
   {
      ADD = 0x01,
      SUB = 0x02,
      MUL = 0x04,
      DIV = 0x08,
      ALL = 0x0F
   };

   void SetupColors( ) noexcept;
   void SetupAnswerImages( ) noexcept;
   void SetupStopwatchImages( ) noexcept;
   void SetupTitleStage( ) noexcept;

   Problem GenerateProblem( ) noexcept;
   void GenerateAdditionProblem( ) noexcept;
   void GenerateSubtractionProblem( ) noexcept;
   void GenerateMultiplicationProblem( ) noexcept;
   void GenerateDivisionProblem( ) noexcept;

   void GradeAnswer( ) noexcept;
   void WriteReport( ) const noexcept;

   std::string GetCurrentUserName( ) const noexcept;
   std::string GenerateReportName( ) const noexcept;
   std::unique_ptr< QSettings > GetSettings( ) const noexcept;
   uint32_t GetEnabledMathFacts( ) const noexcept;
   std::filesystem::path GetReportsDirectory( ) const noexcept;
   std::chrono::milliseconds GetMathPracticeDuration( ) const noexcept;
   std::chrono::milliseconds CalculateStandardDeviationResponseTime( ) const noexcept;
   uint32_t GetMinimumAmountToPractice( ) const noexcept;

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

   Randomizers randomizers_;
   Problem current_problem_;
   std::vector< Problem > answered_problems_;
   
   const QPixmap * answer_image_;
   QPixmap wrong_answer_image_;
   QPixmap correct_answer_image_;

   Stopwatch practice_stopwatch_;
   uint32_t minimum_amount_to_practice_;

};

#endif // _MATH_FACTS_WIDGET_HPP_
