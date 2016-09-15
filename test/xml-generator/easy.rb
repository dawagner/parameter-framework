require "./pfw.rb"

class Easy < PF::Settings
    domain "Domain" do
        conf "Conf" do
            rule "Criterion", :is, "Value"
            set "/A/b", 1
        end
    end
end
