if [ $# -ne 1 ] 
	then echo "nr incorect de argumente"
	exit 1
fi  

caract="$1"
count=0
while IFS= read -r line || [[ -n "$line" ]]; do
	if [[ "$line" =~ ^[A-Z].*[A-Z0-9\ \,\.\!\?]+$ && ! "$line" =~ ,\ (și|[a-zA-Z0-9]) && "$line" =~ [\.\!\?]$ ]]; then
		if [[ "$line" =~ $caract ]]; then
			 ((count++))
		fi
    fi
done

echo "$count"
