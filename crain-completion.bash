#!/usr/bin/env bash

colors="white black red green blue yellow cyan magenta"
speeds="0 1 2 3 4 5 6 7 8 9 10"

get_charset_name(){

	FILE="$HOME/.config/crain/charsets"
	if test -f "$FILE"; then
		cut -d' ' -f1 $FILE
	fi

}

_crain_completions(){

	local cur prev

	COMPREPLY=()
	prev=${COMP_WORDS[COMP_CWORD-1]}

	# Do we have anything after "crain"...
	if [ $COMP_CWORD -ge 2 ]; then

		# Check if previous word has a "-" as first character
		if [ ${prev:0:1} = "-" ]; then

			# Get the last character in the option list and autocomplete accordingly
			case ${prev: -1} in
				"c")
					COMPREPLY=($(compgen -W "$(get_charset_name)" "${COMP_WORDS[COMP_CWORD]}"))
					;;
				"h"|"t")
					COMPREPLY=($(compgen -W "$colors" "${COMP_WORDS[COMP_CWORD]}"))
					;;
				"s")
					COMPREPLY=($(compgen -W "$speeds" "${COMP_WORDS[COMP_CWORD]}"))
					;;
			esac

		fi

	fi

	#
}

complete -F _crain_completions crain
