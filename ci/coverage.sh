#! /bin/sh

wget https://codecov.io/bash -O $HOME/codecov; chmod +x $HOME/codecov
$HOME/codecov \
     -g "*/build_debug/bindings/python/*" \
     -g "*/build_debug/CMakeFiles/*" \
     -g "*/build/*" \
     -g "*/build_less_features/*" \
     -g "*/install/*" \
     -g "*/skeleton-subsystem/*" \
     -g "*/tools/clientSimulator/*" \
     -g "*/test/test-subsystem/*" \
     -g "*/test/introspection-subsystem/*" \
     -g "*/test/functional-tests/*" \
     -g "*/test/tmpfile/*" \
     -g "*/test/tokenizer/*" \
     -g "*/bindings/c/Test.cpp" \
     -g "*/utility/test/*" \
     -x "$gcov" \
     -a '\--long-file-names --preserve-paths';
