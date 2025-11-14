#include "../include/fixed_income_pricer.hpp"
#include <algorithm>
#include <cmath>

namespace pricing {

// YieldCurve Implementation

double YieldCurve::interpolate(double maturity) const {
    // Handle edge cases
    if (maturity <= curve_.front().maturity) {
        return curve_.front().rate;
    }
    if (maturity >= curve_.back().maturity) {
        return curve_.back().rate;
    }

    // Find the two points to interpolate between
    auto it = std::lower_bound(curve_.begin(), curve_.end(), maturity,
        [](const YieldCurvePoint& p, double m) { return p.maturity < m; });

    if (it == curve_.end()) {
        return curve_.back().rate;
    }
    if (it == curve_.begin()) {
        return curve_.front().rate;
    }

    // Linear interpolation
    const auto& p2 = *it;
    const auto& p1 = *(it - 1);

    double weight = (maturity - p1.maturity) / (p2.maturity - p1.maturity);
    return p1.rate + weight * (p2.rate - p1.rate);
}

double YieldCurve::get_rate(double maturity) const {
    return interpolate(maturity);
}

double YieldCurve::get_discount_factor(double time) const {
    if (time <= 0) return 1.0;

    double rate = get_rate(time);

    switch (compounding_) {
        case CompoundingFrequency::CONTINUOUS:
            return std::exp(-rate * time);

        case CompoundingFrequency::ANNUAL:
            return std::pow(1.0 + rate, -time);

        case CompoundingFrequency::SEMI_ANNUAL:
            return std::pow(1.0 + rate / 2.0, -2.0 * time);

        case CompoundingFrequency::QUARTERLY:
            return std::pow(1.0 + rate / 4.0, -4.0 * time);

        case CompoundingFrequency::MONTHLY:
            return std::pow(1.0 + rate / 12.0, -12.0 * time);

        default:
            return std::exp(-rate * time);
    }
}

double YieldCurve::get_forward_rate(double t1, double t2) const {
    if (t1 >= t2) {
        throw std::invalid_argument("t1 must be less than t2");
    }

    double df1 = get_discount_factor(t1);
    double df2 = get_discount_factor(t2);

    // Forward rate for continuous compounding
    return -std::log(df2 / df1) / (t2 - t1);
}

YieldCurve YieldCurve::bootstrap(const std::vector<double>& maturities,
                                 const std::vector<double>& coupon_rates,
                                 const std::vector<double>& prices,
                                 double face_value) {
    if (maturities.size() != coupon_rates.size() ||
        maturities.size() != prices.size()) {
        throw std::invalid_argument("Input vectors must have the same size");
    }

    std::vector<YieldCurvePoint> curve_points;

    // Bootstrap each maturity point
    for (size_t i = 0; i < maturities.size(); ++i) {
        double maturity = maturities[i];
        double coupon_rate = coupon_rates[i];
        double price = prices[i];

        // For zero coupon bonds
        if (coupon_rate == 0) {
            double rate = -std::log(price / face_value) / maturity;
            curve_points.emplace_back(maturity, rate);
            continue;
        }

        // For coupon bonds, need to solve for the rate
        // This is a simplified Newton-Raphson approach
        double rate = 0.05;  // Initial guess
        double tolerance = 1e-8;
        int max_iter = 100;

        for (int iter = 0; iter < max_iter; ++iter) {
            double calc_price = 0.0;
            double deriv = 0.0;

            // Calculate price and derivative using known rates
            YieldCurve temp_curve(curve_points.empty() ?
                std::vector<YieldCurvePoint>{{0.01, 0.05}} : curve_points);

            for (double t = 1.0; t <= maturity; t += 1.0) {
                double coupon = face_value * coupon_rate;
                double df;

                if (t < maturity) {
                    df = temp_curve.get_discount_factor(t);
                } else {
                    df = std::exp(-rate * t);
                    deriv -= t * df * (coupon + face_value);
                }
                calc_price += coupon * df;
            }
            calc_price += face_value * std::exp(-rate * maturity);

            double diff = calc_price - price;
            if (std::abs(diff) < tolerance) {
                curve_points.emplace_back(maturity, rate);
                break;
            }

            if (std::abs(deriv) < 1e-10) {
                throw std::runtime_error("Bootstrap failed: derivative too small");
            }

            rate = rate - diff / deriv;
        }
    }

    return YieldCurve(curve_points, CompoundingFrequency::CONTINUOUS);
}

// ZeroCouponBond Implementation

double ZeroCouponBond::price() const {
    return face_value_ * yield_curve_.get_discount_factor(maturity_);
}

double ZeroCouponBond::yield_to_maturity(double price) const {
    return -std::log(price / face_value_) / maturity_;
}

double ZeroCouponBond::duration() const {
    return maturity_;  // Duration equals maturity for zero coupon bonds
}

double ZeroCouponBond::convexity() const {
    return maturity_ * maturity_;
}

// CouponBond Implementation

void CouponBond::generate_cash_flows() {
    cash_flows_.clear();

    int periods_per_year;
    switch (payment_frequency_) {
        case CompoundingFrequency::ANNUAL:
            periods_per_year = 1;
            break;
        case CompoundingFrequency::SEMI_ANNUAL:
            periods_per_year = 2;
            break;
        case CompoundingFrequency::QUARTERLY:
            periods_per_year = 4;
            break;
        case CompoundingFrequency::MONTHLY:
            periods_per_year = 12;
            break;
        default:
            periods_per_year = 2;  // Default to semi-annual
    }

    double period_length = 1.0 / periods_per_year;
    double coupon_payment = face_value_ * coupon_rate_ / periods_per_year;

    // Generate coupon payments
    for (double t = period_length; t < maturity_; t += period_length) {
        cash_flows_.emplace_back(coupon_payment, t);
    }

    // Final payment (coupon + principal)
    cash_flows_.emplace_back(coupon_payment + face_value_, maturity_);
}

double CouponBond::price() const {
    double pv = 0.0;
    for (const auto& cf : cash_flows_) {
        pv += cf.amount * yield_curve_.get_discount_factor(cf.time);
    }
    return pv;
}

double CouponBond::yield_to_maturity(double price) const {
    // Newton-Raphson method to solve for YTM
    double ytm = 0.05;  // Initial guess
    double tolerance = 1e-8;
    int max_iter = 100;

    for (int iter = 0; iter < max_iter; ++iter) {
        double calc_price = 0.0;
        double deriv = 0.0;

        for (const auto& cf : cash_flows_) {
            double df = std::exp(-ytm * cf.time);
            calc_price += cf.amount * df;
            deriv -= cf.amount * cf.time * df;
        }

        double diff = calc_price - price;
        if (std::abs(diff) < tolerance) {
            return ytm;
        }

        if (std::abs(deriv) < 1e-10) {
            throw std::runtime_error("YTM calculation failed: derivative too small");
        }

        ytm = ytm - diff / deriv;
    }

    throw std::runtime_error("YTM did not converge");
}

double CouponBond::macaulay_duration() const {
    double bond_price = price();
    double weighted_sum = 0.0;

    for (const auto& cf : cash_flows_) {
        double pv = cf.amount * yield_curve_.get_discount_factor(cf.time);
        weighted_sum += cf.time * pv;
    }

    return weighted_sum / bond_price;
}

double CouponBond::modified_duration() const {
    double ytm = yield_to_maturity(price());
    double mac_duration = macaulay_duration();

    int periods_per_year;
    switch (payment_frequency_) {
        case CompoundingFrequency::ANNUAL:
            periods_per_year = 1;
            break;
        case CompoundingFrequency::SEMI_ANNUAL:
            periods_per_year = 2;
            break;
        case CompoundingFrequency::QUARTERLY:
            periods_per_year = 4;
            break;
        case CompoundingFrequency::MONTHLY:
            periods_per_year = 12;
            break;
        default:
            periods_per_year = 2;
    }

    return mac_duration / (1.0 + ytm / periods_per_year);
}

double CouponBond::duration() const {
    return modified_duration();
}

double CouponBond::convexity() const {
    double bond_price = price();
    double weighted_sum = 0.0;

    for (const auto& cf : cash_flows_) {
        double pv = cf.amount * yield_curve_.get_discount_factor(cf.time);
        weighted_sum += cf.time * cf.time * pv;
    }

    return weighted_sum / bond_price;
}

// InterestRateSwap Implementation

double InterestRateSwap::present_value() const {
    int periods_per_year;
    switch (payment_frequency_) {
        case CompoundingFrequency::ANNUAL:
            periods_per_year = 1;
            break;
        case CompoundingFrequency::SEMI_ANNUAL:
            periods_per_year = 2;
            break;
        case CompoundingFrequency::QUARTERLY:
            periods_per_year = 4;
            break;
        case CompoundingFrequency::MONTHLY:
            periods_per_year = 12;
            break;
        default:
            periods_per_year = 4;
    }

    double period_length = 1.0 / periods_per_year;
    double pv_fixed = 0.0;
    double pv_floating = 0.0;

    // Present value of fixed leg
    for (double t = period_length; t <= maturity_; t += period_length) {
        double fixed_payment = notional_ * fixed_rate_ * period_length;
        pv_fixed += fixed_payment * yield_curve_.get_discount_factor(t);
    }

    // Present value of floating leg
    // For a standard swap, PV(floating) = notional * (1 - discount_factor(T))
    pv_floating = notional_ * (1.0 - yield_curve_.get_discount_factor(maturity_));

    // Swap value is PV(floating) - PV(fixed) for the receiver
    return pv_floating - pv_fixed;
}

double InterestRateSwap::fair_swap_rate() const {
    int periods_per_year;
    switch (payment_frequency_) {
        case CompoundingFrequency::ANNUAL:
            periods_per_year = 1;
            break;
        case CompoundingFrequency::SEMI_ANNUAL:
            periods_per_year = 2;
            break;
        case CompoundingFrequency::QUARTERLY:
            periods_per_year = 4;
            break;
        case CompoundingFrequency::MONTHLY:
            periods_per_year = 12;
            break;
        default:
            periods_per_year = 4;
    }

    double period_length = 1.0 / periods_per_year;
    double annuity = 0.0;

    // Calculate annuity factor
    for (double t = period_length; t <= maturity_; t += period_length) {
        annuity += period_length * yield_curve_.get_discount_factor(t);
    }

    // Fair swap rate
    return (1.0 - yield_curve_.get_discount_factor(maturity_)) / annuity;
}

double InterestRateSwap::duration() const {
    double swap_pv = present_value();
    if (std::abs(swap_pv) < 1e-10) {
        return 0.0;
    }

    // Bump the yield curve and recalculate
    // This is simplified - in practice, you'd bump each point
    double h = 0.0001;  // 1 basis point

    // Create a copy of the yield curve with bumped rates
    // This is a simplified approach
    return maturity_ / 2.0;  // Approximate duration
}

double InterestRateSwap::dv01() const {
    // DV01 is the change in value for a 1 basis point change in rates
    return duration() * notional_ * 0.0001;
}

// Utility functions

namespace utils {

double year_fraction(int days, DayCountConvention convention) {
    switch (convention) {
        case DayCountConvention::ACT_360:
            return days / 360.0;
        case DayCountConvention::ACT_365:
            return days / 365.0;
        case DayCountConvention::THIRTY_360:
            return days / 360.0;  // Simplified
        default:
            return days / 365.0;
    }
}

double convert_rate(double rate, CompoundingFrequency from,
                   CompoundingFrequency to, double periods) {
    // Convert to continuous compounding first
    double continuous_rate;

    switch (from) {
        case CompoundingFrequency::CONTINUOUS:
            continuous_rate = rate;
            break;
        case CompoundingFrequency::ANNUAL:
            continuous_rate = std::log(1.0 + rate);
            break;
        case CompoundingFrequency::SEMI_ANNUAL:
            continuous_rate = 2.0 * std::log(1.0 + rate / 2.0);
            break;
        case CompoundingFrequency::QUARTERLY:
            continuous_rate = 4.0 * std::log(1.0 + rate / 4.0);
            break;
        case CompoundingFrequency::MONTHLY:
            continuous_rate = 12.0 * std::log(1.0 + rate / 12.0);
            break;
        default:
            continuous_rate = rate;
    }

    // Convert from continuous to target frequency
    switch (to) {
        case CompoundingFrequency::CONTINUOUS:
            return continuous_rate;
        case CompoundingFrequency::ANNUAL:
            return std::exp(continuous_rate) - 1.0;
        case CompoundingFrequency::SEMI_ANNUAL:
            return 2.0 * (std::exp(continuous_rate / 2.0) - 1.0);
        case CompoundingFrequency::QUARTERLY:
            return 4.0 * (std::exp(continuous_rate / 4.0) - 1.0);
        case CompoundingFrequency::MONTHLY:
            return 12.0 * (std::exp(continuous_rate / 12.0) - 1.0);
        default:
            return continuous_rate;
    }
}

double accrued_interest(double face_value, double coupon_rate,
                       int days_since_last_payment,
                       DayCountConvention convention) {
    double year_frac = year_fraction(days_since_last_payment, convention);
    return face_value * coupon_rate * year_frac;
}

} // namespace utils

} // namespace pricing
