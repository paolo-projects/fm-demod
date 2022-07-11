#pragma once

#include <stdlib.h>
#include <array>
#include <algorithm>
#include <vector>
#include <limits>
#include <list>
#include <fftw3.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <memory>
#include "Complex.h"
#include "LowPass.h"
#include "DataProcessingThreadPool.h"
#include "DataBuffer.h"
#include "DownsampledBufferAccessor.h"
#include "Math.h"

class FmDemodulator
{
public:
    FmDemodulator(const FmDemodulator &) = delete;
    /**
     * @brief Construct a new Demodulator object
     *
     * @param sampleRate SDR Sample rate
     * @param audioSampleRate Output audio sample rate
     */
    FmDemodulator(std::function<void(const DataBuffer<int16_t> &)> demodCallback,
                  int sampleRate, int audioSampleRate, unsigned int sdrBufferSize, float gain = 1.0f);

    ~FmDemodulator();

    void demodulate(const DataBuffer<uint8_t> &buffer, size_t count);

private:
    static constexpr int FM_DOWNSAMPLED = 220500;
    static constexpr int TRDPOOL_SZ = 1;

    std::mutex filterMtx;
    LowPass<Complex> lowPass;
    LowPass<double> audioLowPass;

    int sampleRate, audioSampleRate;
    int sdrBufferSize;
    float digitalGain;
    bool transformRunning = true, filterRunning = true, demodRunning = true;
    size_t downsampledSize;
    size_t outputSize;

    std::function<void(const DataBuffer<int16_t> &)> demodCallback;

    DataProcessingThreadPool<DataBuffer<uint8_t>, TRDPOOL_SZ> sdrTransformPool;
    DataProcessingThreadPool<DataBuffer<Complex>, TRDPOOL_SZ> filterPool;
    DataProcessingThreadPool<DataBuffer<Complex>, TRDPOOL_SZ> demodPool;

    static void transformExecutor(DataBuffer<uint8_t> &data, void *arg);
    static void filterExecutor(DataBuffer<Complex> &data, void *arg);
    static void demodExecutor(DataBuffer<Complex> &data, void *arg);

    template <typename T>
    static inline int16_t coerceToInt16(T value)
    {
        static constexpr T min = std::numeric_limits<int16_t>::min();
        static constexpr T max = std::numeric_limits<int16_t>::max();
        return (int16_t)std::max(min, std::min(max, value));
    }
};
