#include "math-facts-widget.hpp"
#include "arithmetic-problem.hpp"
#include "problem.hpp"

#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QOverload>
#include <QtCore/QPointF>
#include <QtCore/QRect>
#include <QtCore/QRectF>
#include <QtCore/QSettings>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtCore/Qt>
#include <QtCore/QtTypes>
#include <QtCore/QVector>
#include <QtGui/QBrush>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QTextOption>
#include <QtUiTools/QUiLoader>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <ios>
#include <iterator>
#include <system_error>
#include <utility>

#if __has_include(<format>)
#  include <format>
#else
#  include <ctime>
#  include <time.h>
#endif // __has_include(<format>)

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

MathFactsWidget::~MathFactsWidget( ) noexcept
{
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

   current_problem_->SetTextColor(
      current_colors_->text);

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
   if (current_problem_)
   {
      current_problem_->OnKeyReleaseEvent(
         event,
         *this);
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
         { "pushButtonClock", ":/math-button-image-clock", TitleButtonID::CLOCK, EnabledMathFactBits::CLOCK },
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

std::unique_ptr< Problem > MathFactsWidget::GenerateProblem( ) noexcept
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

   std::vector< std::unique_ptr< Problem > > * const problems[] {
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

   std::unique_ptr< Problem > problem {
      std::move(problems[problem_type]->back())
   };

   problems[problem_type]->pop_back();

   problem->SetStartTime(
      std::chrono::steady_clock::now());

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
            std::make_unique< ArithmeticProblem >(
               top,
               bottom,
               ArithmeticProblem::Operation::ADD));

         QObject::connect(
            randomizers_.addition_problems.back().get(),
            &Problem::Answered,
            this,
            &MathFactsWidget::OnProblemAnswered);
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
               std::make_unique< ArithmeticProblem >(
                  top,
                  bottom,
                  ArithmeticProblem::Operation::ADD));

            QObject::connect(
               randomizers_.subtraction_problems.back().get(),
               &Problem::Answered,
               this,
               &MathFactsWidget::OnProblemAnswered);
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
            std::make_unique< ArithmeticProblem >(
               top,
               bottom,
               ArithmeticProblem::Operation::MUL));

         QObject::connect(
            randomizers_.multiplication_problems.back().get(),
            &Problem::Answered,
            this,
            &MathFactsWidget::OnProblemAnswered);
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
            std::make_unique< ArithmeticProblem >(
               denominator * answer,
               denominator,
               ArithmeticProblem::Operation::DIV));

         QObject::connect(
            randomizers_.division_problems.back().get(),
            &Problem::Answered,
            this,
            &MathFactsWidget::OnProblemAnswered);
      }
   }

   std::shuffle(
      randomizers_.division_problems.begin(),
      randomizers_.division_problems.end(),
      std::default_random_engine {
         randomizers_.random_engine });
}

void MathFactsWidget::OnProblemAnswered(
   const AnswerResult result ) noexcept
{
   if (result == AnswerResult::CORRECT)
   {
      current_problem_->SetEndTime(
         std::chrono::steady_clock::now());

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

      current_problem_->SetTextColor(
         current_colors_->text);

      answer_image_ = &correct_answer_image_;
   }
   else
   {
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
                     answer->GetResponseTime());

            answered_problems_for_sort.push_back(
               answer.get());

            if (answer->GetNumberOfResponses() == 1)
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
                        answer.GetResponseTime());

               report_file
                  << answer.GetQuestionWithAnswer().toStdString()
                  << "; response time ms = "
                  << response_time
                  << "; responses = ";

               for (const auto & response : answer.GetResponses())
               {
                  report_file
                     << response.toStdString()
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
                  r->GetNumberOfResponses() <
                  l->GetNumberOfResponses();
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
                  l->GetResponseTime();
               const auto response_time_r =
                  r->GetResponseTime();

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
               *answer);
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
      EnabledMathFactBits::DIV |
      EnabledMathFactBits::CLOCK
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
                  answer->GetResponseTime());
      }
      
      average_response_time /= answered_problems_.size();

      std::chrono::milliseconds::rep sum_of_squares { };

      for (const auto & answer : answered_problems_)
      {
         const auto deviation_from_mean =
            (std::chrono::duration_cast<
               std::chrono::milliseconds >(
                  answer->GetResponseTime()) -
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
   if (current_problem_)
   {
      current_problem_->OnPaintEvent(
         paint_event,
         *this);
   }
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