#pragma once

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <map>

namespace pricing {

// Enum for day count conventions
enum class DayCountConvention {
    ACT_360,    // Actual/360
    ACT_365,    // Actual/365
    THIRTY_360  // 30/360
};

// Enum for compounding frequency
enum class CompoundingFrequency {
    CONTINUOUS,
    ANNUAL,
    SEMI_ANNUAL,
    QUARTERLY,
    MONTHLY
};

// Structure for cash flow
struct CashFlow {
    double amount;
    double time;  // Time to cash flow in years

    CashFlow(double amt, double t) : amount(amt), time(t) {
        if (time < 0) throw std::invalid_argument("Time cannot be negative");
    }
};

// Structure for yield curve point
struct YieldCurvePoint {
    double maturity;  // Time to maturity in years
    double rate;      // Interest rate

    YieldCurvePoint(double m, double r) : maturity(m), rate(r) {
        if (maturity < 0) throw std::invalid_argument("Maturity cannot be negative");
    }

    bool operator<(const YieldCurvePoint& other) const {
        return maturity < other.maturity;
    }
};

// Yield curve class with interpolation
class YieldCurve {
private:
    std::vector<YieldCurvePoint> curve_;
    CompoundingFrequency compounding_;

    // Linear interpolation
    double interpolate(double maturity) const;

public:
    YieldCurve(const std::vector<YieldCurvePoint>& points,
              CompoundingFrequency comp = CompoundingFrequency::CONTINUOUS)
        : curve_(points), compounding_(comp) {
        if (curve_.empty())
            throw std::invalid_argument("Yield curve cannot be empty");
        std::sort(curve_.begin(), curve_.end());
    }

    // Get rate for a given maturity
    double get_rate(double maturity) const;

    // Get discount factor
    double get_discount_factor(double time) const;

    // Get forward rate
    double get_forward_rate(double t1, double t2) const;

    // Bootstrap yield curve from bond prices
    static YieldCurve bootstrap(const std::vector<double>& maturities,
                               const std::vector<double>& coupon_rates,
                               const std::vector<double>& prices,
                               double face_value = 100.0);
};

// Base class for fixed income instruments
class FixedIncomeInstrument {
protected:
    double face_value_;
    double maturity_;
    YieldCurve yield_curve_;

public:
    FixedIncomeInstrument(double face_value, double maturity,
                         const YieldCurve& curve)
        : face_value_(face_value), maturity_(maturity), yield_curve_(curve) {
        if (face_value <= 0)
            throw std::invalid_argument("Face value must be positive");
        if (maturity <= 0)
            throw std::invalid_argument("Maturity must be positive");
    }

    virtual ~FixedIncomeInstrument() = default;

    virtual double price() const = 0;
    virtual double yield_to_maturity(double price) const = 0;
    virtual double duration() const = 0;
    virtual double convexity() const = 0;

    double get_face_value() const { return face_value_; }
    double get_maturity() const { return maturity_; }
};

// Zero coupon bond
class ZeroCouponBond : public FixedIncomeInstrument {
public:
    ZeroCouponBond(double face_value, double maturity,
                   const YieldCurve& curve)
        : FixedIncomeInstrument(face_value, maturity, curve) {}

    double price() const override;
    double yield_to_maturity(double price) const override;
    double duration() const override;
    double convexity() const override;
};

// Coupon bearing bond
class CouponBond : public FixedIncomeInstrument {
private:
    double coupon_rate_;
    CompoundingFrequency payment_frequency_;
    std::vector<CashFlow> cash_flows_;

    void generate_cash_flows();

public:
    CouponBond(double face_value, double maturity, double coupon_rate,
              const YieldCurve& curve,
              CompoundingFrequency freq = CompoundingFrequency::SEMI_ANNUAL)
        : FixedIncomeInstrument(face_value, maturity, curve),
          coupon_rate_(coupon_rate), payment_frequency_(freq) {
        if (coupon_rate < 0)
            throw std::invalid_argument("Coupon rate cannot be negative");
        generate_cash_flows();
    }

    double price() const override;
    double yield_to_maturity(double price) const override;
    double duration() const override;
    double convexity() const override;

    // Macaulay duration
    double macaulay_duration() const;

    // Modified duration
    double modified_duration() const;

    const std::vector<CashFlow>& get_cash_flows() const { return cash_flows_; }
    double get_coupon_rate() const { return coupon_rate_; }
};

// Interest rate swap
class InterestRateSwap {
private:
    double notional_;
    double fixed_rate_;
    double maturity_;
    CompoundingFrequency payment_frequency_;
    YieldCurve yield_curve_;

public:
    InterestRateSwap(double notional, double fixed_rate, double maturity,
                    const YieldCurve& curve,
                    CompoundingFrequency freq = CompoundingFrequency::QUARTERLY)
        : notional_(notional), fixed_rate_(fixed_rate), maturity_(maturity),
          payment_frequency_(freq), yield_curve_(curve) {
        if (notional <= 0)
            throw std::invalid_argument("Notional must be positive");
        if (maturity <= 0)
            throw std::invalid_argument("Maturity must be positive");
    }

    // Present value of swap
    double present_value() const;

    // Fair swap rate
    double fair_swap_rate() const;

    // Duration and DV01
    double duration() const;
    double dv01() const;
};

// Utility functions
namespace utils {
    // Convert between different day count conventions
    double year_fraction(int days, DayCountConvention convention);

    // Convert between compounding frequencies
    double convert_rate(double rate, CompoundingFrequency from,
                       CompoundingFrequency to, double periods = 1.0);

    // Calculate accrued interest
    double accrued_interest(double face_value, double coupon_rate,
                           int days_since_last_payment,
                           DayCountConvention convention);
}

} // namespace pricing
