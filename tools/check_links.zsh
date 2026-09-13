#!/bin/zsh
# Verify that every relative markdown link target exists. Usage: tools/check_links.zsh
cd "$(dirname "$0")/.." || exit 1
broken=$(find . -name '*.md' -not -path './engineering/generated/*' -print0 \
  | xargs -0 grep -o '](\([^)#h][^)]*\))' /dev/null \
  | sed 's/:](/ /; s/)$//' \
  | while read -r f target; do
      t="${target%%#*}"; [[ -z "$t" ]] && continue
      [[ -e "$(dirname "$f")/$t" ]] || echo "BROKEN: $f -> $target"
    done)
if [[ -n "$broken" ]]; then print -r -- "$broken"; exit 1; fi
echo "links ok"
