#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../include/option_pricer.hpp"
#include "../include/fixed_income_pricer.hpp"

namespace py = pybind11;
using namespace pricing;

PYBIND11_MODULE(pricing_engine_py, m) {
    m.doc() = "Production-ready pricing engine for financial instruments";

    // Enums
    py::enum_<OptionType>(m, "OptionType")
        .value("CALL", OptionType::CALL)
        .value("PUT", OptionType::PUT)
        .export_values();

    py::enum_<OptionStyle>(m, "OptionStyle")
        .value("EUROPEAN", OptionStyle::EUROPEAN)
        .value("AMERICAN", OptionStyle::AMERICAN)
        .export_values();

    py::enum_<CompoundingFrequency>(m, "CompoundingFrequency")
        .value("CONTINUOUS", CompoundingFrequency::CONTINUOUS)
        .value("ANNUAL", CompoundingFrequency::ANNUAL)
        .value("SEMI_ANNUAL", CompoundingFrequency::SEMI_ANNUAL)
        .value("QUARTERLY", CompoundingFrequency::QUARTERLY)
        .value("MONTHLY", CompoundingFrequency::MONTHLY)
        .export_values();

    py::enum_<DayCountConvention>(m, "DayCountConvention")
        .value("ACT_360", DayCountConvention::ACT_360)
        .value("ACT_365", DayCountConvention::ACT_365)
        .value("THIRTY_360", DayCountConvention::THIRTY_360)
        .export_values();

    // Structures
    py::class_<MarketData>(m, "MarketData")
        .def(py::init<double, double, double, double, double, double>(),
             py::arg("spot_price"),
             py::arg("strike_price"),
             py::arg("risk_free_rate"),
             py::arg("volatility"),
             py::arg("time_to_maturity"),
             py::arg("dividend_yield") = 0.0)
        .def_readwrite("spot_price", &MarketData::spot_price)
        .def_readwrite("strike_price", &MarketData::strike_price)
        .def_readwrite("risk_free_rate", &MarketData::risk_free_rate)
        .def_readwrite("volatility", &MarketData::volatility)
        .def_readwrite("time_to_maturity", &MarketData::time_to_maturity)
        .def_readwrite("dividend_yield", &MarketData::dividend_yield)
        .def("validate", &MarketData::validate);

    py::class_<Greeks>(m, "Greeks")
        .def(py::init<>())
        .def_readwrite("delta", &Greeks::delta)
        .def_readwrite("gamma", &Greeks::gamma)
        .def_readwrite("theta", &Greeks::theta)
        .def_readwrite("vega", &Greeks::vega)
        .def_readwrite("rho", &Greeks::rho)
        .def("__repr__", [](const Greeks& g) {
            return "Greeks(delta=" + std::to_string(g.delta) +
                   ", gamma=" + std::to_string(g.gamma) +
                   ", theta=" + std::to_string(g.theta) +
                   ", vega=" + std::to_string(g.vega) +
                   ", rho=" + std::to_string(g.rho) + ")";
        });

    py::class_<CashFlow>(m, "CashFlow")
        .def(py::init<double, double>())
        .def_readwrite("amount", &CashFlow::amount)
        .def_readwrite("time", &CashFlow::time);

    py::class_<YieldCurvePoint>(m, "YieldCurvePoint")
        .def(py::init<double, double>())
        .def_readwrite("maturity", &YieldCurvePoint::maturity)
        .def_readwrite("rate", &YieldCurvePoint::rate);

    // Option Pricers
    py::class_<OptionPricer>(m, "OptionPricer")
        .def("price", &OptionPricer::price)
        .def("calculate_greeks", &OptionPricer::calculate_greeks)
        .def("get_market_data", &OptionPricer::get_market_data)
        .def("get_option_type", &OptionPricer::get_option_type)
        .def("get_option_style", &OptionPricer::get_option_style);

    py::class_<BlackScholesPricer, OptionPricer>(m, "BlackScholesPricer")
        .def(py::init<const MarketData&, OptionType>())
        .def("price", &BlackScholesPricer::price)
        .def("calculate_greeks", &BlackScholesPricer::calculate_greeks)
        .def_static("implied_volatility", &BlackScholesPricer::implied_volatility,
                   py::arg("data"),
                   py::arg("type"),
                   py::arg("market_price"),
                   py::arg("tolerance") = 1e-6,
                   py::arg("max_iterations") = 100);

    py::class_<BinomialTreePricer, OptionPricer>(m, "BinomialTreePricer")
        .def(py::init<const MarketData&, OptionType, OptionStyle, int>(),
             py::arg("data"),
             py::arg("type"),
             py::arg("style") = OptionStyle::AMERICAN,
             py::arg("steps") = 100)
        .def("price", &BinomialTreePricer::price)
        .def("calculate_greeks", &BinomialTreePricer::calculate_greeks)
        .def("set_steps", &BinomialTreePricer::set_steps)
        .def("get_steps", &BinomialTreePricer::get_steps);

    py::class_<MonteCarloOptionPricer, OptionPricer>(m, "MonteCarloOptionPricer")
        .def(py::init<const MarketData&, OptionType, OptionStyle, int, unsigned int>(),
             py::arg("data"),
             py::arg("type"),
             py::arg("style") = OptionStyle::EUROPEAN,
             py::arg("num_simulations") = 100000,
             py::arg("seed") = 42)
        .def("price", &MonteCarloOptionPricer::price)
        .def("calculate_greeks", &MonteCarloOptionPricer::calculate_greeks)
        .def("set_num_simulations", &MonteCarloOptionPricer::set_num_simulations)
        .def("get_num_simulations", &MonteCarloOptionPricer::get_num_simulations);

    // Yield Curve
    py::class_<YieldCurve>(m, "YieldCurve")
        .def(py::init<const std::vector<YieldCurvePoint>&, CompoundingFrequency>(),
             py::arg("points"),
             py::arg("compounding") = CompoundingFrequency::CONTINUOUS)
        .def("get_rate", &YieldCurve::get_rate)
        .def("get_discount_factor", &YieldCurve::get_discount_factor)
        .def("get_forward_rate", &YieldCurve::get_forward_rate)
        .def_static("bootstrap", &YieldCurve::bootstrap,
                   py::arg("maturities"),
                   py::arg("coupon_rates"),
                   py::arg("prices"),
                   py::arg("face_value") = 100.0);

    // Fixed Income Instruments
    py::class_<FixedIncomeInstrument>(m, "FixedIncomeInstrument")
        .def("price", &FixedIncomeInstrument::price)
        .def("yield_to_maturity", &FixedIncomeInstrument::yield_to_maturity)
        .def("duration", &FixedIncomeInstrument::duration)
        .def("convexity", &FixedIncomeInstrument::convexity)
        .def("get_face_value", &FixedIncomeInstrument::get_face_value)
        .def("get_maturity", &FixedIncomeInstrument::get_maturity);

    py::class_<ZeroCouponBond, FixedIncomeInstrument>(m, "ZeroCouponBond")
        .def(py::init<double, double, const YieldCurve&>())
        .def("price", &ZeroCouponBond::price)
        .def("yield_to_maturity", &ZeroCouponBond::yield_to_maturity)
        .def("duration", &ZeroCouponBond::duration)
        .def("convexity", &ZeroCouponBond::convexity);

    py::class_<CouponBond, FixedIncomeInstrument>(m, "CouponBond")
        .def(py::init<double, double, double, const YieldCurve&, CompoundingFrequency>(),
             py::arg("face_value"),
             py::arg("maturity"),
             py::arg("coupon_rate"),
             py::arg("curve"),
             py::arg("freq") = CompoundingFrequency::SEMI_ANNUAL)
        .def("price", &CouponBond::price)
        .def("yield_to_maturity", &CouponBond::yield_to_maturity)
        .def("duration", &CouponBond::duration)
        .def("convexity", &CouponBond::convexity)
        .def("macaulay_duration", &CouponBond::macaulay_duration)
        .def("modified_duration", &CouponBond::modified_duration)
        .def("get_cash_flows", &CouponBond::get_cash_flows)
        .def("get_coupon_rate", &CouponBond::get_coupon_rate);

    py::class_<InterestRateSwap>(m, "InterestRateSwap")
        .def(py::init<double, double, double, const YieldCurve&, CompoundingFrequency>(),
             py::arg("notional"),
             py::arg("fixed_rate"),
             py::arg("maturity"),
             py::arg("curve"),
             py::arg("freq") = CompoundingFrequency::QUARTERLY)
        .def("present_value", &InterestRateSwap::present_value)
        .def("fair_swap_rate", &InterestRateSwap::fair_swap_rate)
        .def("duration", &InterestRateSwap::duration)
        .def("dv01", &InterestRateSwap::dv01);

    // Utility functions
    m.def("year_fraction", &utils::year_fraction);
    m.def("convert_rate", &utils::convert_rate,
          py::arg("rate"),
          py::arg("from"),
          py::arg("to"),
          py::arg("periods") = 1.0);
    m.def("accrued_interest", &utils::accrued_interest);
}
