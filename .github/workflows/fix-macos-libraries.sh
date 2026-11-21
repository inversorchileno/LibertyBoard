#!/bin/bash
# Makes every library of the application of macOS point inside it.
#
# macdeployqt leaves some parts with the folders of the build machine, as
# the helper of QtWebEngine with the Qt of Homebrew, split in one folder
# per module. Each such reference becomes @rpath, the framework is copied
# into Contents/Frameworks when it is missing, and the file gets an rpath
# to that folder. It repeats until nothing points outside.
set -eu
app="$1"
frameworks="$app/Contents/Frameworks"
outside='^[[:space:]]+(/Users/|/opt/homebrew|/usr/local)'

machOFiles()
{
    find "$app" -type f \( -perm -u+x -o -name '*.dylib' \) -print0 |
        while IFS= read -r -d '' file
        do
            if file -b "$file" | grep -q 'Mach-O'
            then
                printf '%s\n' "$file"
            fi
        done
}

relativePath()
{
    python3 -c 'import os,sys; print(os.path.relpath(sys.argv[1], sys.argv[2]))' "$1" "$2"
}

for round in 1 2 3 4 5
do
    changed=0
    while IFS= read -r file
    do
        refs=$(otool -L "$file" | tail -n +2 | grep -E "$outside" | awk '{print $1}' || true)
        [ -z "$refs" ] && continue
        chmod u+w "$file"
        for ref in $refs
        do
            if [[ "$ref" == *.framework/* ]]
            then
                # /some/where/QtCore.framework/Versions/A/QtCore
                root="${ref%%.framework/*}.framework"
                name=$(basename "$root")
                inside="${ref#"$root"/}"
                if [ ! -e "$frameworks/$name" ]
                then
                    echo "Copying $name"
                    cp -R "$root" "$frameworks/"
                    chmod -R u+w "$frameworks/$name"
                fi
                new="@rpath/$name/$inside"
            else
                name=$(basename "$ref")
                if [ ! -e "$frameworks/$name" ]
                then
                    echo "Copying $name"
                    cp "$ref" "$frameworks/"
                    chmod u+w "$frameworks/$name"
                fi
                new="@rpath/$name"
            fi
            echo "$file: $ref -> $new"
            install_name_tool -change "$ref" "$new" "$file"
        done
        rpath="@loader_path/$(relativePath "$frameworks" "$(dirname "$file")")"
        if ! otool -l "$file" | grep -A2 LC_RPATH | grep -qF "path $rpath "
        then
            install_name_tool -add_rpath "$rpath" "$file" || true
        fi
        changed=1
    done < <(machOFiles)

    # The own name of a copied library.
    while IFS= read -r file
    do
        id=$(otool -D "$file" | tail -n +2)
        if [ -n "$id" ] && printf '%s\n' "  $id" | grep -qE "$outside"
        then
            if [[ "$id" == *.framework/* ]]
            then
                root="${id%%.framework/*}.framework"
                install_name_tool -id "@rpath/$(basename "$root")/${id#"$root"/}" "$file"
            else
                install_name_tool -id "@rpath/$(basename "$id")" "$file"
            fi
            changed=1
        fi
    done < <(machOFiles)

    [ "$changed" = 0 ] && exit 0
done

echo "Still libraries outside after five rounds"
exit 1
