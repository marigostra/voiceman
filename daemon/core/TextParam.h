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

#ifndef __VOICEMAN_TEXT_PARAM_H__
#define __VOICEMAN_TEXT_PARAM_H__

enum {ParamVolume = 1, ParamPitch = 2, ParamRate = 3};

/**\brief The value of speech attribute
 *
 * This class contains value of any text characteristic. Usually such
 * characteristics are a voice pitch, rate or volume. A value is stored in
 * percents and can be mapped into the provided interval. A interval can be
 * specified by minimum, average and maximum values.
 *
 * \sa TextItem
 */
class TextParam
{
public:
  /**\brief The default constructor*/
  TextParam() 
    : m_value(50) {}

  /**\brief The constructor with initial value specification
   *
   * \param [in] value The initial value for new object
   */
  TextParam(size_t value);

  /**\brief Restores default parameter value
   *
   * Use this method to set value as in just created object (usually 50).
   */
  void reset();

  /**\brief Returns native (not mapped) parameter value
   *
   * This method returns the value of parameter as it stored in the object
   * and not mapped into any interval. This value must be between 0 and
   * 100.
   *
   * \return The native parameter value
   */
  size_t getValue();

  /**\brief Returns the value mapped  into specified interval
   *
   * This method maps stored value into the specified interval. The
   * interval can be specified by three floating point values: two bounds
   * and average point. Average point can be used to shift middle of the
   * interval from its original position. It allows non-linear mapping,
   * what is often required for good adjusting of speech parameters. There
   * is no checking for relations between provided values. The middle point
   * can lay outside of minimal and maximum bounds.
   *
   * \param [in] min The low bound of the interval to map into
   * \param [in] aver The average value of the interval to map into
   * \param [in] max The upper bound of the interval to map into
   *
   * \return The mapped value
   */
  double getValue(double min, double aver, double max) const;

  /**\brief Increments current value onto specified step
   *
   * This operator increments current value onto on specified step. It
   * preserves from overflow and if step is too large maximum valid value
   * will be assumed .
   *
   * \param [in] step The step to change value for
   *
   * \return The reference to the current object
   */
  const TextParam& operator +=(size_t step);

  /**\brief Decrements current value onto specified step
   *
   * This operator decrements current value onto on specified step. It
   * preserves from overflow and if step is too large minimum valid value
   * will be assumed .
   *
   * \param [in] step The step to change value for
   *
   * \return The reference to the current object
   */
  const TextParam& operator -=(size_t step);

  /**\brief Sets new value to this parameter
   *
   * This method sets new value of this object. It contains checking to
   * validate specified argument. If received number too large the maximum
   * value is used.
   * 
   * \param [in] value The new value to set
   *
   * \return The reference to the current object
   */
  const TextParam& operator =(size_t value);

private:
  size_t m_value;
}; //class TextParam;

#endif //__VOICEMAN_TEXTPARAM_H__
