#! /bin/sh
git ls-files | grep -E '\.[ch](pp)?$' |  xargs clang-format-3.8 -i && git diff --exit-code
