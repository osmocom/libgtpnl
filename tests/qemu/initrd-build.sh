#!/bin/sh -e
DIR="$(cd "$(dirname "$0")" && pwd)"
DIR_INITRD="$DIR/_initrd"
SRC_LIBS="$(realpath "$DIR/../../src/.libs/")"
TOOLS_LIBS="$(realpath "$DIR/../../tools/.libs/")"

# Add one or more files to the initramfs, with parent directories.
# usr-merge: resolve symlinks for /lib -> /usr/lib etc. so "cp --parents" does
# not fail with "cp: cannot make directory '/tmp/initrd/lib': File exists"
# $@: path to files
initrd_add_file() {
	local i

	for i in "$@"; do
		case "$i" in
		/bin/*|/sbin/*|/lib/*|/lib64/*)
			cp -a --parents "$i" "$DIR_INITRD"/usr
			;;
		*)
			cp -a --parents "$i" "$DIR_INITRD"
			;;
		esac
	done
}

# Add binaries with depending libraries
# $@: paths to binaries
initrd_add_bin() {
	local bin
	local bin_path
	local file

	for bin in "$@"; do
		local bin_path="$(which "$bin")"
		if [ -z "$bin_path" ]; then
			echo "ERROR: file not found: $bin"
			exit 1
		fi

		lddtree_out="$(lddtree -l "$bin_path")"
		if [ -z "$lddtree_out" ]; then
			echo "ERROR: lddtree failed on '$bin_path'"
			exit 1
		fi

		for file in $lddtree_out; do
			initrd_add_file "$file"

			# Copy resolved symlink
			if [ -L "$file" ]; then
				initrd_add_file "$(realpath "$file")"
			fi
		done
	done
}

# Add command to run inside the initramfs
# $@: commands
initrd_add_cmd() {
	local i

	if ! [ -e "$DIR_INITRD"/cmd.sh ]; then
		echo "#!/bin/sh -ex" > "$DIR_INITRD"/cmd.sh
		chmod +x "$DIR_INITRD"/cmd.sh
	fi

	for i in "$@"; do
		echo "$i" >> "$DIR_INITRD"/cmd.sh
	done
}

rm -rf "$DIR_INITRD"
mkdir -p "$DIR_INITRD"
cd "$DIR_INITRD"

for dir in bin sbin lib lib64; do
	ln -s usr/"$dir" "$dir"
done

mkdir -p \
	dev/net \
	proc \
	run \
	sys \
	tmp \
	usr/bin \
	usr/sbin

initrd_add_bin \
	busybox \
	ip

initrd_add_cmd \
	"export LD_LIBRARY_PATH=$SRC_LIBS:$LD_LIBRARY_PATH"

export LD_LIBRARY_PATH="$SRC_LIBS:$LD_LIBRARY_PATH"

for i in gtp-link gtp-tunnel; do
	initrd_add_bin "$TOOLS_LIBS"/"$i"
	ln -s "$TOOLS_LIBS"/"$i" usr/bin/"$i"
done

mkdir tests
cp "$DIR"/*.sh tests

cp "$DIR"/initrd-init.sh init

find . -print0 \
	| cpio --quiet -o -0 -H newc \
	| gzip -1 > "$DIR"/_initrd.gz
