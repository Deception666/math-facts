#include "problem.hpp"

Problem::~Problem( ) noexcept
{
}

void Problem::SetTextColor(
   const QColor & color ) noexcept
{
   text_color_ = color;
}

const QColor & Problem::GetTextColor( ) const noexcept
{
   return
      text_color_;
}

bool Problem::SetStartTime(
   const std::chrono::steady_clock::time_point start_time ) noexcept
{
   bool set { };

   if (start_time_.time_since_epoch().count() == 0 &&
       end_time_.time_since_epoch().count() == 0)
   {
      start_time_ = start_time;

      set = true;
   }

   return set;
}

bool Problem::SetEndTime(
   const std::chrono::steady_clock::time_point end_time ) noexcept
{
   bool set { };

   if (start_time_.time_since_epoch().count() != 0 &&
       end_time_.time_since_epoch().count() == 0 &&
       end_time.time_since_epoch() >= start_time_.time_since_epoch())
   {
      end_time_ = end_time;

      set = true;
   }

   return set;
}

std::chrono::steady_clock::duration Problem::GetResponseTime( ) const noexcept
{
   return
      end_time_ >= start_time_ ?
         end_time_ - start_time_ :
         std::chrono::steady_clock::duration { };
}
