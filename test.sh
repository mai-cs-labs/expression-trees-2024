#!/bin/sh
readonly tmpfile="$(mktemp -q)"

total=0
passed=0

# @NOTE: Build program first
make -j

for t in $(find . -type f -iname '*-test'); do
	total=$(($total+1))
	name="${t##./tests/}"
	answer="${t%%-test}-answer"

	case "${t}" in
		*simplify*)
			mode=simplify
			;;
		*expand*)
			mode=expand
			;;
		*eval*)
			mode=eval
			;;
	esac

	./expr -f ${t} ${mode:-simplify} >${tmpfile}

	if ! diff -awB --strip-trailing-cr ${tmpfile} ${answer} >/dev/null 2>&1; then
		echo "${name} failed:"
		printf "Expected:\n\t%s\nFound:\n\t%s\n\n" "$(cat ${answer})" "$(cat ${tmpfile})"
	else
		passed=$(($passed+1))
	fi
done; unset t

rm ${tmpfile}

if [ ${passed} -eq ${total} ]; then
	printf "\n◉ All ${total} tests passed!\n"
	exit 0
else
	printf "\n○ ${passed}/${total} tests passed\n"
	exit 1
fi
