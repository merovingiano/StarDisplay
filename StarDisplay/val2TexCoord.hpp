#ifndef VAL2TEXCOORD_HPP_INCLUDED
#define VAL2TEXCOORD_HPP_INCLUDED


struct val2TexCoord
{
  val2TexCoord(float minValue, float maxValue)
    :  minVal(minValue), maxVal(maxValue), scale(1.0f / (maxValue - minValue)) 
  {}

  float operator()(float value) const  { return scale * (value - minVal); }
  float inverse(float tex_u) const { return tex_u / scale + minVal; }

  const float minVal;
  const float maxVal;
  const float scale;
};


#endif


