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
#include"OutputSet.h"

void OutputSet::reinit(const OutputList& outputs)
{
  m_outputs.clear();
  if (outputs.empty())
    return;
  m_outputs.resize(outputs.size());
  OutputVector::size_type index = 0;
  for(OutputList::const_iterator it = outputs.begin();it != outputs.end();it++)
    {
      assert(index < m_outputs.size());
      m_outputs[index] = *it;
      index++;
    } //for();
}

bool OutputSet::hasOutput(const std::string& outputName) const
{
  for(OutputVector::size_type i = 0;i < m_outputs.size();i++)
    if (m_outputs[i].getName() == outputName)
      return 1;
  return 0;
}

std::string OutputSet::prepareSynthCommand(const std::string& outputName, const TextItem& textItem) const
{
  OutputVector::size_type i;
  for(i = 0;i < m_outputs.size();i++)
    if (m_outputs[i].getName() == outputName)
      break;
  assert(i < m_outputs.size());
  return m_outputs[i].prepareSynthCommand(textItem);
}

std::string OutputSet::preparePlayerCommand(const std::string& outputName, PlayerType playerType, const TextItem& textItem) const
{
  OutputVector::size_type i;
  for(i = 0;i < m_outputs.size();i++)
    if (m_outputs[i].getName() == outputName)
      break;
  assert(i < m_outputs.size());
  switch(playerType)
    {
    case PlayerTypeAlsa:
      return m_outputs[i].prepareAlsaPlayerCommand(textItem);
    case PlayerTypePulseaudio:
      return m_outputs[i].preparePulseaudioPlayerCommand(textItem);
    case PlayerTypePcspeaker:
      return m_outputs[i].preparePcspeakerPlayerCommand(textItem);
    } //switch();
  assert(0);
  return "";//just to reduce compilation warnings;
}

std::string OutputSet::prepareText(const std::string& outputName, const TextItem& textItem) const
{
  OutputVector::size_type i;
  for(i = 0;i < m_outputs.size();i++)
    if (m_outputs[i].getName() == outputName)
      break;
  assert(i < m_outputs.size());
  return m_outputs[i].prepareText(textItem);
}

std::string OutputSet::getOutputNameByFamilyAndLangId(const std::string& familyName, LangId langId) const
{
  for(OutputVector::size_type i = 0;i < m_outputs.size();i++)
    if (m_outputs[i].getFamily() == familyName && m_outputs[i].getLangId() == langId)
      return m_outputs[i].getName();
  assert(0);
  return "";//just to reduce compilation warnings;
}

bool OutputSet::isValidFamilyName(LangId langId, const std::string& familyName) const
{
  for(OutputVector::size_type i = 0;i < m_outputs.size();i++)
    if (m_outputs[i].getFamily() == familyName && m_outputs[i].getLangId() == langId)
      return 1;
  return 0;
}
