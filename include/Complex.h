#pragma once

#include <cmath>
#include "Math.h"

struct Complex {
    double re;
    double im;

    Complex& operator*=(double rhs) {
        this->re *= rhs;
        this->im *= rhs;
        return *this;
    }

    Complex operator*(double rhs) const {
        return Complex{
            this->re * rhs,
            this->im * rhs};
    }

    Complex& operator/=(double rhs) {
        this->re /= rhs;
        this->im /= rhs;
        return *this;
    }

    Complex operator/(double rhs) const {
        return Complex{
            this->re / rhs,
            this->im / rhs};
    }

    Complex& operator*=(const Complex& rhs) {
        double r = this->re * rhs.re - this->im * rhs.im;
        double i = this->re * rhs.im + this->im * rhs.re;
        this->re = r;
        this->im = i;
        return *this;
    }

    Complex operator*(const Complex& rhs) const {
        return Complex{
            this->re * rhs.re - this->im * rhs.im,
            this->re * rhs.im + this->im * rhs.re};
    }

    Complex& operator/=(const Complex& rhs) {
        double denom = sqr(rhs.re) + sqr(rhs.im);
        double r = (this->re * rhs.re + this->im * rhs.im) / denom;
        double i = (this->im * rhs.re - this->re * rhs.im) / denom;
        this->re = r;
        this->im = i;
        return *this;
    }

    Complex operator/(const Complex& rhs) const {
        double den = sqr(rhs.re) + sqr(rhs.im);
        return Complex{
            (this->re * rhs.re + this->im * rhs.im) / den,
            (this->im * rhs.re - this->re * rhs.im) / den};
    };

    Complex& operator+=(const Complex& rhs) {
        this->re += rhs.re;
        this->im += rhs.im;
        return *this;
    }

    Complex operator+(const Complex& rhs) const {
        return Complex{
            this->re + rhs.re,
            this->im + rhs.im};
    }

    Complex& operator-=(const Complex& rhs) {
        this->re -= rhs.re;
        this->im -= rhs.im;
        return *this;
    }

    Complex operator-(const Complex& rhs) const {
        return Complex{
            this->re - rhs.re,
            this->im - rhs.im};
    }

    Complex operator-() const {
        return Complex{
            -this->re,
            -this->im};
    }
    
    double magnitude() const {
        return sqrt(this->magnitudeSquared());
    }
    
    double magnitudeSquared() const {
        return sqr(this->re) + sqr(this->im);
    }
};