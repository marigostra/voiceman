/*
	Copyright (c) 2000-2013 Michael Pozhidaev<msp@altlinux.org>
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
#include"TextItem.h"

std::wstring TextItem::getText() const
{
  return m_text;
}

void TextItem::setText(const std::wstring& text)
{
  m_text = text;
  m_marks.clear();
}

void TextItem::resetParams()
{
  m_volume.reset();
  m_pitch.reset();
  m_rate.reset();
}

void TextItem::mark(size_t index)
{
  assert(index < m_text.length());
  ensureMarksSize(index);
  m_marks[index] = 1;
}

void TextItem::unmark(size_t index)
{
  assert(index < m_text.length());
  ensureMarksSize(index);
  m_marks[index] = 0;
}

bool TextItem::isMarked(size_t index) const
{
  if (index >= m_marks.size())
    return 0;
  return m_marks[index];
}

TextParam TextItem::getVolume() const
{
  return m_volume;
}

void TextItem::setVolume(TextParam volume)
{
  m_volume = volume;
}

TextParam TextItem::getPitch() const
{
  return m_pitch;
}

void TextItem::setPitch(TextParam pitch)
{
  m_pitch = pitch;
}

TextParam TextItem::getRate() const
{
  return m_rate;
}

void TextItem::setRate(TextParam rate)
{
  m_rate = rate;
}

std::string TextItem::getOutputName() const
{
  return m_outputName;
}

void TextItem::setOutputName(const std::string& outputName)
{
  m_outputName = outputName;
}

LangId TextItem::getLangId() const
{
  return m_langId;
}

void TextItem::setLangId(LangId langId)
{
  m_langId = langId;
}

void TextItem::ensureMarksSize(size_t index)
{
  if (index < m_marks.size())
    return;
  BoolVector::size_type oldSize = m_marks.size();
  m_marks.resize(index + 1);
  for(BoolVector::size_type i = oldSize;i < m_marks.size();i++)
    m_marks[i] = 0;
}
