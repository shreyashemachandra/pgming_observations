BEGIN {
	FS=")"
}

# Rules
{
	arr[$1] = $0
}

END {
	for (i = 7; i >= 0; i--) {
		if (arr[i]) {
			print arr[i]
		}
	}
}
