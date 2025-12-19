#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "tim.h"

Model::Model() : modelListener(0), lastFrequencyListener(0), lastFrequencyHz(0xFFFFFFFFu), lastPeakToPeakMv(-1.0f)
{

}

void Model::tick()
{
  if (modelListener != lastFrequencyListener)
  {
    lastFrequencyListener = modelListener;
    lastFrequencyHz = 0xFFFFFFFFu;
    lastPeakToPeakMv = -1.0f;
  }

  const uint32_t measuredHz = TIM1_GetFrequencyHz();
  const float measuredVppMv = TIM1_GetPeakToPeakmV();
  static const size_t PREVIEW_POINTS = 100;
  uint16_t preview[PREVIEW_POINTS];
  const size_t previewCount = TIM1_GetWaveformPreview(preview, PREVIEW_POINTS);

  if (modelListener && measuredHz != lastFrequencyHz)
  {
    lastFrequencyHz = measuredHz;
    modelListener->NotifyFrequencyMeasured(measuredHz);
  }

  if (modelListener)
  {
    modelListener->NotifyWaveform(measuredHz, measuredVppMv, preview, previewCount);
    lastPeakToPeakMv = measuredVppMv;
  }
}
