#include "ParameterMgrFullConnector.h"
#include "convert.hpp"
#include <string>

int main(int argc, char *argv[])
{
    if (argc < 4) {
        return -1;
    }

    auto toplevelConf = argv[1];
    int phase = 0;
    int nbBoolCriteria = 0;

    convertTo(argv[2], phase);
    convertTo(argv[3], nbBoolCriteria);

    CParameterMgrFullConnector connector(toplevelConf);
    if (phase == 0) {
        return 0;
    }
    connector.setForceNoRemoteInterface(true);

    std::string error;
    auto moodType = connector.createSelectionCriterionType(false);
    moodType->addValuePair(0, "mad");
    moodType->addValuePair(1, "sad");
    moodType->addValuePair(2, "glad");
    auto mood = connector.createSelectionCriterion("Mood", moodType);

    auto colorsType = connector.createSelectionCriterionType(true);
    colorsType->addValuePair(1<<0, "red");
    colorsType->addValuePair(1<<1, "green");
    colorsType->addValuePair(1<<2, "blue");
    auto colors = connector.createSelectionCriterion("Colors", colorsType);

    if (nbBoolCriteria > 0) {
        auto boolType = connector.createSelectionCriterionType(false);
        boolType->addValuePair(0, "False");
        boolType->addValuePair(1, "True");
        for (int i=0; i<nbBoolCriteria; i++) {
            connector.createSelectionCriterion("Bool" + std::to_string(i), boolType);
        }
    }
    if (phase == 1) {
        return 0;
    }

    connector.start(error);
    if (phase == 2) {
        return 0;
    }

    mood->setCriterionState(2);
    colors->setCriterionState(3);

    connector.applyConfigurations();

    return 0;
}
