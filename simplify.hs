type CriterionValue = String

-- All the legal criterion/value operators
data Test = Is | IsNot | Includes | Excludes deriving (Eq, Show)

-- A criterion with a name and legal values
data Criterion = Inclusive String [CriterionValue] | Exclusive String [CriterionValue] deriving (Eq, Show)

-- A rule: always true, always false or with a test
-- TODO: this is strange that True/False are rules...
data RuleContent =
      TrueRule
    | FalseRule
    | CriterionTest (Criterion, Test, CriterionValue) deriving (Eq, Show)

-- A compound
-- "All []" is true;
-- "Any []" is false.
-- TODO: remove "Rule" from the name
data Compound r =
      AllRule [r]
    | AnyRule [r] deriving (Eq, Show)

-- A rule tree
data Rule content =
      Rule content
    | CompoundRule (Compound (Rule content)) deriving (Eq, Show)

instance Functor Compound where
    fmap f (AllRule rules) = AllRule $ fmap f rules
    fmap f (AnyRule rules) = AnyRule $ fmap f rules

mapToLeaf :: (a -> b) -> Compound (Rule a) -> Compound (Rule b)
-- first fmap unpack 'Compound'; second fmap unpacks 'Rule'
mapToLeaf = fmap . fmap

instance Functor Rule where
    fmap f (Rule content) = Rule $ f content
    fmap f (CompoundRule compound) = CompoundRule $ mapToLeaf f compound

prepend :: r -> Compound r -> Compound r
prepend r (AllRule rules) = AllRule $ r : rules
prepend r (AnyRule rules) = AnyRule $ r : rules

simplify :: Rule RuleContent -> Rule RuleContent
simplify (Rule content) = Rule content
simplify (CompoundRule compound) = minimize . simplifyCompound $ compound

simplifyCompound :: Compound (Rule RuleContent) -> Rule RuleContent
simplifyCompound (AllRule rules) = foldr injectAll (CompoundRule $ AllRule []) (fmap simplify rules)
simplifyCompound (AnyRule rules) = foldr injectAny (CompoundRule $ AnyRule []) (fmap simplify rules)

inject :: (RuleContent, RuleContent) -> Rule RuleContent -> Rule RuleContent -> Rule RuleContent
inject (zero, sink) (Rule candidate) acc
    | candidate == zero = acc
    | candidate == sink = Rule sink
    | acc == Rule sink = Rule sink
inject _ rule (CompoundRule acc) = CompoundRule $ prepend rule acc
inject _ _ _ = error "don't inject in a regular single rule"

injectAll = inject (TrueRule, FalseRule)
injectAny = inject (FalseRule, TrueRule)

minimize :: Rule RuleContent -> Rule RuleContent
minimize (CompoundRule (AllRule [])) = trueRule
minimize (CompoundRule (AnyRule [])) = falseRule
minimize x = x

falseRule = Rule FalseRule
trueRule = Rule TrueRule
allRule = CompoundRule . AllRule
anyRule = CompoundRule . AnyRule

-- test

output_devices = Inclusive "OutputDevices" ["Headset", "Headphone", "Speaker", "Bluetooth"]
mode = Exclusive "AndroidMode" ["Normal", "Ringtone", "InCall", "InCommunication"]

media_speaker = AllRule [Rule $ CriterionTest (output_devices, Is, "Speaker"), Rule $ CriterionTest (mode, Is, "Normal")]

simple_rule = AllRule [Rule $ CriterionTest (Exclusive "Criterion" ["a", "b"], Is, "a")]

no_speaker r =
    if r == CriterionTest (output_devices, Is, "Speaker") then
        FalseRule
    else
        r

sample_rule = AllRule [trueRule, trueRule, allRule [trueRule, falseRule], anyRule [falseRule, trueRule], CompoundRule simple_rule]

recurse_rule = AllRule [allRule [allRule [allRule [allRule [falseRule]]]]]
