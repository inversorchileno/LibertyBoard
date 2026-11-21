#!/bin/bash
# Adds a file to the release of this commit, which the Windows build
# publishes: it waits for it up to an hour.
set -u
file="$1"

for attempt in $(seq 1 60); do
    tag=$(gh api "repos/$GITHUB_REPOSITORY/releases?per_page=20" \
          --jq ".[] | select(.target_commitish == \"$GITHUB_SHA\") | .tag_name" | head -n 1)

    if [ -n "$tag" ]; then
        gh release upload "$tag" "$file" --repo "$GITHUB_REPOSITORY" --clobber
        exit $?
    fi

    echo "Waiting for the release of $GITHUB_SHA ($attempt)..."
    sleep 60
done

echo "No release for $GITHUB_SHA"
exit 1
