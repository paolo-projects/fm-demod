#pragma once

#include <stdlib.h>
#include <array>
#include <vector>
#include <math.h>
#include <assert.h>
#include <fftw3.h>
#include <string.h>
#include <type_traits>
#include "DataBuffer.h"

template<typename T> struct is_complex_type final : std::false_type {
};

template<> struct is_complex_type<Complex> final : std::true_type {
};

template<typename T>
class LowPass {
public:
    LowPass() = delete;
    LowPass(const LowPass &) = delete;

    LowPass(std::mutex& mtx, int frequency, int N, int sampleRate) : mtx(mtx), 
            frequency(frequency), N(N) {
        generateCoefficients(sampleRate);
        generateComplexCoefficients(sampleRate);
    }

    ~LowPass() {
    }

    template<typename Type = T>
    auto filter(DataBuffer<Complex>& data) -> typename std::enable_if<is_complex_type<Type>::value>::type {
        size_t currentIndex = 0;
        
        std::lock_guard<std::mutex> lock(mtx);
        fftw_complex* outputFft = fftw_alloc_complex(N);

        while (currentIndex + N < data.size()) {
            fftw_plan dfftPlan = fftw_plan_dft_1d(N, (fftw_complex *) &data[currentIndex], outputFft, FFTW_FORWARD, FFTW_ESTIMATE);

            fftw_execute(dfftPlan);

            // Execute convolution in frequency domain (it's a multiplication with the filter frequency response)
            for (size_t i = 0; i < N; i++) {
                outputFft[i][0] *= complexCoefficients[i];
                outputFft[i][1] *= complexCoefficients[i];
            }
            fftw_destroy_plan(dfftPlan);
            
            fftw_plan ifftPlan = fftw_plan_dft_1d(N, outputFft, (fftw_complex *) &data[currentIndex], FFTW_BACKWARD, FFTW_ESTIMATE);
            fftw_execute(ifftPlan);
            
            // At this point the filtered signal has a unwanted multiplicative factor of N
            for (size_t i = 0; i < N; i++) {
                data[currentIndex + i] /= N;
            }

            fftw_destroy_plan(ifftPlan);
            currentIndex += N;
        }

        fftw_free(outputFft);
    }

    template<typename Type = T>
    auto filter(DataBuffer<T>& data) -> typename std::enable_if<!is_complex_type<Type>::value>::type {
        size_t currentIndex = 0;
        
        std::lock_guard<std::mutex> lock(mtx);
        fftw_complex *outputFft = fftw_alloc_complex(N);

        while (currentIndex + N < data.size()) {            
            fftw_plan dfftPlan = fftw_plan_dft_r2c_1d(N, &data[currentIndex], outputFft, FFTW_ESTIMATE);
            fftw_execute(dfftPlan);

            // Execute convolution in frequency domain (it's a multiplication with the filter frequency response)
            for (size_t i = 0; i < N; i++) {
                outputFft[i][0] *= coefficients[i];
            }
            fftw_destroy_plan(dfftPlan);
            
            fftw_plan ifftPlan = fftw_plan_dft_c2r_1d(N, outputFft, &data[currentIndex], FFTW_ESTIMATE);
            fftw_execute(ifftPlan);
            // At this point the filtered signal has a unwanted multiplicative factor of N
            for (int i = 0; i < N; i++) {
                data[currentIndex + i] /= N;
            }
            fftw_destroy_plan(ifftPlan);
            
            currentIndex += N;
        }

        fftw_free(outputFft);
    }

private:
    int frequency, N;
    std::vector<double> coefficients;
    std::vector<double> complexCoefficients;
    std::mutex& mtx;

    float logistic(float f, float k, float f0) {
        return 1 - 1 / (1 + exp(-k * (f - f0)));
    }

    void generateCoefficients(int sampleRate) {
        float fftIndex = frequency * (float) N / sampleRate;
        coefficients.resize(N);
        for (int i = 0; i < N / 2; i++) {
            coefficients[i] = logistic(i, 1.0f, fftIndex);
        }
    }

    void generateComplexCoefficients(int sampleRate) {
        float fftIndex = frequency * (float) N / sampleRate;
        complexCoefficients.resize(N);
        for (int i = 0; i < N / 2; i++) {
            complexCoefficients[i] = logistic(i, 1.0f, fftIndex);
        }
        // We want this filter to be symmetrical with respect tp the negative frequencies.
        for (int i = N / 2; i < N; i++)
        {
            complexCoefficients[i] = complexCoefficients[N - i - 1];
        }
    }
};