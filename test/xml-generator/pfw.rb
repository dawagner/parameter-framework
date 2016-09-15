module PF
    # Helpers that should not be accidently called by users of PF::Settings
    class Private
        MATCHERS = {is: "Is", is_not: "IsNot", includes: "Includes", excludes: "Excludes"}

        class << self
            def setRule(domain, conf, rules)
                puts "setRule #{domain} #{conf} All{ #{rules.flatten.join(', ')} }"
            end
        end
    end

    # Implements a Domain-Specific Language for describing Parameter Framework settings.
    # It uses the same concepts and replace the EDD syntax. See details of these concepts here:
    #   https://github.com/01org/parameter-framework/blob/master/tools/xmlGenerator/README.md
    # The same keywords as those in EDD are used in this DSL.
    #
    # TODO: when methods are used in the wrong context (e.g. a 'set' as a
    # direct child of a 'domain'), we should raise an error.
    class Settings
        @domain_ctx = []
        @conf_ctx = []
        # The rule context is a 2-dimensional array: the first dimension
        # corresponds to the nesting of domainGroup/domain/confGroup/conf and
        # the second dimension is the list of rules in each nesting level
        @rule_ctx = []

        class << self

            attr_accessor :domain_ctx, :conf_ctx, :rule_ctx

            # In the context of a conf, set a parameter.
            def set(name, value)
                domain = Settings.domain_ctx.join('.')
                conf = Settings.conf_ctx.join('.')

                # When we get to the point of setting configuration parameters,
                # it means that we are done describing the rules for that
                # config. Hence, let's first commit them.
                # TODO: support "Any" rules
                Private.setRule(domain, conf, Settings.rule_ctx)

                puts "setConfigurationParameter #{domain} #{conf} #{name} #{value}"
            end

            # TODO: should we implement this or let users write ruby code to
            # have the same functionality?
            def component(name)
                raise NotImprementedError
            end

            # In the context of a domainGroup, domain or conf, add a configuration rule.
            def rule(name, matcher, value)
                pprint = lambda {|k| k.inspect.dump}
                matcher_print = Private::MATCHERS[matcher] or raise NameError, \
                    "Unknown matcher '#{matcher.inspect}'." \
                    " Only #{Private::MATCHERS.keys.map(&pprint).join(', ')} are supported", \
                    caller # Pop this method's context, making it obvious where the error occured
                Settings.rule_ctx.last << "#{name} #{matcher_print} #{value}"
            end

            # In the context of a domain, add a configuration.
            def conf(leaf_name)
                Settings.rule_ctx << []
                Settings.conf_ctx << leaf_name

                puts "createConfiguration" \
                    " #{Settings.domain_ctx.join('.')}" \
                    " #{Settings.conf_ctx.join('.')}"

                yield
                Settings.conf_ctx.pop
                Settings.rule_ctx.pop
            end

            # In the context of a domainGroup, a domain or another confGroup,
            # creates a conf namespace.
            def confGroup(name)
                Settings.rule_ctx << []
                Settings.conf_ctx << name
                yield
                Settings.conf_ctx.pop
                Settings.rule_ctx.pop
            end

            # In the context of a domainGroup or domain, reates a conf rules
            # template (conf with this name will inherit the rules of the
            # template).
            # TODO
            def confType(name)
                raise NotImprementedError
            end

            # Create a domain (at the root or in the context or a domainGroup).
            # TODO: support sequence awareness
            def domain(leaf_name)
                Settings.rule_ctx << []
                Settings.domain_ctx << leaf_name

                puts "createDomain #{Settings.domain_ctx.join('.')}"

                yield
                Settings.domain_ctx.pop
                Settings.rule_ctx.pop
            end

            # At the root or in another domainGroup, creates a domain namespace.
            def domainGroup(name)
                Settings.rule_ctx << []
                Settings.domain_ctx << name
                yield
                Settings.domain_ctx.pop
                Settings.rule_ctx.pop
            end
        end # class << self (class methods)
    end # class PF::Settings
end # module PF
