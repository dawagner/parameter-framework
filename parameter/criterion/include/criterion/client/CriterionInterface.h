/*
 * Copyright (c) 2011-2015, Intel Corporation
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
#pragma once

#include <string>
#include <map>

namespace core
{
namespace criterion
{

/** Client criterion interface used for interacting with the system state
 * Allows client to set or retrieve a Criterion state.
 */
class CriterionInterface
{
public:
    /** Type which associate literal and numerical value */
    using Values = std::map<std::string, int>;

    virtual void setCriterionState(int iState) = 0;
    virtual int getCriterionState() const = 0;
    virtual std::string getCriterionName() const = 0;

    /** Retrieve the numerical value from the literal representation of the criterion type.
     *
     * @param[in] literalValue: criterion state value represented as a stream. If the criterion is
     *                          inclusive, it supports more than one criterion type value delimited
     *                          by the "|" symbol.
     * @param[out] numericalValue: criterion state value represented as an integer.
     *
     * @return true if a numerical value is retrieved from the literal one, false otherwise.
     */
    virtual bool getNumericalValue(const std::string& literalValue, int& numericalValue) const = 0;

    /** Retrieve the numerical value from the literal representation of the criterion type.
     *
     * @param[in] numericalValue: criterion state value represented as an integer.
     * @param[out] literalValue: criterion state value represented as a stream. If the criterion is
     *                           inclusive, it supports more than one criterion type value delimited
     *                           by the "|" symbol.
     *
     * @return true if a numerical value is retrieved from the literal one, false otherwise.
     */
    virtual bool getLiteralValue(int numericalValue, std::string& literalValue) const = 0;

    /** Retrieve formatted current criterion state
     *
     * @return formatted string of criterion state
     */
    virtual std::string getFormattedState() const = 0;

    /** Retrieve Criterion type
     *
     * @return true if the criterion is Inclusive, false if it is Exclusive
     */
    virtual bool isInclusive() const = 0;

protected:
    virtual ~CriterionInterface() {}
};

} /** criterion namespace */
} /** core namespace */
