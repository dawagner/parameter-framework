/*
 * Copyright (c) 2015, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include <catch.hpp>

/** Ubuntu keeps using an old catch version where "Method" version of BDD Macros are
 * not available. This macro adds this functionality in case that catch version is too old.
 */
#ifndef SCENARIO_METHOD
#define SCENARIO_METHOD(className, name, tags) \
    INTERNAL_CATCH_TEST_CASE_METHOD(className, "Scenario: " name, tags)
#endif

#include <criterion/Criteria.h>
#include <criterion/Criterion.h>
#include <criterion/InclusiveCriterion.h>
#include <log/Logger.h>
#include "XmlSerializingContext.h"
#include "XmlStreamDocSink.h"
#include "XmlMemoryDocSource.h"

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>
#include <stdexcept>

using namespace core;
using namespace core::criterion;

/** Raw logging class Helper */
class TestLogger : public log::ILogger
{
public:
    void info(const std::string& info) { logBuf << "Info: " << info; }
    void warning(const std::string& warning) { logBuf << "Warning: " << warning; }
    std::string getLog() { return logBuf.str(); }

private:
    std::ostringstream logBuf;
};

struct LoggingTest {
    LoggingTest() : mRawLogger(), mLogger(mRawLogger) {}

    void REQUIRE_FAILURE(bool success, const std::string& result)
    {
        THEN("It should be an error")
        {
            INFO("Result message was: '" + result + "'");
            CHECK(not success);
            CHECK(not result.empty());
        }
    }

    void REQUIRE_FAILURE(bool success)
    {
        THEN("It should be an error")
        {
            CHECK(not success);
        }
    }

    void REQUIRE_SUCCESS(bool success, const std::string& result = "")
    {
        THEN("It should be a success")
        {
            INFO("Result message was" + (result.empty() ? " empty" : ": '" + result + "'"));
            CHECK(success);
        }
    }

    inline void removeWhitespaces(std::string& s)
    {
        s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
    }

    /** xml serialization into a string helper */
    void xmlSerialize(std::string& result, IXmlSource* sourceNode, const std::string& nodeName)
    {
        std::string error;
        CXmlElement xmlElement;
        CXmlSerializingContext context(error);
        CXmlMemoryDocSource source(sourceNode, false, nodeName);

        std::ostringstream sinkStream;
        CXmlStreamDocSink sink(sinkStream);

        // Processing will call XmlSource toXml method
        REQUIRE_SUCCESS(sink.process(source, context), error);
        result = sinkStream.str();
    }

    /** Raw logger we can check */
    TestLogger mRawLogger;
    /** Application standard Logger */
    log::Logger mLogger;
};

/** Test fixtures for Criteria */
struct CriteriaTest : LoggingTest {
    void checkListCriteria(bool human, bool typeInfo)
    {
        std::list<std::string> results, desireds;
        for (auto& description : mDescriptions) {
            desireds.push_back(description.criterion->getFormattedDescription(true, true));
        }
        mCriteria.listSelectionCriteria(results, human, typeInfo);
        THEN("Each criterion description should be in the listing")
        {
            for (const auto& result : results) {
                CAPTURE(result);
                const auto& it = std::find(desireds.begin(), desireds.end(), result);
                CHECK(it != desireds.end());
            }
        }
    }

    /** Structure to describe and test criteria */
    struct CriterionDescription {
        const std::string name;
        bool isInclusive;
        Criterion* criterion;
    };
    using CriterionDescriptions = std::vector<CriterionDescription>;

    /** Tested descriptions */
    CriterionDescriptions mDescriptions {
        { "A", true, nullptr },
        { "B", false, nullptr },
        { "C", true, nullptr },
        { "D", false, nullptr }
    };

    /** Tested criteria object */
    Criteria mCriteria {};
};

/** Test fixtures for Criterion */
struct CriterionTest : public LoggingTest {
    /** Help to generate some values */
    CriterionInterface::Values generateCriterionValues(size_t nbValues)
    {
        CriterionInterface::Values values;

        for (size_t i = 0; i < nbValues; i++) {
            values.emplace("Crit_" + std::to_string(i), i);
        }

        return values;
    }

    void checkUnknownValueGet(Criterion& criterion)
    {
        WHEN("Getting a literal value from an unknown numerical one")
        {
            std::string result;
            std::string literalValue;
            REQUIRE_FAILURE(criterion.getLiteralValue(666, literalValue));
            CHECK(literalValue.empty());
        }
        WHEN("Getting a numerical value from an unknown literal one")
        {
            std::string result;
            int numericalValue = 0;
            REQUIRE_FAILURE(criterion.getNumericalValue("UnknowValue", numericalValue));
            CHECK(numericalValue == 0);
        }
    }

    void checkInclusiveCriterionSet(Criterion& criterion)
    {
        std::string error;
        // Criterion contains 300 value as defined previously
        WHEN("Setting many inclusive value at the same time")
        {
            CriterionInterface::State state{1, 3, 4, 7, 10};
            CriterionInterface::State subState{1, 7, 10};
            CriterionInterface::State almostSubState{1, 7, 10, 15};
            CriterionInterface::State exclude{5, 8, 15};

            REQUIRE_SUCCESS(criterion.setState(state, error), error);

            WHEN("Matching with 'Includes' a state contained in the current state")
            {
                REQUIRE_SUCCESS(criterion.match("Includes", subState));
            }
            WHEN("Matching with 'Includes' a state with not all its bit set in the current state")
            {
                REQUIRE_FAILURE(criterion.match("Includes", almostSubState));
            }
            WHEN("Matching with 'Includes' a state with no common bit with the current state")
            {
                REQUIRE_FAILURE(criterion.match("Includes", exclude));
            }
            WHEN("Matching with 'Excludes' a state contained in the current state")
            {
                REQUIRE_FAILURE(criterion.match("Excludes", subState));
            }
            WHEN("Matching with 'Excludes' a state with not all its bit set in the current state")
            {
                REQUIRE_FAILURE(criterion.match("Excludes", almostSubState));
            }
            WHEN("Matching with 'Excludes' a state with no common bit with the current state")
            {
                REQUIRE_SUCCESS(criterion.match("Excludes", exclude));
            }
        }
        WHEN("Setting many inclusive values with some unknown") {
            REQUIRE_FAILURE(criterion.setState(CriterionInterface::State{666, 777, 1, 7, 10}, error), error);
            THEN("Criterion should not be modified")
            {
                CHECK(not criterion.hasBeenModified());
            }
        }
    }

    void checkExclusiveCriterionSet(Criterion& criterion, CriterionInterface::Values& values)
    {
        std::string error;
        WHEN("Setting the current value")
        {
            CriterionInterface::State currentState = criterion.getState();
            CAPTURE(*currentState.begin());
            std::string oldLog = mRawLogger.getLog();
            REQUIRE_SUCCESS(criterion.setState(currentState, error), error);
            THEN("Criterion should not be modified")
            {
                CHECK(not criterion.hasBeenModified());
            }
            THEN("No information should be logged")
            {
                CHECK(oldLog == mRawLogger.getLog());
            }
        }

        for (auto& value : values) {
            if (CriterionInterface::State{value.second} != criterion.getState()) {
                WHEN("Setting a new value")
                {
                    CAPTURE(value.second);
                    REQUIRE_SUCCESS(criterion.setState(
                                CriterionInterface::State{value.second}, error), error);

                    THEN("State should have been updated")
                    {
                        CHECK(criterion.getState() == CriterionInterface::State{value.second});
                        CHECK(criterion.getFormattedState() == value.first);
                    }
                    THEN("Criterion should be modified")
                    {
                        CHECK(criterion.hasBeenModified());
                    }
                    THEN("Criterion Is match method should be valid")
                    {
                        CHECK(criterion.match("Is", CriterionInterface::State{value.second}));
                    }
                    THEN("Criterion IsNot match method should not be valid")
                    {
                        CHECK(not criterion.match("IsNot",
                                                  CriterionInterface::State{value.second}));
                    }
                    THEN("Criterion update event should be logged")
                    {
                        size_t logPos = mRawLogger.getLog().find(
                            "Info: Selection criterion changed event: " +
                            criterion.getFormattedDescription(false, false));
                        CHECK(logPos != std::string::npos);
                    }
                }
            }
        }
        WHEN("Setting  many value in a raw")
        {
            // Set value which are valid for inclusive or exclusive criterion
            REQUIRE_SUCCESS(criterion.setState(CriterionInterface::State{2}, error), error);
            REQUIRE_SUCCESS(criterion.setState(CriterionInterface::State{4}, error), error);
            THEN("Criterion should be modified")
            {
                CHECK(criterion.hasBeenModified());
            }
            THEN("Criterion multi modification should be logged")
            {
                size_t logPos = mRawLogger.getLog().find("Warning: Selection criterion '" +
                                                         criterion.getCriterionName() +
                                                         "' has been modified 1 time(s) without any"
                                                         " configuration application");
                CHECK(logPos != std::string::npos);
            }
            WHEN("Resetting criterion status")
            {
                criterion.resetModifiedStatus();
                THEN("Criterion should not be modified")
                {
                    CHECK(not criterion.hasBeenModified());
                }
            }
        }
        WHEN("Setting an unknown value") {
            REQUIRE_FAILURE(criterion.setState(CriterionInterface::State{666}, error), error);
            THEN("Criterion should not be modified")
            {
                CHECK(not criterion.hasBeenModified());
            }
        }
        WHEN("Setting no value") {
            REQUIRE_SUCCESS(criterion.setState(CriterionInterface::State{}, error), error);
            THEN("State should have been updated")
            {
                if (criterion.isInclusive()) {
                    CHECK(criterion.getState() == CriterionInterface::State{});
                    CHECK(criterion.getFormattedState() == "none");
                } else {
                    CHECK(criterion.getState() == CriterionInterface::State{0});
                    CHECK(criterion.getFormattedState() == "Crit_0");
                }
            }
        }
        if (!criterion.isInclusive()) {
            WHEN("Setting more than one value in an exclusive criterion")
            {
                REQUIRE_FAILURE(criterion.setState(CriterionInterface::State{1, 2, 3}, error),
                                error);
            }
        }
    }

    void checkSerialization(Criterion& criterion)
    {
        std::string defaultValue = criterion.isInclusive() ? "none" : "a";
        std::string kind = criterion.isInclusive() ? "Inclusive" : "Exclusive";
        WHEN("Serializing through xml")
        {
            std::string xmlDescription =
                R"(<?xml version="1.0" encoding="UTF-8"?>
                  <SelectionCriterion Value=")"
                + defaultValue +
                "\" Name=\"" + criterion.getCriterionName() +
                "\" Kind=\"" + kind +
                R"(">
                    <ValuePair Literal="a" Numerical="2"/>
                    <ValuePair Literal="b" Numerical="3"/>
                    <ValuePair Literal="c" Numerical="4"/>)" +
                "</SelectionCriterion>";

            std::string result;
            xmlSerialize(result, &criterion, "SelectionCriterion");
            // Remove whitespaces as they are not relevant in xml
            removeWhitespaces(result);
            removeWhitespaces(xmlDescription);

            THEN("Generated xml match expectation")
            {
                CHECK(result == xmlDescription);
            }
        }

        WHEN("Serializing through Csv")
        {
            std::string nameInfo = "Criterion name: " + criterion.getCriterionName();
            std::string currentStateInfo = std::string(", current state: ") + defaultValue;

            THEN("Generated csv match expectation")
            {
                std::string csvDescription = nameInfo + currentStateInfo;
                std::string dump = criterion.getFormattedDescription(false, false);
                CHECK(dump == csvDescription);
            }

            THEN("Generated csv with type information match expectation")
            {
                std::string csvDescription = nameInfo + ", type kind: " + kind + currentStateInfo +
                                             ", states: {a, b, c}";
                std::string dump = criterion.getFormattedDescription(true, false);
                CHECK(dump == csvDescription);
            }
        }
        WHEN("Generating human readable description")
        {
            THEN("Generated description match expectation")
            {
                std::string description = criterion.getCriterionName() + " = " + defaultValue;
                std::string dump = criterion.getFormattedDescription(false, true);
                CHECK(dump == description);
            }

            THEN("Generated description with type information match expectation")
            {
                std::string description = criterion.getCriterionName() + ":";
                std::string titleDecorator(description.length(), '=');
                description = "\n" + description + "\n" + titleDecorator +
                              "\nPossible states (" + kind + "): {a, b, c}" +
                              "\n" + "Current state = " + defaultValue;
                std::string dump = criterion.getFormattedDescription(true, true);
                CHECK(dump == description);
            }
        }
    }

    template <class CriterionType>
    void checkDisplay(const std::string& criterionName)
    {
        WHEN("Wanting to serialize it")
        {
            CriterionInterface::Values values = { { "a", 2 }, { "b", 3 }, { "c", 4 } };
            CriterionType criterion(criterionName, values, mLogger);
            std::string possibleValues = "{a, b, c}";
            THEN("Possible values match all values added in the criterion")
            {
                CHECK(criterion.listPossibleValues() == possibleValues);
            }

            checkSerialization(criterion);

            if (criterion.isInclusive()) {
                CriterionInterface::State validState{2, 3, 4};
                std::string error;
                WHEN("Setting some criterion value")
                {
                    REQUIRE_SUCCESS(criterion.setState(validState, error), error);
                    THEN("Formatted state contains all set values")
                    {
                        std::string formattedState = "a|b|c";
                        CHECK(criterion.getFormattedState() == formattedState);
                    }
                }
                WHEN("Setting a mask containing unknown values")
                {
                    CriterionInterface::State erroneousState{2, 3, 4, 6, 8, 10, 12, 14};
                    REQUIRE_FAILURE(criterion.setState(erroneousState, error), error);
                }
            }
        }
    }

    void checkInsertionBijectivity(Criterion& criterion, CriterionInterface::Values values)
    {
        WHEN("Checking internal value")
        {
            for (auto& value : values) {
                THEN("Numerical value should correspond")
                {
                    int numericalValue;
                    REQUIRE_SUCCESS(criterion.getNumericalValue(value.first, numericalValue));
                    CHECK(numericalValue == value.second);
                }
                THEN("Literal value should correspond")
                {
                    std::string literalValue;
                    REQUIRE_SUCCESS(criterion.getLiteralValue(value.second, literalValue));
                    CHECK(literalValue == value.first);
                }
            }

            checkExclusiveCriterionSet(criterion, values);
            if (criterion.isInclusive()) {
                // Inclusive criterion has a more evolved setting behavior
                checkInclusiveCriterionSet(criterion);
            }
        }
    }

    void checkCriterionBasicBehavior(Criterion& criterion,
                                     std::string name,
                                     CriterionInterface::Values values)
    {
        using MatchMethods = std::vector<std::string>;
        /** key indicates if it's available for exclusive criterion */
        const std::map<bool, MatchMethods> mMatchMethods {
            { true, { "Is", "IsNot" } },
            { false, { "Includes", "Excludes" } }
        };

        THEN("Only corresponding match method are available")
        {
            for (auto& matcher : mMatchMethods) {
                for (auto& matchMethod : matcher.second) {
                    INFO("Checking availibility of '" + matchMethod + "' match method");
                    /** A method is authorized if available or if the criterion is not inclusive
                     * and the method is defined in mMatchMethods as unavailable for exclusive
                     * criterion
                     */
                    bool isAuthorizedMethod = criterion.isMatchMethodAvailable(matchMethod) or
                                              not (criterion.isInclusive() or matcher.first);
                    CHECK(isAuthorizedMethod);
                }
            }
        }
        WHEN("Undefined match method is requested")
        {
            REQUIRE_THROWS_AS(criterion.match("InvalidMatch", CriterionInterface::State{0}), std::out_of_range);
        }

        THEN("The criterion has not been modified")
        {
            CHECK(not criterion.hasBeenModified());
        }
        THEN("The criterion has the good name")
        {
            CHECK(criterion.getCriterionName() == name);
        }

        checkInsertionBijectivity(criterion, values);
        checkUnknownValueGet(criterion);
    }

    template <class CriterionType>
    void checkNoValue(const std::string& criterionName)
    {
        WHEN("Creating it with no value")
        {
            REQUIRE_THROWS_AS(CriterionType criterion(criterionName, {}, mLogger),
                              Criterion::InvalidCriterionError);
        }
    }
};

SCENARIO_METHOD(CriterionTest, "Criterion lifecycle", "[criterion]")
{

    GIVEN("Some criterion values")
    {
        const CriterionInterface::Values values = generateCriterionValues(300);
        GIVEN("An exclusive criterion")
        {
            const std::string criterionName = "ExclusiveCriterion";
            WHEN("Creating it with some values")
            {
                Criterion criterion(criterionName, values, mLogger);

                THEN("The criterion is not inclusive")
                {
                    CHECK(not criterion.isInclusive());
                }

                checkCriterionBasicBehavior(criterion, criterionName, values);
            }
            WHEN("Creating it with only one value")
            {
                REQUIRE_THROWS_AS(Criterion criterion(criterionName, { { "A", 1 } }, mLogger),
                                  Criterion::InvalidCriterionError);
            }
            checkNoValue<Criterion>(criterionName);
            checkDisplay<Criterion>(criterionName);
        }

        GIVEN("An inclusive criterion")
        {
            const std::string criterionName = "InclusiveCriterion";
            WHEN("Creating it with some values")
            {
                InclusiveCriterion criterion(criterionName, values, mLogger);

                THEN("The criterion is inclusive")
                {
                    CHECK(criterion.isInclusive());
                }
                THEN("Default state is set")
                {
                    CHECK(criterion.getState() == CriterionInterface::State{});
                    CHECK(criterion.getFormattedState() == "none");
                }

                checkCriterionBasicBehavior(criterion, criterionName, values);
            }
            checkNoValue<InclusiveCriterion>(criterionName);
            checkDisplay<InclusiveCriterion>(criterionName);
        }
    }
}

SCENARIO_METHOD(CriteriaTest, "Criteria Use", "[criterion]")
{

    GIVEN("A criteria object")
    {
        WHEN("Adding invalid criteria")
        {
            std::string error;
            REQUIRE_FAILURE(mCriteria.createInclusiveCriterion("InvalidInclusive", {},
                                                               mLogger, error),
                            error);
            REQUIRE_FAILURE(mCriteria.createExclusiveCriterion("InvalidExclusive", {},
                                                               mLogger, error),
                            error);
        }
        WHEN("Adding some criteria")
        {
            CriterionInterface::Values values = { { "State", 1 }, { "State2", 2 } };
            for (auto& description : mDescriptions) {
                std::string error;
                Criterion* addedCriterion = (description.isInclusive ?
                                             mCriteria.createInclusiveCriterion(description.name,
                                                                                values, mLogger,
                                                                                error) :
                                             mCriteria.createExclusiveCriterion(description.name,
                                                                                values, mLogger,
                                                                                error));
                REQUIRE_SUCCESS(addedCriterion != nullptr, error);
                description.criterion = addedCriterion;
                THEN("Added criteria match the request")
                {
                    CAPTURE(description.name);
                    CHECK(addedCriterion->isInclusive() == description.isInclusive);
                    CHECK(addedCriterion->getCriterionName() == description.name);
                }
            }
            WHEN("Retrieving added criteria")
            {
                for (auto& description : mDescriptions) {
                    CAPTURE(description.name);
                    CHECK(mCriteria.getSelectionCriterion(description.name) ==
                          description.criterion);
                    const Criterion* criterion = mCriteria.getSelectionCriterion(description.name);
                    CHECK(criterion == description.criterion);
                }
            }
            WHEN("Retrieving unknown criteria")
            {
                /** FIXME: nullptr in check expression is not available in
                 * Ubuntu catch version for now. We should upgrade it one day.
                 */
                CHECK(mCriteria.getSelectionCriterion("Unknown") == NULL);
            }
            WHEN("Modifying criteria")
            {
                for (auto& description : mDescriptions) {
                    std::string error;
                    REQUIRE_SUCCESS(description.criterion->setState(
                                CriterionInterface::State{2}, error), error);
                    CHECK(description.criterion->hasBeenModified());
                }
                WHEN("Resetting criteria status")
                {
                    mCriteria.resetModifiedStatus();
                    THEN("Status of all criteria has been reset")
                    {
                        for (auto& description : mDescriptions) {
                            CHECK(not description.criterion->hasBeenModified());
                        }
                    }
                }
            }
            WHEN("Listing criteria")
            {
                checkListCriteria(true, true);
                checkListCriteria(true, false);
                checkListCriteria(false, true);
                checkListCriteria(false, false);
            }
            WHEN("Serializing criteria")
            {
                std::string xmlDescription =
                    R"(<?xml version="1.0" encoding="UTF-8"?>
                       <SelectionCriteria>
                        <SelectionCriterion Value="none" Name="A" Kind="Inclusive">
                            <ValuePair Literal="State" Numerical="1"/>
                            <ValuePair Literal="State2" Numerical="2"/>
                        </SelectionCriterion>
                        <SelectionCriterion Value="State" Name="B" Kind="Exclusive">
                            <ValuePair Literal="State" Numerical="1"/>
                            <ValuePair Literal="State2" Numerical="2"/>
                        </SelectionCriterion>
                        <SelectionCriterion Value="none" Name="C" Kind="Inclusive">
                            <ValuePair Literal="State" Numerical="1"/>
                            <ValuePair Literal="State2" Numerical="2"/>
                        </SelectionCriterion>
                        <SelectionCriterion Value="State" Name="D" Kind="Exclusive">
                            <ValuePair Literal="State" Numerical="1"/>
                            <ValuePair Literal="State2" Numerical="2"/>
                        </SelectionCriterion>
                       </SelectionCriteria>)";

                std::string result;
                xmlSerialize(result, &mCriteria, "SelectionCriteria");

                // Remove whitespaces as they are not relevant in xml
                removeWhitespaces(result);
                removeWhitespaces(xmlDescription);

                THEN("Generated xml match expectation")
                {
                    CHECK(result == xmlDescription);
                }
            }
        }
    }
}
