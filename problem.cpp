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
