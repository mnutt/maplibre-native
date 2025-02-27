#include <mbgl/style/conversion/function.hpp>
#include <mbgl/style/conversion/position.hpp>
#include <mbgl/style/conversion/rotation.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/expression/case.hpp>
#include <mbgl/style/expression/dsl.hpp>
#include <mbgl/style/expression/dsl_impl.hpp>
#include <mbgl/style/expression/format_expression.hpp>
#include <mbgl/style/expression/image_expression.hpp>
#include <mbgl/style/expression/interpolate.hpp>
#include <mbgl/style/expression/match.hpp>
#include <mbgl/style/expression/step.hpp>
#include <mbgl/util/string.hpp>

#include <cassert>
#include <utility>

namespace mbgl {
namespace style {
namespace conversion {

using namespace expression;
using namespace expression::dsl;

const static std::string tokenReservedChars = "{}";

bool hasTokens(const std::string& source) {
    auto pos = source.begin();
    const auto end = source.end();

    while (pos != end) {
        auto brace = std::find(pos, end, '{');
        if (brace == end) return false;
        for (brace++; brace != end && tokenReservedChars.find(*brace) == std::string::npos; brace++)
            ;
        if (brace != end && *brace == '}') {
            return true;
        }
        pos = brace;
    }

    return false;
}

std::unique_ptr<Expression> convertTokenStringToFormatExpression(const std::string& source) {
    auto textExpression = convertTokenStringToExpression(source);
    std::vector<FormatExpressionSection> sections{FormatExpressionSection(std::move(textExpression))};
    return std::make_unique<FormatExpression>(std::move(sections));
}

std::unique_ptr<Expression> convertTokenStringToImageExpression(const std::string& source) {
    return std::make_unique<ImageExpression>(convertTokenStringToExpression(source));
}

std::unique_ptr<Expression> convertTokenStringToExpression(const std::string& source) {
    std::vector<std::unique_ptr<Expression>> inputs;

    auto pos = source.begin();
    const auto end = source.end();

    while (pos != end) {
        auto brace = std::find(pos, end, '{');
        if (pos != brace) {
            inputs.push_back(literal(std::string(pos, brace)));
        }
        pos = brace;
        if (pos != end) {
            for (brace++; brace != end && tokenReservedChars.find(*brace) == std::string::npos; brace++)
                ;
            if (brace != end && *brace == '}') {
                inputs.push_back(get(literal(std::string(pos + 1, brace))));
                pos = brace + 1;
            } else {
                inputs.push_back(literal(std::string(pos, brace)));
                pos = brace;
            }
        }
    }

    switch (inputs.size()) {
        case 0:
            return literal(source);
        case 1:
            return expression::dsl::toString(std::move(inputs[0]));
        default:
            return concat(std::move(inputs));
    }
}

template <class T>
std::optional<PropertyExpression<T>> convertFunctionToExpression(const Convertible& value,
                                                                 Error& error,
                                                                 bool convertTokens) {
    auto expression = convertFunctionToExpression(
        expression::valueTypeToExpressionType<T>(), value, error, convertTokens);
    if (!expression) {
        return std::nullopt;
    }

    std::optional<T> defaultValue{};

    auto defaultValueValue = objectMember(value, "default");
    if (defaultValueValue) {
        defaultValue = convert<T>(*defaultValueValue, error);
        if (!defaultValue) {
            error.message = R"(wrong type for "default": )" + error.message;
            return std::nullopt;
        }
    }

    return PropertyExpression<T>(std::move(*expression), defaultValue);
}

template std::optional<PropertyExpression<AlignmentType>> convertFunctionToExpression<AlignmentType>(const Convertible&,
                                                                                                     Error&,
                                                                                                     bool);
template std::optional<PropertyExpression<bool>> convertFunctionToExpression<bool>(const Convertible&, Error&, bool);
template std::optional<PropertyExpression<CirclePitchScaleType>> convertFunctionToExpression<CirclePitchScaleType>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<float>> convertFunctionToExpression<float>(const Convertible&, Error&, bool);
template std::optional<PropertyExpression<HillshadeIlluminationAnchorType>>
convertFunctionToExpression<HillshadeIlluminationAnchorType>(const Convertible&, Error&, bool);
template std::optional<PropertyExpression<IconTextFitType>> convertFunctionToExpression<IconTextFitType>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<LightAnchorType>> convertFunctionToExpression<LightAnchorType>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<LineCapType>> convertFunctionToExpression<LineCapType>(const Convertible&,
                                                                                                 Error&,
                                                                                                 bool);
template std::optional<PropertyExpression<LineJoinType>> convertFunctionToExpression<LineJoinType>(const Convertible&,
                                                                                                   Error&,
                                                                                                   bool);
template std::optional<PropertyExpression<Color>> convertFunctionToExpression<Color>(const Convertible&, Error&, bool);
template std::optional<PropertyExpression<Position>> convertFunctionToExpression<Position>(const Convertible&,
                                                                                           Error&,
                                                                                           bool);
template std::optional<PropertyExpression<RasterResamplingType>> convertFunctionToExpression<RasterResamplingType>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<std::array<float, 2>>> convertFunctionToExpression<std::array<float, 2>>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<std::array<float, 4>>> convertFunctionToExpression<std::array<float, 4>>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<std::string>> convertFunctionToExpression<std::string>(const Convertible&,
                                                                                                 Error&,
                                                                                                 bool);
template std::optional<PropertyExpression<std::vector<float>>> convertFunctionToExpression<std::vector<float>>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<std::vector<std::string>>>
convertFunctionToExpression<std::vector<std::string>>(const Convertible&, Error&, bool);
template std::optional<PropertyExpression<SymbolAnchorType>> convertFunctionToExpression<SymbolAnchorType>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<std::vector<TextVariableAnchorType>>>
convertFunctionToExpression<std::vector<TextVariableAnchorType>>(const Convertible&, Error&, bool);
template std::optional<PropertyExpression<SymbolPlacementType>> convertFunctionToExpression<SymbolPlacementType>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<SymbolZOrderType>> convertFunctionToExpression<SymbolZOrderType>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<TextJustifyType>> convertFunctionToExpression<TextJustifyType>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<TextTransformType>> convertFunctionToExpression<TextTransformType>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<TranslateAnchorType>> convertFunctionToExpression<TranslateAnchorType>(
    const Convertible&, Error&, bool);
template std::optional<PropertyExpression<Formatted>> convertFunctionToExpression<Formatted>(const Convertible&,
                                                                                             Error&,
                                                                                             bool);
template std::optional<PropertyExpression<std::vector<TextWritingModeType>>>
convertFunctionToExpression<std::vector<TextWritingModeType>>(const Convertible&, Error&, bool);
template std::optional<PropertyExpression<Image>> convertFunctionToExpression<Image>(const Convertible&, Error&, bool);
template std::optional<PropertyExpression<Rotation>> convertFunctionToExpression<Rotation>(const Convertible&,
                                                                                           Error&,
                                                                                           bool);

// Ad-hoc Converters for double and int64_t. We should replace float with double
// wholesale, and promote the int64_t Converter to general use (and it should
// check that the input is an integer).
template <>
struct Converter<double> {
    std::optional<double> operator()(const Convertible& value, Error& error) const {
        auto converted = convert<float>(value, error);
        if (!converted) {
            return std::nullopt;
        }
        return *converted;
    }
};

template <>
struct Converter<int64_t> {
    std::optional<int64_t> operator()(const Convertible& value, Error& error) const {
        auto converted = convert<float>(value, error);
        if (!converted) {
            return std::nullopt;
        }
        return static_cast<int64_t>(*converted);
    }
};

enum class FunctionType {
    Interval,
    Exponential,
    Categorical,
    Identity,
    Invalid
};

static bool interpolatable(type::Type type) {
    return type.match([&](const type::NumberType&) { return true; },
                      [&](const type::ColorType&) { return true; },
                      [&](const type::Array& array) { return array.N && array.itemType == type::Number; },
                      [&](const auto&) { return false; });
}

static std::optional<std::unique_ptr<Expression>> convertLiteral(type::Type type,
                                                                 const Convertible& value,
                                                                 Error& error,
                                                                 bool convertTokens = false) {
    return type.match(
        [&](const type::NumberType&) -> std::optional<std::unique_ptr<Expression>> {
            auto result = convert<float>(value, error);
            if (!result) {
                return std::nullopt;
            }
            return literal(static_cast<double>(*result));
        },
        [&](const type::BooleanType&) -> std::optional<std::unique_ptr<Expression>> {
            auto result = convert<bool>(value, error);
            if (!result) {
                return std::nullopt;
            }
            return literal(*result);
        },
        [&](const type::StringType&) -> std::optional<std::unique_ptr<Expression>> {
            auto result = convert<std::string>(value, error);
            if (!result) {
                return std::nullopt;
            }
            return convertTokens ? convertTokenStringToExpression(*result) : literal(*result);
        },
        [&](const type::ColorType&) -> std::optional<std::unique_ptr<Expression>> {
            auto result = convert<Color>(value, error);
            if (!result) {
                return std::nullopt;
            }
            return literal(*result);
        },
        [&](const type::Array& array) -> std::optional<std::unique_ptr<Expression>> {
            if (!isArray(value)) {
                error.message = "value must be an array";
                return std::nullopt;
            }
            if (array.N && arrayLength(value) != *array.N) {
                error.message = "value must be an array of length " + util::toString(*array.N);
                return std::nullopt;
            }
            return array.itemType.match(
                [&](const type::NumberType&) -> std::optional<std::unique_ptr<Expression>> {
                    std::vector<expression::Value> result;
                    result.reserve(arrayLength(value));
                    for (std::size_t i = 0; i < arrayLength(value); ++i) {
                        std::optional<float> number = toNumber(arrayMember(value, i));
                        if (!number) {
                            error.message = "value must be an array of numbers";
                            return std::nullopt;
                        }
                        result.emplace_back(static_cast<double>(*number));
                    }
                    return literal(result);
                },
                [&](const type::StringType&) -> std::optional<std::unique_ptr<Expression>> {
                    std::vector<expression::Value> result;
                    result.reserve(arrayLength(value));
                    for (std::size_t i = 0; i < arrayLength(value); ++i) {
                        std::optional<std::string> string = toString(arrayMember(value, i));
                        if (!string) {
                            error.message = "value must be an array of strings";
                            return std::nullopt;
                        }
                        result.emplace_back(*string);
                    }
                    return literal(result);
                },
                [&](const auto&) -> std::optional<std::unique_ptr<Expression>> {
                    assert(false); // No properties use this type.
                    return std::nullopt;
                });
        },
        [&](const type::NullType&) -> std::optional<std::unique_ptr<Expression>> {
            assert(false); // No properties use this type.
            return std::nullopt;
        },
        [&](const type::ObjectType&) -> std::optional<std::unique_ptr<Expression>> {
            assert(false); // No properties use this type.
            return std::nullopt;
        },
        [&](const type::ErrorType&) -> std::optional<std::unique_ptr<Expression>> {
            assert(false); // No properties use this type.
            return std::nullopt;
        },
        [&](const type::ValueType&) -> std::optional<std::unique_ptr<Expression>> {
            assert(false); // No properties use this type.
            return std::nullopt;
        },
        [&](const type::CollatorType&) -> std::optional<std::unique_ptr<Expression>> {
            assert(false); // No properties use this type.
            return std::nullopt;
        },
        [&](const type::FormattedType&) -> std::optional<std::unique_ptr<Expression>> {
            auto result = convert<std::string>(value, error);
            if (!result) {
                return std::nullopt;
            }
            return convertTokens ? convertTokenStringToFormatExpression(*result) : literal(Formatted(result->c_str()));
        },
        [&](const type::ImageType&) -> std::optional<std::unique_ptr<Expression>> {
            auto result = convert<std::string>(value, error);
            if (!result) {
                return std::nullopt;
            }
            return convertTokens ? std::make_unique<ImageExpression>(convertTokenStringToImageExpression(*result))
                                 : literal(Image(result->c_str()));
        });
}

static std::optional<std::map<double, std::unique_ptr<Expression>>> convertStops(const type::Type& type,
                                                                                 const Convertible& value,
                                                                                 Error& error,
                                                                                 bool convertTokens) {
    auto stopsValue = objectMember(value, "stops");
    if (!stopsValue) {
        error.message = "function value must specify stops";
        return std::nullopt;
    }

    if (!isArray(*stopsValue)) {
        error.message = "function stops must be an array";
        return std::nullopt;
    }

    if (arrayLength(*stopsValue) == 0) {
        error.message = "function must have at least one stop";
        return std::nullopt;
    }

    std::map<double, std::unique_ptr<Expression>> stops;
    for (std::size_t i = 0; i < arrayLength(*stopsValue); ++i) {
        const auto& stopValue = arrayMember(*stopsValue, i);

        if (!isArray(stopValue)) {
            error.message = "function stop must be an array";
            return std::nullopt;
        }

        if (arrayLength(stopValue) != 2) {
            error.message = "function stop must have two elements";
            return std::nullopt;
        }

        std::optional<float> t = convert<float>(arrayMember(stopValue, 0), error);
        if (!t) {
            return std::nullopt;
        }

        std::optional<std::unique_ptr<Expression>> e = convertLiteral(
            type, arrayMember(stopValue, 1), error, convertTokens);
        if (!e) {
            return std::nullopt;
        }

        stops.emplace(*t, std::move(*e));
    }

    return {std::move(stops)};
}

static void omitFirstStop(std::map<double, std::unique_ptr<Expression>>& stops) {
    double min = std::numeric_limits<double>::max();
    for (auto& s : stops) {
        if (s.first < min) {
            min = s.first;
        }
    }
    stops.emplace(-std::numeric_limits<double>::infinity(), std::move(stops[min]));
    stops.erase(min);
}

template <class T>
std::optional<std::map<T, std::unique_ptr<Expression>>> convertBranches(const type::Type& type,
                                                                        const Convertible& value,
                                                                        Error& error) {
    auto stopsValue = objectMember(value, "stops");
    if (!stopsValue) {
        error.message = "function value must specify stops";
        return std::nullopt;
    }

    if (!isArray(*stopsValue)) {
        error.message = "function stops must be an array";
        return std::nullopt;
    }

    if (arrayLength(*stopsValue) == 0) {
        error.message = "function must have at least one stop";
        return std::nullopt;
    }

    std::map<T, std::unique_ptr<Expression>> stops;
    for (std::size_t i = 0; i < arrayLength(*stopsValue); ++i) {
        const auto& stopValue = arrayMember(*stopsValue, i);

        if (!isArray(stopValue)) {
            error.message = "function stop must be an array";
            return std::nullopt;
        }

        if (arrayLength(stopValue) != 2) {
            error.message = "function stop must have two elements";
            return std::nullopt;
        }

        std::optional<T> t = convert<T>(arrayMember(stopValue, 0), error);
        if (!t) {
            return std::nullopt;
        }

        std::optional<std::unique_ptr<Expression>> e = convertLiteral(type, arrayMember(stopValue, 1), error);
        if (!e) {
            return std::nullopt;
        }

        stops.emplace(*t, std::move(*e));
    }

    return {std::move(stops)};
}

static std::optional<double> convertBase(const Convertible& value, Error& error) {
    auto baseValue = objectMember(value, "base");

    if (!baseValue) {
        return 1.0;
    }

    auto base = toNumber(*baseValue);
    if (!base) {
        error.message = "function base must be a number";
        return std::nullopt;
    }

    return *base;
}

static std::unique_ptr<Expression> step(const type::Type& type,
                                        std::unique_ptr<Expression> input,
                                        std::map<double, std::unique_ptr<Expression>> stops) {
    return std::make_unique<Step>(type, std::move(input), std::move(stops));
}

static std::unique_ptr<Expression> interpolate(type::Type type,
                                               Interpolator interpolator,
                                               std::unique_ptr<Expression> input,
                                               std::map<double, std::unique_ptr<Expression>> stops) {
    ParsingContext ctx;
    auto result = createInterpolate(std::move(type), std::move(interpolator), std::move(input), std::move(stops), ctx);
    if (!result) {
        assert(false);
        return {};
    }
    return std::move(*result);
}

template <class T>
std::unique_ptr<Expression> categorical(const type::Type& type,
                                        const std::string& property,
                                        std::map<T, std::unique_ptr<Expression>> branches,
                                        std::unique_ptr<Expression> def) {
    std::unordered_map<T, std::shared_ptr<Expression>> convertedBranches;
    for (auto& b : branches) {
        convertedBranches[b.first] = std::move(b.second);
    }
    return std::make_unique<Match<T>>(type,
                                      get(literal(property)),
                                      std::move(convertedBranches),
                                      def ? std::move(def) : error("replaced with default"));
}

template <>
std::unique_ptr<Expression> categorical<bool>(const type::Type& type,
                                              const std::string& property,
                                              std::map<bool, std::unique_ptr<Expression>> branches,
                                              std::unique_ptr<Expression> def) {
    auto it = branches.find(true);
    std::unique_ptr<Expression> trueCase = it == branches.end() ? error("replaced with default")
                                                                : std::move(it->second);

    it = branches.find(false);
    std::unique_ptr<Expression> falseCase = it == branches.end() ? error("replaced with default")
                                                                 : std::move(it->second);

    std::vector<typename Case::Branch> convertedBranches;
    convertedBranches.emplace_back(eq(get(literal(property)), literal(Value(true))), std::move(trueCase));
    convertedBranches.emplace_back(eq(get(literal(property)), literal(Value(false))), std::move(falseCase));

    return std::make_unique<Case>(
        type, std::move(convertedBranches), def ? std::move(def) : error("replaced with default"));
}

static std::unique_ptr<Expression> numberOrDefault(const type::Type& type,
                                                   std::unique_ptr<Expression> get,
                                                   std::unique_ptr<Expression> expr,
                                                   std::unique_ptr<Expression> def) {
    if (!def) {
        return expr;
    }

    std::vector<Case::Branch> branches;
    branches.emplace_back(eq(compound("typeof", std::move(get)), literal("number")), std::move(expr));
    return std::make_unique<Case>(type, std::move(branches), std::move(def));
}

static std::optional<std::unique_ptr<Expression>> convertIntervalFunction(
    const type::Type& type,
    const Convertible& value,
    Error& error,
    const std::function<std::unique_ptr<Expression>(bool)>& makeInput,
    std::unique_ptr<Expression> def,
    bool convertTokens = false) {
    auto stops = convertStops(type, value, error, convertTokens);
    if (!stops) {
        return std::nullopt;
    }
    omitFirstStop(*stops);

    return numberOrDefault(type, makeInput(false), step(type, makeInput(true), std::move(*stops)), std::move(def));
}

static std::optional<std::unique_ptr<Expression>> convertExponentialFunction(
    const type::Type& type,
    const Convertible& value,
    Error& error,
    const std::function<std::unique_ptr<Expression>(bool)>& makeInput,
    std::unique_ptr<Expression> def,
    bool convertTokens = false) {
    auto stops = convertStops(type, value, error, convertTokens);
    if (!stops) {
        return std::nullopt;
    }
    auto base = convertBase(value, error);
    if (!base) {
        return std::nullopt;
    }

    return numberOrDefault(type,
                           makeInput(false),
                           interpolate(type, exponential(*base), makeInput(true), std::move(*stops)),
                           std::move(def));
}

static std::optional<std::unique_ptr<Expression>> convertCategoricalFunction(const type::Type& type,
                                                                             const Convertible& value,
                                                                             Error& err,
                                                                             const std::string& property,
                                                                             std::unique_ptr<Expression> def) {
    auto stopsValue = objectMember(value, "stops");
    if (!stopsValue) {
        err.message = "function value must specify stops";
        return std::nullopt;
    }

    if (!isArray(*stopsValue)) {
        err.message = "function stops must be an array";
        return std::nullopt;
    }

    if (arrayLength(*stopsValue) == 0) {
        err.message = "function must have at least one stop";
        return std::nullopt;
    }

    const auto& first = arrayMember(*stopsValue, 0);

    if (!isArray(first)) {
        err.message = "function stop must be an array";
        return std::nullopt;
    }

    if (arrayLength(first) != 2) {
        err.message = "function stop must have two elements";
        return std::nullopt;
    }

    if (toBool(arrayMember(first, 0))) {
        auto branches = convertBranches<bool>(type, value, err);
        if (!branches) {
            return std::nullopt;
        }
        return categorical(type, property, std::move(*branches), std::move(def));
    }

    if (toNumber(arrayMember(first, 0))) {
        auto branches = convertBranches<int64_t>(type, value, err);
        if (!branches) {
            return std::nullopt;
        }
        return categorical(type, property, std::move(*branches), std::move(def));
    }

    if (toString(arrayMember(first, 0))) {
        auto branches = convertBranches<std::string>(type, value, err);
        if (!branches) {
            return std::nullopt;
        }
        return categorical(type, property, std::move(*branches), std::move(def));
    }

    err.message = "stop domain value must be a number, string, or boolean";
    return std::nullopt;
}

template <class T, class Fn>
std::optional<std::unique_ptr<Expression>> composite(type::Type type,
                                                     const Convertible& value,
                                                     Error& error,
                                                     const Fn& makeInnerExpression) {
    auto base = convertBase(value, error);
    if (!base) {
        return std::nullopt;
    }

    auto stopsValue = objectMember(value, "stops");

    // Checked by caller.
    assert(stopsValue);
    assert(isArray(*stopsValue));

    std::map<float, std::map<T, std::unique_ptr<Expression>>> map;

    for (std::size_t i = 0; i < arrayLength(*stopsValue); ++i) {
        const auto& stopValue = arrayMember(*stopsValue, i);

        if (!isArray(stopValue)) {
            error.message = "function stop must be an array";
            return std::nullopt;
        }

        if (arrayLength(stopValue) != 2) {
            error.message = "function stop must have two elements";
            return std::nullopt;
        }

        const auto& stopInput = arrayMember(stopValue, 0);

        if (!isObject(stopInput)) {
            error.message = "stop input must be an object";
            return std::nullopt;
        }

        auto zoomValue = objectMember(stopInput, "zoom");
        if (!zoomValue) {
            error.message = "stop input must specify zoom";
            return std::nullopt;
        }

        auto sourceValue = objectMember(stopInput, "value");
        if (!sourceValue) {
            error.message = "stop input must specify value";
            return std::nullopt;
        }

        std::optional<float> z = convert<float>(*zoomValue, error);
        if (!z) {
            return std::nullopt;
        }

        std::optional<T> d = convert<T>(*sourceValue, error);
        if (!d) {
            return std::nullopt;
        }

        std::optional<std::unique_ptr<Expression>> r = convertLiteral(type, arrayMember(stopValue, 1), error);
        if (!r) {
            return std::nullopt;
        }

        map[*z].emplace(*d, std::move(*r));
    }

    std::map<double, std::unique_ptr<Expression>> stops;

    for (auto& e : map) {
        stops.emplace(e.first, makeInnerExpression(type, *base, std::move(e.second)));
    }

    if (interpolatable(type)) {
        return interpolate(type, linear(), zoom(), std::move(stops));
    } else {
        return step(type, zoom(), std::move(stops));
    }
}

std::optional<std::unique_ptr<Expression>> convertFunctionToExpression(type::Type type,
                                                                       const Convertible& value,
                                                                       Error& err,
                                                                       bool convertTokens) {
    if (!isObject(value)) {
        err.message = "function must be an object";
        return std::nullopt;
    }

    FunctionType functionType = FunctionType::Invalid;

    auto typeValue = objectMember(value, "type");
    if (!typeValue) {
        functionType = interpolatable(type) ? FunctionType::Exponential : FunctionType::Interval;
    } else {
        auto string = toString(*typeValue);
        if (string) {
            if (*string == "interval") functionType = FunctionType::Interval;
            if (*string == "exponential" && interpolatable(type)) functionType = FunctionType::Exponential;
            if (*string == "categorical") functionType = FunctionType::Categorical;
            if (*string == "identity") functionType = FunctionType::Identity;
        }
    }

    auto defaultExpr = [&]() -> std::unique_ptr<Expression> {
        auto member = objectMember(value, "default");
        if (member) {
            auto literal = convertLiteral(type, *member, err);
            if (literal) {
                return std::move(*literal);
            }
        }
        return nullptr;
    };

    if (!objectMember(value, "property")) {
        // Camera function.
        switch (functionType) {
            case FunctionType::Interval:
                return convertIntervalFunction(
                    type, value, err, [](bool) { return zoom(); }, defaultExpr(), convertTokens);
            case FunctionType::Exponential:
                return convertExponentialFunction(
                    type, value, err, [](bool) { return zoom(); }, defaultExpr(), convertTokens);
            default:
                err.message = "unsupported function type";
                return std::nullopt;
        }
    }

    auto propertyValue = objectMember(value, "property");
    if (!propertyValue) {
        err.message = "function must specify property";
        return std::nullopt;
    }

    auto property = toString(*propertyValue);
    if (!property) {
        err.message = "function property must be a string";
        return std::nullopt;
    }

    if (functionType == FunctionType::Identity) {
        return type.match(
            [&](const type::StringType&) -> std::optional<std::unique_ptr<Expression>> {
                return string(get(literal(*property)), defaultExpr());
            },
            [&](const type::NumberType&) -> std::optional<std::unique_ptr<Expression>> {
                return number(get(literal(*property)), defaultExpr());
            },
            [&](const type::BooleanType&) -> std::optional<std::unique_ptr<Expression>> {
                return boolean(get(literal(*property)), defaultExpr());
            },
            [&](const type::ColorType&) -> std::optional<std::unique_ptr<Expression>> {
                return toColor(get(literal(*property)), defaultExpr());
            },
            [&](const type::Array& array) -> std::optional<std::unique_ptr<Expression>> {
                return assertion(array, get(literal(*property)), defaultExpr());
            },
            [&](const type::FormattedType&) -> std::optional<std::unique_ptr<Expression>> {
                return toFormatted(get(literal(*property)), defaultExpr());
            },
            [&](const type::ImageType&) -> std::optional<std::unique_ptr<Expression>> {
                return toImage(get(literal(*property)), defaultExpr());
            },
            [&](const auto&) -> std::optional<std::unique_ptr<Expression>> {
                assert(false); // No properties use this type.
                return std::nullopt;
            });
    }

    auto stopsValue = objectMember(value, "stops");
    if (!stopsValue) {
        err.message = "function value must specify stops";
        return std::nullopt;
    }

    if (!isArray(*stopsValue)) {
        err.message = "function stops must be an array";
        return std::nullopt;
    }

    if (arrayLength(*stopsValue) == 0) {
        err.message = "function must have at least one stop";
        return std::nullopt;
    }

    const auto& first = arrayMember(*stopsValue, 0);

    if (!isArray(first)) {
        err.message = "function stop must be an array";
        return std::nullopt;
    }

    if (arrayLength(first) != 2) {
        err.message = "function stop must have two elements";
        return std::nullopt;
    }

    const auto& stop = arrayMember(first, 0);

    const auto getProperty = [&](bool coerce) {
        if (coerce) {
            return number(get(literal(*property)));
        } else {
            return get(literal(*property));
        }
    };

    if (!isObject(stop)) {
        // Source function.
        switch (functionType) {
            case FunctionType::Interval:
                return convertIntervalFunction(type, value, err, getProperty, defaultExpr());
            case FunctionType::Exponential:
                return convertExponentialFunction(type, value, err, getProperty, defaultExpr());
            case FunctionType::Categorical:
                return convertCategoricalFunction(type, value, err, *property, defaultExpr());
            default:
                err.message = "unsupported function type";
                return std::nullopt;
        }
    } else {
        // Composite function.
        auto sourceValue = objectMember(stop, "value");
        if (!sourceValue) {
            err.message = "stop must specify value";
            return std::nullopt;
        }

        if (toBool(*sourceValue)) {
            switch (functionType) {
                case FunctionType::Categorical:
                    return composite<bool>(
                        type,
                        value,
                        err,
                        [&](const type::Type& type_, double, std::map<bool, std::unique_ptr<Expression>> stops) {
                            return categorical<bool>(type_, *property, std::move(stops), defaultExpr());
                        });
                default:
                    err.message = "unsupported function type";
                    return std::nullopt;
            }
        }

        if (toNumber(*sourceValue)) {
            switch (functionType) {
                case FunctionType::Interval:
                    return composite<double>(
                        type,
                        value,
                        err,
                        [&](const type::Type& type_, double, std::map<double, std::unique_ptr<Expression>> stops) {
                            omitFirstStop(stops);
                            return numberOrDefault(type,
                                                   getProperty(false),
                                                   step(type_, getProperty(true), std::move(stops)),
                                                   defaultExpr());
                        });
                case FunctionType::Exponential:
                    return composite<double>(
                        type,
                        value,
                        err,
                        [&](type::Type type_, double base, std::map<double, std::unique_ptr<Expression>> stops) {
                            return numberOrDefault(
                                type,
                                getProperty(false),
                                interpolate(std::move(type_), exponential(base), getProperty(true), std::move(stops)),
                                defaultExpr());
                        });
                case FunctionType::Categorical:
                    return composite<int64_t>(
                        type,
                        value,
                        err,
                        [&](const type::Type& type_, double, std::map<int64_t, std::unique_ptr<Expression>> stops) {
                            return categorical<int64_t>(type_, *property, std::move(stops), defaultExpr());
                        });
                default:
                    err.message = "unsupported function type";
                    return std::nullopt;
            }
        }

        if (toString(*sourceValue)) {
            switch (functionType) {
                case FunctionType::Categorical:
                    return composite<std::string>(
                        type,
                        value,
                        err,
                        [&](const type::Type& type_, double, std::map<std::string, std::unique_ptr<Expression>> stops) {
                            return categorical<std::string>(type_, *property, std::move(stops), defaultExpr());
                        });
                default:
                    err.message = "unsupported function type";
                    return std::nullopt;
            }
        }

        err.message = "stop domain value must be a number, string, or boolean";
        return std::nullopt;
    }
}

} // namespace conversion
} // namespace style
} // namespace mbgl
