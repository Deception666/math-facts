#include <QtCore/QFile>
#include <QtCore/QRect>
#include <QtCore/QRectF>
#include <QtCore/QSettings>
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
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <random>

#if __has_include(<format>)
#  include <format>
#else
#  include <ctime>
#  include <time.h>
#endif // __has_include(<format>)

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

MathFactsWidget::MathFactsWidget(
   QWidget * const parent ) noexcept :
QWidget { parent },
current_stage_ { Stage::TITLE },
chosen_problems_ { },
title_stage_buttons_ { nullptr },
current_colors_ { nullptr },
answer_image_ { nullptr },
minimum_amount_to_practice_ { 50 }
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

   minimum_amount_to_practice_ =
      GetMinimumAmountToPractice();

   practice_stopwatch_.start_time =
      std::chrono::steady_clock::now();
   practice_stopwatch_.end_time =
      practice_stopwatch_.start_time +
      GetMathPracticeDuration();
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
      // [[fall_through]];

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
      const uint32_t enabled_math_facts =
         GetEnabledMathFacts();

      title_stage_buttons_->setParent(
         this);

      struct ButtonSetup
      {
         const char * const object_name;
         const char * const resource_id;
         TitleButtonID id;
         EnabledMathFactBits enabled_math_fact_bit;
      };

      const ButtonSetup button_setup[] {
         { "pushButtonAdd", ":/math-button-image-add", TitleButtonID::ADD, EnabledMathFactBits::ADD },
         { "pushButtonSub", ":/math-button-image-sub", TitleButtonID::SUB, EnabledMathFactBits::SUB },
         { "pushButtonMul", ":/math-button-image-mul", TitleButtonID::MUL, EnabledMathFactBits::MUL },
         { "pushButtonDiv", ":/math-button-image-div", TitleButtonID::DIV, EnabledMathFactBits::DIV },
         { "pushButtonAll", ":/math-buttons-image-sheet", TitleButtonID::ALL, EnabledMathFactBits::ALL }
      };

      for (const auto & button : button_setup)
      {
         QPushButton * push_button =
            title_stage_buttons_->findChild< QPushButton * >(
               button.object_name,
               Qt::FindChildOption::FindChildrenRecursively);

         push_button->setStyleSheet(
            QString {
               "QPushButton { \
                  background-color: rgba(0,0,0,0); \
                  border-style: outset; \
                  border-width: 2px; \
                  border-radius: 10px; \
                  border-color: beige; \
                  padding: 1px; \
                  image: url(%1); \
               } \
               QPushButton:hover:!pressed { \
                  background-color: rgba(172,255,126,255); \
               }"
            }.arg(button.resource_id));

         QObject::connect(
            push_button,
            &QPushButton::pressed,
            std::bind(
               &MathFactsWidget::OnTitleButtonPressed,
               this,
               button.id));

         if (button.id == TitleButtonID::ALL)
         {
            push_button->setEnabled(
               enabled_math_facts != 0u);
         }
         else
         {
            push_button->setEnabled(
               enabled_math_facts & button.enabled_math_fact_bit);
         }
      }
   }
}

MathFactsWidget::Problem MathFactsWidget::GenerateProblem( ) noexcept
{
   if (randomizers_.addition_problems.empty() &&
       randomizers_.subtraction_problems.empty() &&
       randomizers_.multiplication_problems.empty() &&
       randomizers_.division_problems.empty())
   {
      const uint32_t enabled_math_facts =
         GetEnabledMathFacts();

      if (enabled_math_facts & EnabledMathFactBits::ADD &&
         (TitleButtonID::ADD == chosen_problems_ ||
          TitleButtonID::ALL == chosen_problems_))
      {
         GenerateAdditionProblem();
      }

      if (enabled_math_facts & EnabledMathFactBits::SUB &&
         (TitleButtonID::SUB == chosen_problems_ ||
          TitleButtonID::ALL == chosen_problems_))
      {
         GenerateSubtractionProblem();
      }

      if (enabled_math_facts & EnabledMathFactBits::MUL &&
         (TitleButtonID::MUL == chosen_problems_ ||
          TitleButtonID::ALL == chosen_problems_))
      {
         GenerateMultiplicationProblem();
      }

      if (enabled_math_facts & EnabledMathFactBits::DIV &&
         (TitleButtonID::DIV == chosen_problems_ ||
          TitleButtonID::ALL == chosen_problems_))
      {
         GenerateDivisionProblem();
      }
   }

   std::vector< Problem > * const problems[] {
      &randomizers_.addition_problems,
      &randomizers_.subtraction_problems,
      &randomizers_.multiplication_problems,
      &randomizers_.division_problems
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

   problem.start =
      std::chrono::steady_clock::now();

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

void MathFactsWidget::GenerateDivisionProblem () noexcept
{
   for (int32_t denominator { 1 }; denominator <= 12; ++denominator)
   {
      for (int32_t answer { }; answer <= 12; ++answer)
      {
         randomizers_.division_problems.emplace_back(
            Problem {
               QString::number(denominator * answer),
               "\u00F7  " + QString::number(denominator),
               QString { },
               answer
            });
      }
   }

   std::shuffle(
      randomizers_.division_problems.begin(),
      randomizers_.division_problems.end(),
      std::default_random_engine {
         randomizers_.random_engine });
}

void MathFactsWidget::GradeAnswer( ) noexcept
{
   const int32_t response =
      current_problem_.line3.toInt();

   current_problem_.responses.push_back(
      response);

   if (response == current_problem_.answer)
   {
      current_problem_.end =
         std::chrono::steady_clock::now();

      answered_problems_.emplace_back(
         std::move(current_problem_));

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

void MathFactsWidget::WriteReport( ) const noexcept
{
   auto report_directory =
      GetReportsDirectory();

   const bool can_write_report =
      !std::filesystem::exists(report_directory) ||
      std::filesystem::is_directory(report_directory);

   if (!can_write_report)
   {
      QMessageBox::critical(
         nullptr,
         "Write Report Error",
         QString {
            "Cannot write to report directory.  The directory "
            "'{}' already exists and is not a directory." }
               .arg(QString::fromStdString(report_directory.string())),
         QMessageBox::StandardButton::Ok);
   }
   else
   {
      if (!std::filesystem::exists(report_directory))
      {
         std::error_code create_dir_error { };
         
         const bool directory_created =
            std::filesystem::create_directory(
               report_directory,
               create_dir_error);

         if (!directory_created)
         {
            QMessageBox::critical(
               nullptr,
               "Write Report Error",
               QString {
                  "Cannot write to report directory.  The directory "
                  "'{}' could not be created (EC: {})." }
                     .arg(QString::fromStdString(report_directory.string()))
                     .arg(QString::fromStdString(create_dir_error.message())),
               QMessageBox::StandardButton::Ok);
         }
      }

      const auto report_filepath =
         report_directory /
         (GenerateReportName() +
          ".txt");

      std::ofstream report_file {
         report_filepath,
         std::ios_base::out
      };

      if (!report_file.is_open())
      {
         QMessageBox::critical(
            nullptr,
            "Write Report Error",
            QString {
               "Cannot write to report file.  The file "
               "'{}' could not be opened for writing." }
                  .arg(QString::fromStdString(report_filepath.string())),
            QMessageBox::StandardButton::Ok);
      }
      else
      {
         std::vector< const Problem * > answered_problems_for_sort;

         answered_problems_for_sort.reserve(
            answered_problems_.size());

         qreal percentage_answers_correct { };
         std::chrono::milliseconds average_response_time { };

         for (const auto & answer : answered_problems_)
         {
            average_response_time +=
               std::chrono::duration_cast<
                  std::chrono::milliseconds >(
                     answer.end - answer.start);

            answered_problems_for_sort.push_back(
               &answer);

            if (answer.responses.size() == 1)
            {
               ++percentage_answers_correct;
            }
         }

         if (!answered_problems_.empty())
         {
            average_response_time /= answered_problems_.size();
            percentage_answers_correct /= answered_problems_.size();
         }

         report_file
            << "duration = "
            << GetMathPracticeDuration()
            << "\n";

         report_file
            << "total problems answered = "
            << answered_problems_.size()
            << "\n";

         report_file
            << "average response time = "
            << average_response_time
            << "\n";

         report_file
            << "standard deviation response time = "
            << CalculateStandardDeviationResponseTime()
            << "\n";

         report_file
            << "percentage correct = "
            << percentage_answers_correct
            << "\n";

         const auto PrintAnswer =
            [ & ] (
               const Problem & answer )
            {
               const auto response_time =
                  std::chrono::duration_cast<
                     std::chrono::milliseconds >(
                        answer.end - answer.start);

               report_file
                  << answer.line1.toStdString()
                  << answer.line2.toStdString()
                  << " = "
                  << answer.line3.toStdString()
                  << "; response time ms = "
                  << response_time
                  << "; responses = ";

               for (const auto & response : answer.responses)
               {
                  report_file
                     << response
                     << "   ";
               }

               report_file << "\n";
            };

         report_file
            << "\ntop ten most responses\n";

         std::sort(
            answered_problems_for_sort.begin(),
            answered_problems_for_sort.end(),
            [ ] (
               const Problem * const l,
               const Problem * const r )
            {
               return
                  r->responses.size() <
                  l->responses.size();
            });

         std::for_each(
            answered_problems_for_sort.begin(),
            answered_problems_for_sort.size() >= 10 ?
               answered_problems_for_sort.begin() + 10 :
               answered_problems_for_sort.end(),
            [ & ] (
               const Problem * const problem )
            {
               PrintAnswer(
                  *problem);
            });

         report_file
            << "\ntop ten longest responses\n";

         std::sort(
            answered_problems_for_sort.begin(),
            answered_problems_for_sort.end(),
            [ ] (
               const Problem * const l,
               const Problem * const r )
            {
               const auto response_time_l =
                  l->end - l->start;
               const auto response_time_r =
                  r->end - r->start;

               return
                  response_time_r <
                  response_time_l;
            });

         std::for_each(
            answered_problems_for_sort.begin(),
            answered_problems_for_sort.size() >= 10 ?
               answered_problems_for_sort.begin() + 10 :
               answered_problems_for_sort.end(),
            [ & ] (
               const Problem * const problem )
            {
               PrintAnswer(
                  *problem);
            });

         report_file
            << "\nall answers\n";

         for (const auto & answer : answered_problems_)
         {
            PrintAnswer(
               answer);
         }
      }
   }
}

std::string MathFactsWidget::GetCurrentUserName( ) const noexcept
{
#if _WIN32
   return
      std::getenv("USERNAME");
#elif __linux__ || __unix__
   return
      std::getenv("USER");
#else
#  error "Define for this platform!"
#endif
}

std::string MathFactsWidget::GenerateReportName( ) const noexcept
{
   const std::string username =
      GetCurrentUserName();

#if __has_include(<format>)
   // may throw but lets assume not
   const std::string date_time =
      std::format(
         "{:%F-%H.%M.%OS}",
         std::chrono::current_zone()->to_local(
            std::chrono::system_clock::now()));
#else
   const auto sys_time_now =
      std::chrono::system_clock::to_time_t(
         std::chrono::system_clock::now());

   std::string date_time;
   date_time.resize(512);

   std::tm local_time_now;

   // assume no failures for both
#ifdef _WIN32
   localtime_s(
      &local_time_now,
      &sys_time_now);
#else
   localtime_r(
      &sys_time_now,
      &local_time_now);
#endif // _WIN32

   const size_t bytes_written =
      std::strftime(
         date_time.data(),
         date_time.size(),
         "%F-%H.%M.%S",
         &local_time_now);

   date_time.resize(
      bytes_written);
#endif // __has_include(<format>)

   return
      username +
      "-" +
      date_time;
}

std::unique_ptr< QSettings > MathFactsWidget::GetSettings( ) const noexcept
{
   std::unique_ptr< QSettings > settings;

   const std::string username =
      GetCurrentUserName();

   const std::filesystem::path application_dir_path =
      QApplication::applicationDirPath().toStdString();

#if _MSC_VER

   const std::filesystem::path current_working_dir_path =
      std::filesystem::current_path();

   if (const auto settings_path =
          current_working_dir_path / (username + "-math-facts.ini");
       std::filesystem::is_regular_file(settings_path))
   {
      settings =
         std::make_unique< QSettings >(
            QString::fromStdString(
               settings_path.string()),
            QSettings::Format::IniFormat);
   }
   else if (const auto settings_path =
               current_working_dir_path / ("math-facts.ini");
            std::filesystem::is_regular_file(settings_path))
   {
      settings =
         std::make_unique< QSettings >(
            QString::fromStdString(
               settings_path.string()),
            QSettings::Format::IniFormat);
   }
   else
   
#endif // _MSC_VER

   if (const auto settings_path =
          application_dir_path / (username + "-math-facts.ini");
      std::filesystem::is_regular_file(settings_path))
   {
      settings =
         std::make_unique< QSettings >(
            QString::fromStdString(
               settings_path.string()),
            QSettings::Format::IniFormat);
   }
   else if (const auto settings_path =
               application_dir_path / (username + "-math-facts.ini");
            std::filesystem::is_regular_file(settings_path))
   {
      settings =
         std::make_unique< QSettings >(
            QString::fromStdString(
               settings_path.string()),
            QSettings::Format::IniFormat);
   }
   else
   {
      settings =
         std::make_unique< QSettings >(
            "math-facts.ini",
            QSettings::Format::IniFormat);
   }

   return
      settings;
}

uint32_t MathFactsWidget::GetEnabledMathFacts( ) const noexcept
{
   uint32_t enabled_math_facts {
      EnabledMathFactBits::ADD |
      EnabledMathFactBits::SUB |
      EnabledMathFactBits::MUL |
      EnabledMathFactBits::DIV
   };

   const auto settings =
      GetSettings();

   enabled_math_facts =
      settings->value(
         "enabled_math_facts",
         enabled_math_facts).toString().toInt(
            nullptr,
            16);

   return
      enabled_math_facts;
}

std::filesystem::path MathFactsWidget::GetReportsDirectory( ) const noexcept
{
   std::string directory { "../math-facts-reports/" };

   const auto settings =
      GetSettings();

   directory =
      settings->value(
         "reports_directory",
         QString::fromStdString(directory))
            .toString()
            .toStdString();

   return
      directory;
}

std::chrono::milliseconds MathFactsWidget::GetMathPracticeDuration( ) const noexcept
{
   int32_t duration { 300000 };

   const auto settings =
      GetSettings();

   duration =
      settings->value(
         "math_practice_duration_ms",
         duration).toInt();

   if (duration <= 0)
   {
      duration = 300000;
   }

   return
      std::chrono::milliseconds {
         duration
      };
}

std::chrono::milliseconds MathFactsWidget::CalculateStandardDeviationResponseTime( ) const noexcept
{
   std::chrono::milliseconds standard_deviation { };

   if (answered_problems_.size() > 1)
   {
      std::chrono::milliseconds average_response_time { };

      for (const auto & answer : answered_problems_)
      {
         average_response_time +=
            std::chrono::duration_cast<
               std::chrono::milliseconds >(
                  answer.end - answer.start);
      }
      
      average_response_time /= answered_problems_.size();

      std::chrono::milliseconds::rep sum_of_squares { };

      for (const auto & answer : answered_problems_)
      {
         const auto deviation_from_mean =
            (std::chrono::duration_cast<
               std::chrono::milliseconds >(
                  answer.end - answer.start) -
             average_response_time).count();

         const auto deviation_from_mean_squared =
            deviation_from_mean * deviation_from_mean;

         sum_of_squares +=
            deviation_from_mean_squared;
      }

      const auto variance =
         sum_of_squares /
         (answered_problems_.size() - 1.0);

      standard_deviation =
         std::chrono::milliseconds {
            static_cast< std::chrono::milliseconds::rep >(
               std::sqrt(variance))
         };
   }

   return
      standard_deviation;
}

uint32_t MathFactsWidget::GetMinimumAmountToPractice( ) const noexcept
{
   int32_t minimum { 50 };

   const auto settings =
      GetSettings();

   minimum =
      settings->value(
         "minimum_amount_to_practice",
         minimum).toInt();

   if (minimum <= 0)
   {
      minimum = 50;
   }

   return
      minimum;
}

void MathFactsWidget::PaintProblem(
   QPaintEvent * paint_event ) noexcept
{
   if (!practice_stopwatch_.PracticeTimeExceeded() ||
       answered_problems_.size() < minimum_amount_to_practice_)
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
   else
   {
      QMessageBox::information(
         this,
         "Time Expired",
         "The time allotted has expired.",
         QMessageBox::StandardButton::Ok);

      WriteReport();

      QApplication::exit(0);
   }
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
      remaining_time.count() >= 0 ?
         remaining_time.count() * 360.0 / total_time.count() :
         0.0;

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
