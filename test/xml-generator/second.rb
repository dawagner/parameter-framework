require "./pfw.rb"

class Second < PF::Settings
    domainGroup("EddGroup") {
        rule "Colors", :includes, "Red"

        domain("Second") {
            # confType "On" # TODO

            confGroup("Blue") {
                rule "Colors", :includes, "Blue"

                conf("On") {
                    set "/Test/test/block/2/uint8", 3
                }
                conf("Off") {
                    set "/Test/test/block/2/uint8", 4
                }
            }

            conf("Green") {
                rule "Colors", :includes, "Green"
                set "/Test/test/block/2/uint8", 5
            }

            conf("Default") {
                set "/Test/test/block/2/uint8", 6
            }
        }
    }
end
