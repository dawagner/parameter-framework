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

#include <sstream>
#include <assert.h>
#include "TestPlatform.h"
#include "ParameterMgrPlatformConnector.h"
#include "RemoteProcessorServer.h"

using std::string;
using core::criterion::CriterionInterface;

namespace test
{
namespace platform
{

CTestPlatform::CTestPlatform(const string& strClass, int iPortNumber, sem_t& exitSemaphore) :
    _pParameterMgrPlatformConnector(new CParameterMgrPlatformConnector(strClass)),
    _pParameterMgrPlatformConnectorLogger(new log::CParameterMgrPlatformConnectorLogger),
    _commandParser(*this),
    _exitSemaphore(exitSemaphore)
{
    // Create server
    _pRemoteProcessorServer = new CRemoteProcessorServer(iPortNumber,
                                                         _commandParser.getCommandHandler());

    _pParameterMgrPlatformConnector->setLogger(_pParameterMgrPlatformConnectorLogger);
}

CTestPlatform::~CTestPlatform()
{
    delete _pRemoteProcessorServer;
    delete _pParameterMgrPlatformConnectorLogger;
    delete _pParameterMgrPlatformConnector;
}

bool CTestPlatform::load(std::string& strError)
{
    // Start remote processor server
    if (!_pRemoteProcessorServer->start(strError)) {

        strError = "TestPlatform: Unable to start remote processor server: " + strError;
        return false;
    }

    return true;
}

//////////////// Remote command handlers

bool CTestPlatform::createExclusiveCriterionFromStateList(const string& strName,
                                                          const IRemoteCommand& remoteCommand,
                                                          string& strResult)
{

    assert(_pParameterMgrPlatformConnector != NULL);

    CriterionInterface* pCriterion =
        _pParameterMgrPlatformConnector->createExclusiveCriterion(strName);

    assert(pCriterion!= NULL);

    uint32_t uiNbStates = remoteCommand.getArgumentCount() - 1;
    uint32_t uiState;

    for (uiState = 0; uiState < uiNbStates; uiState++) {

        const std::string& strValue = remoteCommand.getArgument(uiState + 1);

        if (!pCriterion->addValuePair(uiState, strValue, strResult)) {

            strResult = "Unable to add value: " + strValue + ": " + strResult;

            return false;
        }
    }


    return true;
}

bool CTestPlatform::createInclusiveCriterionFromStateList(const string& strName,
                                                          const IRemoteCommand& remoteCommand,
                                                          string& strResult)
{
    assert(_pParameterMgrPlatformConnector != NULL);

    CriterionInterface* pCriterion =
        _pParameterMgrPlatformConnector->createInclusiveCriterion(strName);

    assert(pCriterion != NULL);

    uint32_t uiNbStates = remoteCommand.getArgumentCount() - 1;

    if (uiNbStates > 32) {

        strResult = "Maximum number of states for inclusive criterion is 32";

        return false;
    }

    uint32_t uiState;

    for (uiState = 0; uiState < uiNbStates; uiState++) {

        const std::string& strValue = remoteCommand.getArgument(uiState + 1);

        if (!pCriterion->addValuePair(0x1 << uiState, strValue, strResult)) {

            strResult = "Unable to add value: " + strValue + ": " + strResult;

            return false;
        }
    }

    return true;
}


bool CTestPlatform::createExclusiveCriterion(const string& strName,
                                             uint32_t uiNbStates,
                                             string& strResult)
{
    CriterionInterface* pCriterion =
        _pParameterMgrPlatformConnector->createExclusiveCriterion(strName);

    uint32_t uistate;

    for (uistate = 0; uistate < uiNbStates; uistate++) {

	std::ostringstream ostrValue;

        ostrValue << "State_";
        ostrValue << uistate;

        if (!pCriterion->addValuePair(uistate, ostrValue.str(), strResult)) {

            strResult = "Unable to add value: "
                + ostrValue.str() + ": " + strResult;

            return false;
        }
    }

    return true;
}

bool CTestPlatform::createInclusiveCriterion(const string& strName,
                                             uint32_t uiNbStates,
                                             string& strResult)
{
    CriterionInterface* pCriterion =
        _pParameterMgrPlatformConnector->createInclusiveCriterion(strName);

    if (uiNbStates > 32) {

        strResult = "Maximum number of states for inclusive criterion is 32";

        return false;
    }

    uint32_t uiState;

    for (uiState = 0; uiState < uiNbStates; uiState++) {

	std::ostringstream ostrValue;

        ostrValue << "State_0x";
        ostrValue << (0x1 << uiState);

        if (!pCriterion->addValuePair(0x1 << uiState, ostrValue.str(), strResult)) {

            strResult = "Unable to add value: "
                + ostrValue.str() + ": " + strResult;

            return false;
        }
    }

    return true;
}

bool CTestPlatform::setCriterionState(const string& strName, uint32_t uiState, string& strResult)
{
    CriterionInterface* pCriterion =
        _pParameterMgrPlatformConnector->getSelectionCriterion(strName);

    if (!pCriterion) {

        strResult = "Unable to retrieve selection criterion: " + strName;

        return false;
    }

    pCriterion->setCriterionState(uiState);

    return true;
}

bool CTestPlatform::setCriterionStateByLexicalSpace(const IRemoteCommand& remoteCommand,
                                                    string& strResult)
{

    // Get criterion name
    std::string strCriterionName = remoteCommand.getArgument(0);

    CriterionInterface* pCriterion =
        _pParameterMgrPlatformConnector->getSelectionCriterion(strCriterionName);

    if (!pCriterion) {

        strResult = "Unable to retrieve selection criterion: " + strCriterionName;

        return false;
    }

    // Get substate number, the first argument (index 0) is the criterion name
    uint32_t uiNbSubStates = remoteCommand.getArgumentCount() - 1;

    // Check that exclusive criterion has only one substate
    if (!pCriterion->isInclusive() && uiNbSubStates != 1) {

        strResult = "Exclusive criterion " + strCriterionName + " can only have one state";

        return false;
    }

    /// Translate lexical state to numerical state
    int iNumericalState = 0;
    uint32_t uiLexicalSubStateIndex;

    // Parse lexical substates
    std::string strLexicalState = "";

    for (uiLexicalSubStateIndex = 1;
         uiLexicalSubStateIndex <= uiNbSubStates;
         uiLexicalSubStateIndex++) {
        /*
         * getNumericalValue method from CriterionInterface strip his parameter
         * first parameter based on | sign. In case that the user uses multiple parameters
         * to set InclusiveCriterion value, we aggregate all desired values to be sure
         * they will be handled correctly.
         */
        if (uiLexicalSubStateIndex != 1) {
            strLexicalState += "|";
        }
        strLexicalState += remoteCommand.getArgument(uiLexicalSubStateIndex);
    }

    // Translate lexical to numerical substate
    if (!pCriterion->getNumericalValue(strLexicalState, iNumericalState)) {

        strResult = "Unable to find lexical state \""
            + strLexicalState + "\" in criteria " + strCriterionName;

        return false;
    }

    // Set criterion new state
    pCriterion->setCriterionState(iNumericalState);

    return true;
}

} /** platform namespace */
} /** test namespace */
