# bash completion for gti                                  -*- shell-script -*-

# We assume that git is installed and its autocompletion script is nearby
___gti_complete() {
  local PWD=$(dirname "${BASH_SOURCE[0]}")

  if [ -f "${PWD}/git" ]; then
    source "${PWD}/git"
    if [ "$(type -t ___git_complete)" ]; then
      # gti is just a proxy, so it supports all things from original git
      ___git_complete gti __git_main
    fi
  fi
}

___gti_complete
