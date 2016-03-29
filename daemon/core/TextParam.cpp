/*
	Copyright (c) 2000-2016 Michael Pozhidaev<michael.pozhidaev@gmail.com>
   This file is part of the VoiceMan speech service.

   VoiceMan speech service is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   VoiceMan speech service is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/

#include"voiceman.h"
#include"TextParam.h"

static double mapDoubleValue(size_t value, double min, double max)
{
  const double d = max - min;
  return min + (d * ((double)value / 50));
}

TextParam::TextParam(size_t value)
{
  m_value = value <= 100?value:100;
}

void TextParam::reset()
{
  m_value = 50;
}

size_t TextParam::getValue()
{
  return m_value;
}

double TextParam::getValue(double min, double aver, double max) const
{
  assert(m_value >= 0 && m_value <= 100);
  if (m_value == 0)
    return min;
  if (m_value >= 100)
    return max;
  if (m_value == 50)
    return aver;
  if (m_value < 50)
    return mapDoubleValue(m_value, min, aver); else
    return mapDoubleValue(m_value - 50, aver, max);
}

const TextParam& TextParam::operator +=(size_t step)
{
  if (step >= 100 || m_value + step >= 100)
    m_value = 100;
  else
    m_value += step;
  return *this;
}

const TextParam& TextParam::operator -=(size_t step)
{
  if (step >= m_value)
    m_value = 0; else
    m_value -= step;
  return *this;
}

const TextParam& TextParam::operator =(size_t value)
{
  m_value = value <= 100?value:100;
  return *this;
}
