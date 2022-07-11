#include "FmDemodulator.h"

#define DurationMs(end, begin) std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()

FmDemodulator::FmDemodulator(std::function<void(const DataBuffer<int16_t> &)> demodCallback, int sampleRate,
                             int audioSampleRate, float gain)
    : demodCallback(demodCallback),
      sampleRate(sampleRate),
      audioSampleRate(audioSampleRate),
      digitalGain(gain),
      filterMtx(),
      lowPass(filterMtx, 100000, 8192, sampleRate),
      audioLowPass(filterMtx, 20000, 4096, FM_DOWNSAMPLED),
      sdrTransformPool(&FmDemodulator::transformExecutor, this),
      filterPool(&FmDemodulator::filterExecutor, this),
      demodPool(&FmDemodulator::demodExecutor, this)
{
}

FmDemodulator::~FmDemodulator()
{
}

void FmDemodulator::demodulate(const DataBuffer<uint8_t> &buffer, size_t count)
{
    sdrTransformPool.process(DataBuffer<uint8_t>(buffer.get(), count));
}

void FmDemodulator::demodulate(DataBuffer<uint8_t> &&buffer)
{
    sdrTransformPool.process(std::move(buffer));
}

void FmDemodulator::transformExecutor(DataBuffer<uint8_t> &data, void *arg)
{
    FmDemodulator *_this = reinterpret_cast<FmDemodulator *>(arg);

    // Transform data by subtracting 128 (ADC middle point) and converting to std::complex
    DataBuffer<Complex> tfData(data.size() / 2);
    for (size_t i = 0; i < data.size(); i += 2)
    {
        double re = (data[i] - 128.0);
        double im = (data[i + 1] - 128.0);
        tfData[i / 2] = {re, im};
    }

    _this->filterPool.process(tfData);
}

void FmDemodulator::filterExecutor(DataBuffer<Complex> &data, void *arg)
{
    FmDemodulator *_this = reinterpret_cast<FmDemodulator *>(arg);

    // Lowpass 100kHz
    _this->lowPass.filter(data);

    _this->demodPool.process(data);
}

void FmDemodulator::demodExecutor(DataBuffer<Complex> &data, void *arg)
{
    FmDemodulator *_this = reinterpret_cast<FmDemodulator *>(arg);

    std::unique_lock<std::mutex> srLock(_this->sampleRateMtx);
    int sRate = _this->sampleRate;
    srLock.unlock();

    std::unique_lock<std::mutex> gainLock(_this->dGainMtx);
    float dGain = _this->digitalGain;
    gainLock.unlock();

    size_t downsampledSize = data.size() / (sRate / FM_DOWNSAMPLED);
    size_t outputSize = (downsampledSize - 1) / (FM_DOWNSAMPLED / _this->audioSampleRate);

    DataBuffer<double> demodulatedBuffer(downsampledSize - 1);
    DataBuffer<int16_t> audioBuffer(outputSize);

    DownsampledBufferAccessor<Complex> dataDs(data, sRate, FM_DOWNSAMPLED);
    for (size_t i = 1; i < dataDs.size(); i++)
    {
        demodulatedBuffer[i - 1] =
            (dataDs[i].re * (dataDs[i].im - dataDs[i - 1].im) - dataDs[i].im * (dataDs[i].re - dataDs[i - 1].re)) / (sqr(dataDs[i].re) + sqr(dataDs[i].im));
    }

    // Lowpass 20kHz
    _this->audioLowPass.filter(demodulatedBuffer);

    // Downsample to 44.1kHz sample rate
    DownsampledBufferAccessor<double> demodDs(demodulatedBuffer, FM_DOWNSAMPLED, _this->audioSampleRate);
    for (int i = 0; i < demodDs.size(); i++)
    {
        audioBuffer[i] = FmDemodulator::coerceToInt16(demodDs[i] * dGain);
    }

    _this->demodCallback(audioBuffer);
}

void FmDemodulator::setSampleRate(int sampleRate) {
    demodPool.clear();
    std::lock_guard<std::mutex> lock(sampleRateMtx);
    this->sampleRate = sampleRate;
}

void FmDemodulator::setDigitalGain(float gain) {
    std::lock_guard<std::mutex> lock(dGainMtx);
    this->digitalGain = gain;
}

int FmDemodulator::getSampleRate() const {
    return this->sampleRate;
}

float FmDemodulator::getDigitalGain() const {
    return this->digitalGain;
}