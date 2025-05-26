#include "math-facts-widget.hpp"

#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>

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
