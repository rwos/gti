#compdef gti=git
# pretty simple one, it's just reuse the git one

type -w _git | grep -q function && \
  _git
